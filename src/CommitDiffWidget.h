#ifndef COMMITDIFFWIDGET_H
#define COMMITDIFFWIDGET_H

#include <QDockWidget>
#include <git2.h>

class QSplitter;
class SyntaxManager;
class ThemeManager;
class GitDiffProvider;
class FileListWidget;
class DiffViewWidget;

class CommitDiffWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit CommitDiffWidget(QWidget *parent = nullptr);
    ~CommitDiffWidget();

    void setRepository(git_repository *repo);
    
    // Theme management
    ThemeManager* getThemeManager() const { return m_themeManager; }

public slots:
    void displayCommitFiles(const git_oid &oid);

private slots:
    void onFileSelected(const QString &filePath);

private:
    void setupUI();
    void clearDisplay();
    void loadSyntaxDefinitions();
    void loadThemeDefinitions();

    // UI Components
    QSplitter *m_mainSplitter;
    FileListWidget *m_fileListWidget;
    DiffViewWidget *m_diffViewWidget;
    
    // Git operations
    GitDiffProvider *m_diffProvider;
    git_repository *m_currentRepo;
    git_oid m_currentCommitOid;
    
    // Syntax highlighting
    SyntaxManager *m_syntaxManager;
    ThemeManager *m_themeManager;
};

#endif // COMMITDIFFWIDGET_H
