#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include "RepositoryTreeWidget.h"

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
    void createLeftPanel();
    void createToolBar();

    // UI Components
    RepositoryTreeWidget *repositoryTree;
    QDockWidget *topDockWidget;
    QDockWidget *bottomDockWidget;
};

#endif // MAINWINDOW_H
