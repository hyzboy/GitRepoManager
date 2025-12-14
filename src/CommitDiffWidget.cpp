#include "CommitDiffWidget.h"
#include "SyntaxManager.h"
#include "ThemeManager.h"
#include "SyntaxHighlighter.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

CommitDiffWidget::CommitDiffWidget(QWidget *parent)
    : QDockWidget("Commit Changes", parent), currentRepo(nullptr), syntaxManager(nullptr), themeManager(nullptr), themeComboBox(nullptr), syntaxComboBox(nullptr), syntaxHighlighter(nullptr)
{
    setupUI();
    
    // Initialize syntax manager
    syntaxManager = new SyntaxManager(this);
    loadSyntaxDefinitions();
    
    // Initialize theme manager
    themeManager = new ThemeManager(this);
    loadThemeDefinitions();
    
    // Initialize syntax highlighter with the diffDisplay document
    syntaxHighlighter = new SyntaxHighlighter(diffDisplay->document());
    
    // Populate combo boxes after managers are initialized
    populateThemeComboBox();
    populateSyntaxComboBox();
}

CommitDiffWidget::~CommitDiffWidget()
{
    // SyntaxManager, ThemeManager, and SyntaxHighlighter will be automatically deleted as they are children of this widget
}

void CommitDiffWidget::loadSyntaxDefinitions()
{
    // Try multiple possible paths for syntax definitions
    QStringList possiblePaths;
    
    // 1. Path relative to application directory
    QString appDir = QCoreApplication::applicationDirPath() + "/syntax";
    
    bool loaded = false;
    QDir dir(appDir);

    if (dir.exists())
    {
        qDebug() << "Attempting to load syntax definitions from:" << dir.absolutePath();
        if (syntaxManager->loadSyntaxDirectory(dir.absolutePath())) {
            qDebug() << "Successfully loaded" << syntaxManager->syntaxCount() 
                        << "syntax definitions from:" << dir.absolutePath();
            loaded = true;
        }
    }
    
    if (!loaded)
    {
        qWarning() << "Failed to load syntax definitions from any known path";
        qDebug() << "Tried paths:";
        qDebug() << "  -" << appDir;
    }
}

void CommitDiffWidget::loadThemeDefinitions()
{
    // Try multiple possible paths for theme definitions
    QStringList possiblePaths;
    
    // 1. Path relative to application directory
    QString appDir = QCoreApplication::applicationDirPath() + "/themes";
    
    bool loaded = false;
    QDir dir(appDir);

    if (dir.exists())
    {
        qDebug() << "Attempting to load theme definitions from:" << dir.absolutePath();
        if (themeManager->loadThemeDirectory(dir.absolutePath())) {
            qDebug() << "Successfully loaded" << themeManager->themeCount() 
                        << "theme definitions from:" << dir.absolutePath();
            loaded = true;
        }
    }
    
    if (!loaded) {
        qWarning() << "Failed to load theme definitions from any known path";
        qDebug() << "Tried paths:";
        qDebug() << "  -" << appDir;
    }
}

void CommitDiffWidget::populateThemeComboBox()
{
    if (!themeComboBox || !themeManager) {
        return;
    }
    
    // Populate theme combo box
    QStringList themes = themeManager->getAvailableThemeNames();
    themeComboBox->addItems(themes);
    
    // Set current theme
    QString currentTheme = themeManager->getActiveThemeName();
    if (!currentTheme.isEmpty()) {
        int index = themeComboBox->findText(currentTheme);
        if (index >= 0) {
            themeComboBox->setCurrentIndex(index);
        }
    }
}

void CommitDiffWidget::populateSyntaxComboBox()
{
    if (!syntaxComboBox || !syntaxManager) {
        return;
    }
    
    // Add "Auto" as first option for automatic syntax detection
    syntaxComboBox->addItem("Auto");
    
    // Populate syntax combo box with available syntax definitions
    QStringList syntaxNames = syntaxManager->getAvailableSyntaxNames();
    syntaxNames.sort(Qt::CaseInsensitive);
    syntaxComboBox->addItems(syntaxNames);
    
    // Set "Auto" as default
    syntaxComboBox->setCurrentIndex(0);
}

void CommitDiffWidget::onThemeChanged(int index)
{
    if (index < 0 || !themeComboBox || !themeManager) {
        return;
    }
    
    QString themeName = themeComboBox->currentText();
    if (!themeName.isEmpty()) {
        themeManager->setActiveTheme(themeName);
        qDebug() << "Theme changed to:" << themeName;
        
        // Apply theme to diffDisplay
        applySyntaxHighlighting();
    }
}

void CommitDiffWidget::onSyntaxChanged(int index)
{
    if (index < 0 || !syntaxComboBox) {
        return;
    }
    
    QString syntaxName = syntaxComboBox->currentText();
    qDebug() << "Syntax changed to:" << syntaxName;
    
    // If user manually changes syntax, re-display the current file with new syntax
    // Get the currently selected file
    QListWidgetItem *item = fileList->currentItem();
    if (item) {
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
        
        // If we have a file path and user selected "Auto", re-detect syntax
        if (!filePath.isEmpty() && index == 0) {
            // Re-trigger showFileDiff to auto-detect syntax
            showFileDiff(filePath);
        } else if (!filePath.isEmpty()) {
            // User manually selected a syntax, apply it
            applySyntaxHighlighting();
        }
    }
}

void CommitDiffWidget::setupUI()
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
    connect(fileList, &QListWidget::currentRowChanged, this, &CommitDiffWidget::onFileSelected);
    
    // Set minimum and initial width for file list based on character width
    QFontMetrics fm(fileList->font());
    int charWidth = fm.averageCharWidth();
    int fileListWidth = charWidth * 32;
    leftPanel->setMinimumWidth(fileListWidth / 2); // Minimum 16 chars
    
    leftLayout->addWidget(fileList);
    
    mainSplitter->addWidget(leftPanel);
    
    // Right panel - Diff Display with Theme and Syntax Selectors
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    // Header row with diff label and selectors
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(4, 4, 4, 4);
    
    QLabel *diffLabel = new QLabel("File Diff:", this);
    diffLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    headerLayout->addWidget(diffLabel);
    
    headerLayout->addStretch();
    
    // Theme selector
    QLabel *themeLabel = new QLabel("Theme:", this);
    headerLayout->addWidget(themeLabel);
    
    themeComboBox = new QComboBox(this);
    themeComboBox->setMinimumWidth(150);
    connect(themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CommitDiffWidget::onThemeChanged);
    headerLayout->addWidget(themeComboBox);
    
    // Syntax selector
    QLabel *syntaxLabel = new QLabel("Syntax:", this);
    headerLayout->addWidget(syntaxLabel);
    
    syntaxComboBox = new QComboBox(this);
    syntaxComboBox->setMinimumWidth(150);
    connect(syntaxComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CommitDiffWidget::onSyntaxChanged);
    headerLayout->addWidget(syntaxComboBox);

    rightLayout->addLayout(headerLayout);
    
    // Diff display
    diffDisplay = new QTextEdit(this);
    diffDisplay->setReadOnly(true);
    diffDisplay->setFont(QFont("Consolas", 10)); // Use monospace font for diff
    diffDisplay->setLineWrapMode(QTextEdit::NoWrap);
    diffDisplay->setPlaceholderText("Select a file to view changes...");
    rightLayout->addWidget(diffDisplay);
    
    mainSplitter->addWidget(rightPanel);
    
    // Set stretch factors: file list gets less space, diff display gets more
    // Ratio approximately 1:3 (file list : diff display)
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 4);
    
    // Set initial sizes based on character width
    // This will be used as a hint when the widget is first shown
    mainSplitter->setSizes(QList<int>() << fileListWidth << (fileListWidth * 4));
    
    mainLayout->addWidget(mainSplitter);
    setWidget(content);
}

void CommitDiffWidget::setRepository(git_repository *repo)
{
    currentRepo = repo;
}

void CommitDiffWidget::displayCommitFiles(const git_oid &oid)
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

void CommitDiffWidget::populateFileList(git_commit *commit)
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

void CommitDiffWidget::onFileSelected()
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

void CommitDiffWidget::applySyntaxHighlighting()
{
    if (!syntaxHighlighter || !syntaxManager || !themeManager) {
        return;
    }
    
    // Get current syntax from combo box
    QString syntaxName = syntaxComboBox ? syntaxComboBox->currentText() : "";
    
    // If "Auto" is selected or no syntax selected, try to detect from current file
    if (syntaxName.isEmpty() || syntaxName == "Auto") {
        if (!currentFilePath.isEmpty()) {
            SyntaxDefinition syntaxDef = syntaxManager->getSyntaxByFilename(currentFilePath);
            if (!syntaxDef.name.isEmpty()) {
                syntaxHighlighter->setSyntaxDefinition(syntaxDef);
                qDebug() << "Applied auto-detected syntax:" << syntaxDef.name;
            }
        }
    } else {
        // Use manually selected syntax
        SyntaxDefinition syntaxDef = syntaxManager->getSyntaxByName(syntaxName);
        if (!syntaxDef.name.isEmpty()) {
            syntaxHighlighter->setSyntaxDefinition(syntaxDef);
            qDebug() << "Applied selected syntax:" << syntaxDef.name;
        }
    }
    
    // Apply current theme
    QString themeName = themeComboBox ? themeComboBox->currentText() : "";
    if (!themeName.isEmpty()) {
        ThemeLoader *themeLoader = themeManager->getTheme(themeName);
        if (themeLoader) {
            syntaxHighlighter->setTheme(themeLoader);
            
            // Also apply editor background color
            EditorColors colors = themeLoader->getEditorColors();
            QPalette palette = diffDisplay->palette();
            palette.setColor(QPalette::Base, colors.backgroundColor);
            palette.setColor(QPalette::Text, colors.textColor);
            diffDisplay->setPalette(palette);
            
            qDebug() << "Applied theme:" << themeName;
        }
    }
    
    // Force rehighlight
    syntaxHighlighter->rehighlight();
}

void CommitDiffWidget::showFileDiff(const QString &filePath)
{
    if (!currentRepo || filePath.isEmpty()) {
        diffDisplay->setPlainText("No file selected or repository not available.");
        return;
    }
    
    // Store current file path
    currentFilePath = filePath;
    
    // Auto-select syntax based on file extension if in Auto mode
    if (syntaxComboBox && syntaxManager && syntaxComboBox->currentIndex() == 0) {
        // Get syntax definition for this file
        SyntaxDefinition syntaxDef = syntaxManager->getSyntaxByFilename(filePath);
        if (!syntaxDef.name.isEmpty()) {
            // Find and select the syntax in the combo box
            int syntaxIndex = syntaxComboBox->findText(syntaxDef.name);
            if (syntaxIndex > 0) { // > 0 to skip "Auto" at index 0
                // Temporarily block signals to avoid triggering onSyntaxChanged
                syntaxComboBox->blockSignals(true);
                syntaxComboBox->setCurrentIndex(syntaxIndex);
                syntaxComboBox->blockSignals(false);
                
                qDebug() << "Auto-selected syntax:" << syntaxDef.name << "for file:" << filePath;
            }
        }
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
    
    // Apply syntax highlighting after text is set
    applySyntaxHighlighting();
}

void CommitDiffWidget::clearDisplay()
{
    fileList->clear();
    diffDisplay->clear();
    filePathMap.clear();
}
