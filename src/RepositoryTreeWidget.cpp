#include "RepositoryTreeWidget.h"
#include <QHeaderView>
#include <QDebug>
#include <cstring>

namespace
{   
    // Helper: get current branch name (returns empty QString on error).
    // If HEAD is detached, returns "detached: <short-oid>"
    static QString getCurrentBranchName(git_repository *repo)
    {
        if (!repo) {
            return QString();
        }

        git_reference *head = nullptr;
        if (git_repository_head(&head, repo) != 0 || head == nullptr) {
            // Could not resolve HEAD
            return QString();
        }

        QString result;
        const char *branch_name = nullptr;
        if (git_branch_name(&branch_name, head) == 0 && branch_name) {
            result = QString::fromUtf8(branch_name);
        } else {
            // Detached HEAD: show short SHA
            const git_oid *oid = git_reference_target(head);
            if (oid) {
                char oid_str[GIT_OID_HEXSZ + 1] = {0};
                git_oid_tostr(oid_str, sizeof(oid_str), oid);
                result = QStringLiteral("detached: %1").arg(QString::fromUtf8(oid_str));
            }
        }

        git_reference_free(head);
        return result;
    }
}//namespace

RepositoryTreeWidget::RepositoryTreeWidget(QWidget *parent)
    : QTreeWidget(parent), rootItem(nullptr)
{
    setupTree();
}

RepositoryTreeWidget::~RepositoryTreeWidget()
{
}

void RepositoryTreeWidget::setupTree()
{
    setColumnCount(1);
    setHeaderLabel("Repositories");
    
    // Set some visual properties
    setAnimated(true);
    setIndentation(15);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void RepositoryTreeWidget::addRepository(const QString &repoPath, git_repository *repo)
{
    QTreeWidgetItem *repoItem = createRepositoryNode(repoPath, repo);
    if (repoItem) {
        addTopLevelItem(repoItem);
        expandAll(); // Expand all nodes after adding a repository
    }
}

void RepositoryTreeWidget::clearRepositories()
{
    clear();
}

QTreeWidgetItem *RepositoryTreeWidget::createRepositoryNode(const QString &repoPath, git_repository *repo)
{
    QTreeWidgetItem *repoItem = new QTreeWidgetItem();
    repoItem->setText(0, repoPath);
    repoItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    
    // Add child nodes with repository data
    addBranchesNode(repoItem, repo);
    addTagsNode(repoItem, repo);
    addRemotesNode(repoItem, repo);
    addSubmodulesNode(repoItem, repo);
    
    return repoItem;
}

void RepositoryTreeWidget::addBranchesNode(QTreeWidgetItem *repoItem, git_repository *repo)
{
    QTreeWidgetItem *branchesItem = new QTreeWidgetItem(repoItem);
    branchesItem->setText(0, "Branches");
    branchesItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    
    if (!repo) {
        return;
    }
    
    // Determine current branch (used to mark the current branch)
    QString currentBranch = getCurrentBranchName(repo);

    // Get branch iterator
    git_branch_iterator *branch_iter = nullptr;
    int error = git_branch_iterator_new(&branch_iter, repo, GIT_BRANCH_ALL);
    
    if (error == 0 && branch_iter) {
        git_reference *ref = nullptr;
        git_branch_t branch_type;
        
        while (git_branch_next(&ref, &branch_type, branch_iter) == 0) {
            const char *branch_name = nullptr;
            git_branch_name(&branch_name, ref);
            
            if (branch_name) {
                QTreeWidgetItem *branchItem = new QTreeWidgetItem(branchesItem);
                branchItem->setText(0, QString::fromUtf8(branch_name));

                if(currentBranch == QString::fromUtf8(branch_name)) {
                    branchItem->setText(0, branchItem->text(0) + "  (current)");
                    QFont font = branchItem->font(0);
                    font.setBold(true);
                    branchItem->setFont(0, font);
                }
            }
            
            git_reference_free(ref);
        }
        
        git_branch_iterator_free(branch_iter);
    }
}

void RepositoryTreeWidget::addTagsNode(QTreeWidgetItem *repoItem, git_repository *repo)
{
    QTreeWidgetItem *tagsItem = new QTreeWidgetItem(repoItem);
    tagsItem->setText(0, "Tags");
    tagsItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    
    if (!repo) {
        return;
    }
    
    // Get tags
    git_strarray tag_names = {0};
    int error = git_tag_list(&tag_names, repo);
    
    if (error == 0) {
        for (size_t i = 0; i < tag_names.count; ++i) {
            QTreeWidgetItem *tagItem = new QTreeWidgetItem(tagsItem);
            tagItem->setText(0, QString::fromUtf8(tag_names.strings[i]));
        }
        
        git_strarray_free(&tag_names);
    }
}

void RepositoryTreeWidget::addRemotesNode(QTreeWidgetItem *repoItem, git_repository *repo)
{
    QTreeWidgetItem *remotesItem = new QTreeWidgetItem(repoItem);
    remotesItem->setText(0, "Remotes");
    remotesItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    
    if (!repo) {
        return;
    }
    
    // Get remotes
    git_strarray remote_names = {0};
    int error = git_remote_list(&remote_names, repo);
    
    if (error == 0) {
        for (size_t i = 0; i < remote_names.count; ++i) {
            QTreeWidgetItem *remoteItem = new QTreeWidgetItem(remotesItem);
            remoteItem->setText(0, QString::fromUtf8(remote_names.strings[i]));
        }
        
        git_strarray_free(&remote_names);
    }
}

void RepositoryTreeWidget::addSubmodulesNode(QTreeWidgetItem *repoItem, git_repository *repo)
{
    QTreeWidgetItem *submodulesItem = new QTreeWidgetItem(repoItem);
    submodulesItem->setText(0, "Submodules");
    submodulesItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    
    if (!repo) {
        return;
    }
    
    // Get submodules using git_submodule_foreach
    git_config *cfg = nullptr;
    if (git_repository_config(&cfg, repo) == 0) {
        git_config_iterator *iter = nullptr;
        if (git_config_iterator_new(&iter, cfg) == 0) {
            git_config_entry *entry = nullptr;
            
            while (git_config_next(&entry, iter) == 0) {
                const char *key = entry->name;
                // Look for submodule entries (they start with "submodule.")
                if (key && strncmp(key, "submodule.", 10) == 0) {
                    // Extract submodule name
                    const char *start = key + 10;
                    const char *dot = strchr(start, '.');
                    if (dot) {
                        int len = dot - start;
                        QString submoduleName = QString::fromUtf8(start, len);
                        
                        // Check if we already added this submodule
                        bool found = false;
                        for (int i = 0; i < submodulesItem->childCount(); ++i) {
                            if (submodulesItem->child(i)->text(0) == submoduleName) {
                                found = true;
                                break;
                            }
                        }
                        
                        if (!found) {
                            QTreeWidgetItem *submoduleItem = new QTreeWidgetItem(submodulesItem);
                            submoduleItem->setText(0, submoduleName);
                        }
                    }
                }
            }
            
            git_config_iterator_free(iter);
        }
        git_config_free(cfg);
    }
}
