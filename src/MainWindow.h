#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QComboBox>
#include <git2.h>
#include "RepositoryTreeWidget.h"
#include "CommitsDockWidget.h"
#include "BottomDockWidget.h"
#include "CommitDetailWidget.h"
#include "ThemeManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showAbout();
    void onOpenRepository();
    void onThemeChanged(int index);

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
    QComboBox *themeComboBox;
    
    // Managers
    ThemeManager *themeManager;
    
    // Repository data
    git_repository *currentRepo;
};

#endif // MAINWINDOW_H
