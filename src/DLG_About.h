#ifndef DLG_ABOUT_H
#define DLG_ABOUT_H

#include <QDialog>

class DLG_About : public QDialog {
    Q_OBJECT

public:
    explicit DLG_About(QWidget *parent = nullptr);
    ~DLG_About();
};

#endif // DLG_ABOUT_H
