#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <git2.h>
#include "RepositoryTreeWidget.h"
#include "CommitDetailWidget.h"
#include "CommitsDockWidget.h"
#include "CommitDiffWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenRepository();
    void showAbout();

private:
    void setupUI();
    void createLeftPanels();
    void createRightPanels();
    void createToolBar();
    bool openRepositoryPath(const QString &repoPath);

    // UI Components
    RepositoryTreeWidget *repositoryTree;
    CommitDetailWidget *commitDetailWidget;
    CommitsDockWidget *commitsDockWidget;
    CommitDiffWidget *bottomDockWidget;  // Keep variable name for now to minimize changes
    
    // Git repository
    git_repository *currentRepo;
};

#endif // MAINWINDOW_H
