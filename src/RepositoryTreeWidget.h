#ifndef REPOSITORYTREEWIDGET_H
#define REPOSITORYTREEWIDGET_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <git2.h>

class RepositoryTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit RepositoryTreeWidget(QWidget *parent = nullptr);
    ~RepositoryTreeWidget();

    void addRepository(const QString &repoPath, git_repository *repo = nullptr);
    void clearRepositories();

private:
    void setupTree();
    QTreeWidgetItem *createRepositoryNode(const QString &repoPath, git_repository *repo);
    void addBranchesNode(QTreeWidgetItem *repoItem, git_repository *repo);
    void addTagsNode(QTreeWidgetItem *repoItem, git_repository *repo);
    void addRemotesNode(QTreeWidgetItem *repoItem, git_repository *repo);
    void addSubmodulesNode(QTreeWidgetItem *repoItem, git_repository *repo);

    // UI Components
    QTreeWidgetItem *rootItem;
};

#endif // REPOSITORYTREEWIDGET_H
