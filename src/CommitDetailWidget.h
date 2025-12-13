#ifndef COMMITDETAILWIDGET_H
#define COMMITDETAILWIDGET_H

#include <QWidget>

class CommitDetailWidget : public QWidget {
    Q_OBJECT

public:
    explicit CommitDetailWidget(QWidget *parent = nullptr);
    ~CommitDetailWidget();

private:
    void setupUI();
};

#endif // COMMITDETAILWIDGET_H
