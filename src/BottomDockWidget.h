#ifndef BOTTOMDOCKWIDGET_H
#define BOTTOMDOCKWIDGET_H

#include <QDockWidget>

class BottomDockWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit BottomDockWidget(QWidget *parent = nullptr);
    ~BottomDockWidget();

private:
    void setupUI();

    // UI Components
};

#endif // BOTTOMDOCKWIDGET_H
