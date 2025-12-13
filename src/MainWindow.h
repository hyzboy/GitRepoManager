#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QDockWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showAbout();

private:
    void setupUI();
    void createLeftPanel();

    // UI Components
    QTreeView *treeView;
    QDockWidget *topDockWidget;
    QDockWidget *bottomDockWidget;
};

#endif // MAINWINDOW_H
