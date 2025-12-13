#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <git2.h>
#include "DLG_About.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Git Repository Manager");
        setMinimumSize(400, 200);

        // Create central widget with layout
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        QPushButton *aboutButton = new QPushButton("About", this);
        connect(aboutButton, &QPushButton::clicked, this, &MainWindow::showAbout);

        layout->addStretch();
        layout->addWidget(aboutButton, 0, Qt::AlignCenter);
        layout->addStretch();

        setCentralWidget(centralWidget);
    }

private slots:
    void showAbout() {
        DLG_About about(this);
        about.exec();
    }
};

int main(int argc, char *argv[]) {
    // Initialize libgit2 once at application start
    int result = git_libgit2_init();
    if (result < 0) {
        qCritical("Failed to initialize libgit2");
        return 1;
    }

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    int exitCode = app.exec();

    // Shutdown libgit2 once at application end
    git_libgit2_shutdown();

    return exitCode;
}

#include "main.moc"
