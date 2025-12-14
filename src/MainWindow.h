#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <git2.h>
#include "RepositoryTreeWidget.h"
#include "CommitDetailWidget.h"
#include "CommitsDockWidget.h"
#include "BottomDockWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showAbout();
    void onOpenRepository();

private:
    void setupUI();
    void createLeftPanels();
    void createToolBar();
    void createRightPanels();
    bool openRepositoryPath(const QString &repoPath);

    // UI Components
    RepositoryTreeWidget *repositoryTree;
    CommitDetailWidget *commitDetailWidget;
    CommitsDockWidget *commitsDockWidget;
    BottomDockWidget *bottomDockWidget;
    
    // Repository data
    git_repository *currentRepo;
};

#endif // MAINWINDOW_H
