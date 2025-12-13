#include <QApplication>
#include <git2.h>
#include "MainWindow.h"

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
