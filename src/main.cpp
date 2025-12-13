#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QString>
#include <git2.h>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Git Repository Manager");
        setMinimumSize(400, 200);

        // Get libgit2 version
        int major, minor, rev;
        git_libgit2_version(&major, &minor, &rev);
        QString libgit2Version = QString("libgit2 version: %1.%2.%3")
            .arg(major)
            .arg(minor)
            .arg(rev);

        // Create central widget with layout
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        QLabel *titleLabel = new QLabel("Git Repository Manager", this);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignCenter);

        QLabel *qtLabel = new QLabel(QString("Qt version: %1").arg(QT_VERSION_STR), this);
        qtLabel->setAlignment(Qt::AlignCenter);

        QLabel *git2Label = new QLabel(libgit2Version, this);
        git2Label->setAlignment(Qt::AlignCenter);

        QLabel *statusLabel = new QLabel("✓ Qt6 and libgit2 successfully integrated!", this);
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setStyleSheet("color: green; font-weight: bold;");

        layout->addStretch();
        layout->addWidget(titleLabel);
        layout->addWidget(qtLabel);
        layout->addWidget(git2Label);
        layout->addWidget(statusLabel);
        layout->addStretch();

        setCentralWidget(centralWidget);
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
