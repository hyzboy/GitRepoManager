#include "RepositoryTreeWidget.h"
#include <QHeaderView>

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

void RepositoryTreeWidget::addRepository(const QString &repoPath)
{
    QTreeWidgetItem *repoItem = createRepositoryNode(repoPath);
    if (repoItem) {
        addTopLevelItem(repoItem);
        expandAll(); // Expand all nodes after adding a repository
    }
}

void RepositoryTreeWidget::clearRepositories()
{
    clear();
}

QTreeWidgetItem *RepositoryTreeWidget::createRepositoryNode(const QString &repoPath)
{
    QTreeWidgetItem *repoItem = new QTreeWidgetItem();
    repoItem->setText(0, repoPath);
    repoItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    
    // Add child nodes
    addBranchesNode(repoItem);
    addTagsNode(repoItem);
    addRemotesNode(repoItem);
    addSubmodulesNode(repoItem);
    
    return repoItem;
}

void RepositoryTreeWidget::addBranchesNode(QTreeWidgetItem *repoItem)
{
    QTreeWidgetItem *branchesItem = new QTreeWidgetItem(repoItem);
    branchesItem->setText(0, "Branches");
    branchesItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    
    // Add example branches (these can be populated from git data later)
    QTreeWidgetItem *mainItem = new QTreeWidgetItem(branchesItem);
    mainItem->setText(0, "main");
    
    QTreeWidgetItem *devItem = new QTreeWidgetItem(branchesItem);
    devItem->setText(0, "develop");
}

void RepositoryTreeWidget::addTagsNode(QTreeWidgetItem *repoItem)
{
    QTreeWidgetItem *tagsItem = new QTreeWidgetItem(repoItem);
    tagsItem->setText(0, "Tags");
    tagsItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
}

void RepositoryTreeWidget::addRemotesNode(QTreeWidgetItem *repoItem)
{
    QTreeWidgetItem *remotesItem = new QTreeWidgetItem(repoItem);
    remotesItem->setText(0, "Remotes");
    remotesItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    
    // Add example remote
    QTreeWidgetItem *originItem = new QTreeWidgetItem(remotesItem);
    originItem->setText(0, "origin");
}

void RepositoryTreeWidget::addSubmodulesNode(QTreeWidgetItem *repoItem)
{
    QTreeWidgetItem *submodulesItem = new QTreeWidgetItem(repoItem);
    submodulesItem->setText(0, "Submodules");
    submodulesItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
}
