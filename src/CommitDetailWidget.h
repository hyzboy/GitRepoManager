#ifndef COMMITDETAILWIDGET_H
#define COMMITDETAILWIDGET_H

#include <QWidget>
#include <git2.h>

class QLabel;

class CommitDetailWidget : public QWidget {
    Q_OBJECT

public:
    explicit CommitDetailWidget(QWidget *parent = nullptr);
    ~CommitDetailWidget();

    void setRepository(git_repository *repo);

public slots:
    void displayCommitDetail(const git_oid &oid);

private:
    void setupUI();
    void clearDetail();

    // UI Components
    QLabel *commitHashLabel;
    QLabel *authorLabel;
    QLabel *dateLabel;
    QLabel *messageLabel;
    QLabel *parentLabel;
    
    // Repository pointer
    git_repository *currentRepo;
};

#endif // COMMITDETAILWIDGET_H
