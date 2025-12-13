#include "DLG_About.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QString>
#include <git2.h>

DLG_About::DLG_About(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About Git Repository Manager");
    setMinimumSize(400, 200);
    setModal(true);

    // Get libgit2 version
    int major, minor, rev;
    git_libgit2_version(&major, &minor, &rev);
    QString libgit2Version = QString("libgit2 version: %1.%2.%3")
        .arg(major)
        .arg(minor)
        .arg(rev);

    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Git Repository Manager", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *verLabel = new QLabel(QString("version: 0.1"), this);
    verLabel->setAlignment(Qt::AlignCenter);

    QLabel *qtLabel = new QLabel(QString("Qt version: %1").arg(QT_VERSION_STR), this);
    qtLabel->setAlignment(Qt::AlignCenter);

    QLabel *git2Label = new QLabel(libgit2Version, this);
    git2Label->setAlignment(Qt::AlignCenter);

    QLabel *statusLabel = new QLabel("✓ Qt6 and libgit2 successfully integrated!", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: green; font-weight: bold;");

    layout->addStretch();
    layout->addWidget(titleLabel);
    layout->addWidget(verLabel);
    layout->addWidget(qtLabel);
    layout->addWidget(git2Label);
    layout->addWidget(statusLabel);
    layout->addStretch();
}

DLG_About::~DLG_About()
{
}
