#ifndef COMMITSDOCKWIDGET_H
#define COMMITSDOCKWIDGET_H

#include <QDockWidget>
#include <QTableWidget>
#include <git2.h>

class CommitsDockWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit CommitsDockWidget(QWidget *parent = nullptr);
    ~CommitsDockWidget();

    void loadCommits(git_repository *repo);
    void clearCommits();

private:
    void setupUI();
    void populateCommitTable(git_repository *repo);

    // UI Components
    QTableWidget *commitTable;
};

#endif // COMMITSDOCKWIDGET_H
