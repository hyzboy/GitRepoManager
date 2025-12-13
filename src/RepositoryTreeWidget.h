#ifndef REPOSITORYTREEWIDGET_H
#define REPOSITORYTREEWIDGET_H

#include <QTreeWidget>
#include <QTreeWidgetItem>

class RepositoryTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit RepositoryTreeWidget(QWidget *parent = nullptr);
    ~RepositoryTreeWidget();

    void addRepository(const QString &repoPath);
    void clearRepositories();

private:
    void setupTree();
    QTreeWidgetItem *createRepositoryNode(const QString &repoPath);
    void addBranchesNode(QTreeWidgetItem *repoItem);
    void addTagsNode(QTreeWidgetItem *repoItem);
    void addRemotesNode(QTreeWidgetItem *repoItem);
    void addSubmodulesNode(QTreeWidgetItem *repoItem);

    // UI Components
    QTreeWidgetItem *rootItem;
};

#endif // REPOSITORYTREEWIDGET_H
