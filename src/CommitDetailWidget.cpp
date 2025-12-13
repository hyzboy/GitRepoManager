#include "CommitDetailWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDateTime>
#include <QDebug>

CommitDetailWidget::CommitDetailWidget(QWidget *parent)
    : QWidget(parent), currentRepo(nullptr)
{
    setupUI();
}

CommitDetailWidget::~CommitDetailWidget()
{
}

void CommitDetailWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(4);
    
    // Commit Hash
    QHBoxLayout *hashLayout = new QHBoxLayout();
    QLabel *hashKeyLabel = new QLabel("Commit Hash:", this);
    hashKeyLabel->setStyleSheet("font-weight: bold;");
    commitHashLabel = new QLabel(this);
    commitHashLabel->setWordWrap(true);
    hashLayout->addWidget(hashKeyLabel);
    hashLayout->addWidget(commitHashLabel);
    hashLayout->addStretch();
    mainLayout->addLayout(hashLayout);
    
    // Author
    QHBoxLayout *authorLayout = new QHBoxLayout();
    QLabel *authorKeyLabel = new QLabel("Author:", this);
    authorKeyLabel->setStyleSheet("font-weight: bold;");
    authorLabel = new QLabel(this);
    authorLayout->addWidget(authorKeyLabel);
    authorLayout->addWidget(authorLabel);
    authorLayout->addStretch();
    mainLayout->addLayout(authorLayout);
    
    // Date
    QHBoxLayout *dateLayout = new QHBoxLayout();
    QLabel *dateKeyLabel = new QLabel("Date:", this);
    dateKeyLabel->setStyleSheet("font-weight: bold;");
    dateLabel = new QLabel(this);
    dateLayout->addWidget(dateKeyLabel);
    dateLayout->addWidget(dateLabel);
    dateLayout->addStretch();
    mainLayout->addLayout(dateLayout);
    
    // Parent Commit
    QHBoxLayout *parentLayout = new QHBoxLayout();
    QLabel *parentKeyLabel = new QLabel("Parent:", this);
    parentKeyLabel->setStyleSheet("font-weight: bold;");
    parentLabel = new QLabel(this);
    parentLabel->setWordWrap(true);
    parentLayout->addWidget(parentKeyLabel);
    parentLayout->addWidget(parentLabel);
    parentLayout->addStretch();
    mainLayout->addLayout(parentLayout);
    
    // Separator
    mainLayout->addSpacing(8);
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);
    
    // Message
    QLabel *messageKeyLabel = new QLabel("Message:", this);
    messageKeyLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(messageKeyLabel);
    
    messageLabel = new QLabel(this);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet("padding: 8px; border: 1px solid palette(mid); border-radius: 4px; background-color: palette(base);");
    mainLayout->addWidget(messageLabel);
    
    // Add stretch to push content to top
    mainLayout->addStretch();
}

void CommitDetailWidget::setRepository(git_repository *repo)
{
    currentRepo = repo;
}

void CommitDetailWidget::displayCommitDetail(const git_oid &oid)
{
    if (!currentRepo) {
        clearDetail();
        return;
    }
    
    // Get commit object
    git_commit *commit = nullptr;
    if (git_commit_lookup(&commit, currentRepo, &oid) != 0) {
        clearDetail();
        return;
    }
    
    // Display commit hash
    char oid_str[GIT_OID_HEXSZ + 1] = {0};
    git_oid_tostr(oid_str, sizeof(oid_str), &oid);
    commitHashLabel->setText(QString::fromUtf8(oid_str));
    
    // Display author
    const git_signature *author = git_commit_author(commit);
    QString authorStr = author ? QString::fromUtf8(author->name) : "Unknown";
    authorLabel->setText(authorStr);
    
    // Display date
    git_time_t commit_time = git_commit_time(commit);
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(commit_time * 1000);
    QString dateStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    dateLabel->setText(dateStr);
    
    // Display parent commit(s)
    unsigned int parent_count = git_commit_parentcount(commit);
    if (parent_count == 0) {
        parentLabel->setText("(Initial commit)");
    } else if (parent_count == 1) {
        const git_oid *parent_oid = git_commit_parent_id(commit, 0);
        char parent_str[GIT_OID_HEXSZ + 1] = {0};
        git_oid_tostr(parent_str, sizeof(parent_str), parent_oid);
        parentLabel->setText(QString::fromUtf8(parent_str));
    } else {
        QString parentStr;
        for (unsigned int i = 0; i < parent_count; ++i) {
            if (i > 0) parentStr += ", ";
            const git_oid *parent_oid = git_commit_parent_id(commit, i);
            char parent_str[GIT_OID_HEXSZ + 1] = {0};
            git_oid_tostr(parent_str, sizeof(parent_str), parent_oid);
            parentStr += QString::fromUtf8(parent_str);
        }
        parentLabel->setText(parentStr);
    }
    
    // Display message
    const char *message = git_commit_message(commit);
    messageLabel->setText(QString::fromUtf8(message));
    
    git_commit_free(commit);
}

void CommitDetailWidget::clearDetail()
{
    commitHashLabel->setText("");
    authorLabel->setText("");
    dateLabel->setText("");
    messageLabel->setText("");
    parentLabel->setText("");
}
