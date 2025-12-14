#ifndef BOTTOMDOCKWIDGET_H
#define BOTTOMDOCKWIDGET_H

#include <QDockWidget>
#include <QListWidget>
#include <QTextEdit>
#include <git2.h>
#include <QMap>

class QSplitter;
class QComboBox;
class SyntaxManager;
class ThemeManager;
class SyntaxHighlighter;

class BottomDockWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit BottomDockWidget(QWidget *parent = nullptr);
    ~BottomDockWidget();

    void setRepository(git_repository *repo);
    
    // Theme management
    ThemeManager* getThemeManager() const { return themeManager; }

public slots:
    void displayCommitFiles(const git_oid &oid);

private slots:
    void onFileSelected();
    void onThemeChanged(int index);
    void onSyntaxChanged(int index);

private:
    void setupUI();
    void clearDisplay();
    void populateFileList(git_commit *commit);
    void showFileDiff(const QString &filePath);
    void loadSyntaxDefinitions();
    void loadThemeDefinitions();
    void populateThemeComboBox();
    void populateSyntaxComboBox();
    void applySyntaxHighlighting();

    // UI Components
    QSplitter *mainSplitter;
    QListWidget *fileList;
    QTextEdit *diffDisplay;
    QComboBox *themeComboBox;
    QComboBox *syntaxComboBox;
    
    // Git data
    git_repository *currentRepo;
    git_oid currentCommitOid;
    QMap<int, QString> filePathMap;  // Map row index to file path
    
    // Syntax highlighting
    SyntaxManager *syntaxManager;
    ThemeManager *themeManager;
    SyntaxHighlighter *syntaxHighlighter;
    
    // Current file path for re-applying highlighting
    QString currentFilePath;
};

#endif // BOTTOMDOCKWIDGET_H
