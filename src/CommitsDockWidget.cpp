#include "CommitsDockWidget.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDateTime>
#include <QDebug>

CommitsDockWidget::CommitsDockWidget(QWidget *parent)
    : QDockWidget("Commits", parent), commitTable(nullptr)
{
    setupUI();
}

CommitsDockWidget::~CommitsDockWidget()
{
}

void CommitsDockWidget::setupUI()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Create table widget
    commitTable = new QTableWidget(this);
    commitTable->setColumnCount(3);
    commitTable->setHorizontalHeaderLabels(QStringList() << "Commit Message" << "Author" << "Date");
    
    // Hide row numbers (vertical header)
    commitTable->verticalHeader()->setVisible(false);
    
    // Hide grid lines
    commitTable->setShowGrid(false);

    // Configure table appearance
    commitTable->horizontalHeader()->setStretchLastSection(false);
    commitTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    commitTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    commitTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    commitTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    commitTable->setSelectionMode(QAbstractItemView::SingleSelection);
    commitTable->setAlternatingRowColors(true);
    
    layout->addWidget(commitTable);
    
    setWidget(content);
}

void CommitsDockWidget::loadCommits(git_repository *repo)
{
    clearCommits();
    if (repo) {
        populateCommitTable(repo);
    }
}

void CommitsDockWidget::clearCommits()
{
    if (commitTable) {
        commitTable->setRowCount(0);
    }
}

void CommitsDockWidget::populateCommitTable(git_repository *repo)
{
    if (!repo) {
        return;
    }
    
    // Get reference to HEAD
    git_oid oid;
    int error = git_reference_name_to_id(&oid, repo, "HEAD");
    if (error != 0) {
        qDebug() << "Failed to get HEAD reference";
        return;
    }
    
    // Create revwalk object for iterating commits
    git_revwalk *walker = nullptr;
    error = git_revwalk_new(&walker, repo);
    if (error != 0) {
        qDebug() << "Failed to create revwalk";
        return;
    }
    
    // Push HEAD to the walker
    git_revwalk_push(walker, &oid);
    
    // Iterate through commits
    int row = 0;
    const int MAX_COMMITS = 100;  // Limit to first 100 commits
    
    while (git_revwalk_next(&oid, walker) == 0 && row < MAX_COMMITS) {
        // Get commit object
        git_commit *commit = nullptr;
        if (git_commit_lookup(&commit, repo, &oid) != 0) {
            continue;
        }
        
        // Insert new row
        commitTable->insertRow(row);
        
        // Get commit message
        const char *message = git_commit_message(commit);
        QTableWidgetItem *messageItem = new QTableWidgetItem(QString::fromUtf8(message).split('\n').first());
        commitTable->setItem(row, 0, messageItem);
        
        // Get author name
        const git_signature *author = git_commit_author(commit);
        QString authorName = author ? QString::fromUtf8(author->name) : "Unknown";
        QTableWidgetItem *authorItem = new QTableWidgetItem(authorName);
        commitTable->setItem(row, 1, authorItem);
        
        // Get commit date
        git_time_t commit_time = git_commit_time(commit);
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(commit_time * 1000);
        QString dateStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
        QTableWidgetItem *dateItem = new QTableWidgetItem(dateStr);
        commitTable->setItem(row, 2, dateItem);
        
        git_commit_free(commit);
        row++;
    }
    
    git_revwalk_free(walker);
    
    qDebug() << "Loaded" << row << "commits";
}
