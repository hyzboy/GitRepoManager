#ifndef COMMITSDOCKWIDGET_H
#define COMMITSDOCKWIDGET_H

#include <QDockWidget>
#include <QTableWidget>
#include <git2.h>
#include <QMap>

class CommitsDockWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit CommitsDockWidget(QWidget *parent = nullptr);
    ~CommitsDockWidget();

    void loadCommits(git_repository *repo);
    void clearCommits();

signals:
    void commitSelected(const git_oid &oid);

private slots:
    void onCommitSelectionChanged();

private:
    void setupUI();
    void populateCommitTable(git_repository *repo);

    // UI Components
    QTableWidget *commitTable;
    QMap<int, git_oid> commitOidMap;  // Map row index to commit OID
};

#endif // COMMITSDOCKWIDGET_H
