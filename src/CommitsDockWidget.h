#ifndef COMMITSDOCKWIDGET_H
#define COMMITSDOCKWIDGET_H

#include <QDockWidget>

class CommitsDockWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit CommitsDockWidget(QWidget *parent = nullptr);
    ~CommitsDockWidget();

private:
    void setupUI();

    // UI Components
};

#endif // COMMITSDOCKWIDGET_H
