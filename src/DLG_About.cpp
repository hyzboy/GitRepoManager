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
    {
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignCenter);
    }

    QLabel *verLabel = new QLabel(QString("Version: 0.1"), this);
    verLabel->setAlignment(Qt::AlignCenter);

    QLabel *authorLabel = new QLabel(QString("Author: YingzhuoHu"), this);
    {
        authorLabel->setAlignment(Qt::AlignCenter);
        //QFont authorFont=authorLabel->font();
        //authorFont.setBold(true);
        //authorLabel->setFont(authorFont);
        authorLabel->setStyleSheet("color: yellow; font-weight: bold;");
    }

    QLabel *qtLabel = new QLabel(QString("Qt version: %1").arg(QT_VERSION_STR), this);
    qtLabel->setAlignment(Qt::AlignCenter);

    QLabel *git2Label = new QLabel(libgit2Version, this);
    git2Label->setAlignment(Qt::AlignCenter);

    QLabel *repoLinkLabel = new QLabel("<a href=\"https://github.com/hyzboy/GitRepoManager.git\">https://github.com/hyzboy/GitRepoManager.git</a>", this);
    {
        repoLinkLabel->setAlignment(Qt::AlignCenter);
        repoLinkLabel->setTextFormat(Qt::RichText);
        repoLinkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        repoLinkLabel->setOpenExternalLinks(true);
    }

    layout->addStretch();
    layout->addWidget(titleLabel);
    layout->addWidget(authorLabel);
    layout->addWidget(verLabel);
    layout->addWidget(qtLabel);
    layout->addWidget(git2Label);
    layout->addWidget(repoLinkLabel);
    layout->addStretch();
}

DLG_About::~DLG_About()
{
}
