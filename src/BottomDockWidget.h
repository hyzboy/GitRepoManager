#ifndef BOTTOMDOCKWIDGET_H
#define BOTTOMDOCKWIDGET_H

#include <QDockWidget>
#include <QListWidget>
#include <QTextEdit>
#include <git2.h>
#include <QMap>

class QSplitter;

class BottomDockWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit BottomDockWidget(QWidget *parent = nullptr);
    ~BottomDockWidget();

    void setRepository(git_repository *repo);

public slots:
    void displayCommitFiles(const git_oid &oid);

private slots:
    void onFileSelected();

private:
    void setupUI();
    void clearDisplay();
    void populateFileList(git_commit *commit);
    void showFileDiff(const QString &filePath);

    // UI Components
    QSplitter *mainSplitter;
    QListWidget *fileList;
    QTextEdit *diffDisplay;
    
    // Git data
    git_repository *currentRepo;
    git_oid currentCommitOid;
    QMap<int, QString> filePathMap;  // Map row index to file path
};

#endif // BOTTOMDOCKWIDGET_H
