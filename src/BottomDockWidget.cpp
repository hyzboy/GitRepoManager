#include "BottomDockWidget.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QDebug>

BottomDockWidget::BottomDockWidget(QWidget *parent)
    : QDockWidget("Commit Changes", parent), currentRepo(nullptr)
{
    setupUI();
}

BottomDockWidget::~BottomDockWidget()
{
}

void BottomDockWidget::setupUI()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    
    // Create horizontal splitter for file list and diff display
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Left panel - File List
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *filesLabel = new QLabel("Changed Files:", this);
    filesLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    leftLayout->addWidget(filesLabel);
    
    fileList = new QListWidget(this);
    connect(fileList, &QListWidget::currentRowChanged, this, &BottomDockWidget::onFileSelected);
    leftLayout->addWidget(fileList);
    
    mainSplitter->addWidget(leftPanel);
    
    // Right panel - Diff Display
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *diffLabel = new QLabel("File Diff:", this);
    diffLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    rightLayout->addWidget(diffLabel);
    
    diffDisplay = new QTextEdit(this);
    diffDisplay->setReadOnly(true);
    diffDisplay->setFont(QFont("Consolas", 10)); // Use monospace font for diff
    diffDisplay->setLineWrapMode(QTextEdit::NoWrap);
    diffDisplay->setPlaceholderText("Select a file to view changes...");
    rightLayout->addWidget(diffDisplay);
    
    mainSplitter->addWidget(rightPanel);
    
    // Set splitter proportions (1:2 ratio - file list smaller, diff larger)
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 2);
    
    mainLayout->addWidget(mainSplitter);
    setWidget(content);
}

void BottomDockWidget::setRepository(git_repository *repo)
{
    currentRepo = repo;
}

void BottomDockWidget::displayCommitFiles(const git_oid &oid)
{
    clearDisplay();
    
    if (!currentRepo) {
        return;
    }
    
    // Store current commit OID
    git_oid_cpy(&currentCommitOid, &oid);
    
    // Get commit object
    git_commit *commit = nullptr;
    if (git_commit_lookup(&commit, currentRepo, &oid) != 0) {
        qDebug() << "Failed to lookup commit";
        return;
    }
    
    populateFileList(commit);
    git_commit_free(commit);
}

void BottomDockWidget::populateFileList(git_commit *commit)
{
    if (!commit) return;
    
    // Clear the file path map
    filePathMap.clear();
    
    // Get commit tree
    git_tree *commit_tree = nullptr;
    if (git_commit_tree(&commit_tree, commit) != 0) {
        qDebug() << "Failed to get commit tree";
        return;
    }
    
    // For now, we'll show a simplified view
    // In a real implementation, you'd want to compare with parent commit(s)
    unsigned int parent_count = git_commit_parentcount(commit);
    
    if (parent_count == 0) {
        // Initial commit - show all files as added
        fileList->addItem("(Initial commit - all files added)");
    } else {
        // Get parent commit for comparison
        git_commit *parent_commit = nullptr;
        if (git_commit_parent(&parent_commit, commit, 0) == 0) {
            
            git_tree *parent_tree = nullptr;
            if (git_commit_tree(&parent_tree, parent_commit) == 0) {
                
                // Create diff between parent and current commit
                git_diff *diff = nullptr;
                if (git_diff_tree_to_tree(&diff, currentRepo, parent_tree, commit_tree, nullptr) == 0) {
                    
                    // Get number of deltas (changed files)
                    size_t num_deltas = git_diff_num_deltas(diff);
                    
                    for (size_t i = 0; i < num_deltas; ++i) {
                        const git_diff_delta *delta = git_diff_get_delta(diff, i);
                        if (delta) {
                            QString filePath = QString::fromUtf8(delta->new_file.path);
                            QString status;
                            switch (delta->status) {
                                case GIT_DELTA_ADDED: status = "[A] "; break;
                                case GIT_DELTA_DELETED: status = "[D] "; break;
                                case GIT_DELTA_MODIFIED: status = "[M] "; break;
                                case GIT_DELTA_RENAMED: status = "[R] "; break;
                                case GIT_DELTA_COPIED: status = "[C] "; break;
                                default: status = "[?] "; break;
                            }
                            int row = fileList->count();
                            fileList->addItem(status + filePath);
                            filePathMap[row] = filePath;
                        }
                    }
                    
                    git_diff_free(diff);
                }
                
                git_tree_free(parent_tree);
            }
            
            git_commit_free(parent_commit);
        }
    }
    
    git_tree_free(commit_tree);
}

void BottomDockWidget::onFileSelected()
{
    QListWidgetItem *item = fileList->currentItem();
    if (!item) {
        diffDisplay->clear();
        return;
    }
    
    int row = fileList->row(item);
    QString filePath;
    
    if (filePathMap.contains(row)) {
        filePath = filePathMap[row];
    } else {
        // Extract file path for initial commit
        QString itemText = item->text();
        if (itemText.startsWith("[") && itemText.indexOf("] ") > 0) {
            filePath = itemText.mid(itemText.indexOf("] ") + 2);
        }
    }
    
    showFileDiff(filePath);
}

void BottomDockWidget::showFileDiff(const QString &filePath)
{
    if (!currentRepo || filePath.isEmpty()) {
        diffDisplay->setPlainText("No file selected or repository not available.");
        return;
    }
    
    // Get commit object
    git_commit *commit = nullptr;
    if (git_commit_lookup(&commit, currentRepo, &currentCommitOid) != 0) {
        diffDisplay->setPlainText("Failed to lookup commit.");
        return;
    }
    
    unsigned int parent_count = git_commit_parentcount(commit);
    if (parent_count == 0) {
        // Initial commit - show file content
        git_tree *tree = nullptr;
        if (git_commit_tree(&tree, commit) == 0) {
            git_tree_entry *entry = nullptr;
            if (git_tree_entry_bypath(&entry, tree, filePath.toUtf8().constData()) == 0) {
                git_blob *blob = nullptr;
                if (git_blob_lookup(&blob, currentRepo, git_tree_entry_id(entry)) == 0) {
                    const void *content = git_blob_rawcontent(blob);
                    size_t size = git_blob_rawsize(blob);
                    QString text = QString::fromUtf8(static_cast<const char*>(content), size);
                    diffDisplay->setPlainText(text);
                    git_blob_free(blob);
                } else {
                    diffDisplay->setPlainText("Failed to read file content.");
                }
                git_tree_entry_free(entry);
            } else {
                diffDisplay->setPlainText("File not found in commit.");
            }
            git_tree_free(tree);
        }
    } else {
        // Show diff with parent - filter for specific file
        git_commit *parent_commit = nullptr;
        if (git_commit_parent(&parent_commit, commit, 0) == 0) {
            git_tree *commit_tree = nullptr;
            git_tree *parent_tree = nullptr;
            
            if (git_commit_tree(&commit_tree, commit) == 0 && 
                git_commit_tree(&parent_tree, parent_commit) == 0) {
                
                git_diff *diff = nullptr;
                if (git_diff_tree_to_tree(&diff, currentRepo, parent_tree, commit_tree, nullptr) == 0) {
                    
                    // Generate full patch and filter for specific file
                    git_buf patch_buf = GIT_BUF_INIT;
                    if (git_diff_to_buf(&patch_buf, diff, GIT_DIFF_FORMAT_PATCH) == 0) {
                        QString fullPatch = QString::fromUtf8(patch_buf.ptr, patch_buf.size);
                        
                        // Filter the patch to show only the specific file
                        QString output;
                        QStringList lines = fullPatch.split('\n');
                        bool inTargetFile = false;
                        
                        for (const QString &line : lines) {
                            // Check if this line starts a new file diff
                            if (line.startsWith("diff --git")) {
                                // Check if it's our target file
                                if (line.contains(" b/" + filePath)) {
                                    inTargetFile = true;
                                    output += line + '\n';
                                } else if (!output.isEmpty() && inTargetFile) {
                                    // We were in target file but now moved to another file
                                    break;
                                } else {
                                    inTargetFile = false;
                                }
                            } else if (inTargetFile) {
                                output += line + '\n';
                            }
                        }
                        
                        if (output.isEmpty()) {
                            diffDisplay->setPlainText("File not found in commit diff.");
                        } else {
                            // Remove trailing newline if present
                            if (output.endsWith('\n')) {
                                output.chop(1);
                            }
                            diffDisplay->setPlainText(output);
                        }
                    } else {
                        diffDisplay->setPlainText("Failed to generate diff.");
                    }
                    git_buf_dispose(&patch_buf);
                    git_diff_free(diff);
                } else {
                    diffDisplay->setPlainText("Failed to create diff.");
                }
            }
            
            if (commit_tree) git_tree_free(commit_tree);
            if (parent_tree) git_tree_free(parent_tree);
            git_commit_free(parent_commit);
        }
    }
    
    git_commit_free(commit);
}

void BottomDockWidget::clearDisplay()
{
    fileList->clear();
    diffDisplay->clear();
    filePathMap.clear();
}
