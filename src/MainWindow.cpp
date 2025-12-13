#include "MainWindow.h"
#include "DLG_About.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QToolBar>
#include <QFontMetrics>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentRepo(nullptr)
{
    setWindowTitle("Git Repository Manager");
    setMinimumSize(800, 600);

    setupUI();
}

MainWindow::~MainWindow()
{
    // Close repository if it's open
    if (currentRepo) {
        git_repository_free(currentRepo);
        currentRepo = nullptr;
    }
}

void MainWindow::setupUI()
{
    // Create main central widget
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create horizontal splitter for left and right panels
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);

    // Create left panel
    createLeftPanel();
    mainSplitter->addWidget(repositoryTree);

    // Create vertical splitter for right dock panels
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical);
    
    // Create right panels
    createRightPanels();
    rightSplitter->addWidget(commitsDockWidget);
    rightSplitter->addWidget(bottomDockWidget);
    rightSplitter->setOrientation(Qt::Vertical);

    // Add both splitters to main splitter
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    
    // Set initial sizes: left panel ~30 character widths, right panel fills remaining space
    QFontMetrics fm(repositoryTree->font());
    int charWidth = fm.averageCharWidth();
    int leftPanelWidth = charWidth * 30;
    mainSplitter->setSizes(QList<int>() << leftPanelWidth << (800 - leftPanelWidth));

    mainLayout->addWidget(mainSplitter);

    // Create toolbar
    createToolBar();

    setCentralWidget(centralWidget);
}

void MainWindow::createLeftPanel()
{
    // Create RepositoryTreeWidget for left panel
    repositoryTree = new RepositoryTreeWidget(this);
    
    // Add an example repository (this can be replaced with actual git operations)
    repositoryTree->addRepository("Current Repository");
}

void MainWindow::createRightPanels()
{
    // Create commits dock widget
    commitsDockWidget = new CommitsDockWidget(this);
    
    // Create bottom dock widget
    bottomDockWidget = new BottomDockWidget(this);
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = addToolBar("Tools");
    
    QPushButton *openButton = new QPushButton("Open", this);
    connect(openButton, &QPushButton::clicked, this, &MainWindow::onOpenRepository);
    toolBar->addWidget(openButton);
    
    toolBar->addSeparator();
    
    QPushButton *aboutButton = new QPushButton("About", this);
    connect(aboutButton, &QPushButton::clicked, this, &MainWindow::showAbout);
    toolBar->addWidget(aboutButton);
}

void MainWindow::showAbout()
{
    DLG_About about(this);
    about.exec();
}

void MainWindow::onOpenRepository()
{
    QString dirPath = QFileDialog::getExistingDirectory(this,
        tr("Open Git Repository"), "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dirPath.isEmpty()) {
        if (openRepositoryPath(dirPath)) {
            repositoryTree->clearRepositories();
            repositoryTree->addRepository(dirPath, currentRepo);
            
            // Load commits into the commits dock widget
            if (commitsDockWidget) {
                commitsDockWidget->loadCommits(currentRepo);
            }
        }
    }
}

bool MainWindow::openRepositoryPath(const QString &repoPath)
{
    // Close previous repository if open
    if (currentRepo) {
        git_repository_free(currentRepo);
        currentRepo = nullptr;
    }
    
    // Convert QString to UTF-8 for libgit2
    QByteArray repoPathBytes = repoPath.toUtf8();
    const char *path = repoPathBytes.constData();
    
    // Try to open repository
    int error = git_repository_open(&currentRepo, path);
    
    if (error != 0) {
        const git_error *giterr = git_error_last();
        QString errorMsg = QString("Failed to open repository:\n%1")
            .arg(giterr ? giterr->message : "Unknown error");
        QMessageBox::critical(this, "Error", errorMsg);
        return false;
    }
    
    // Successfully opened repository
    QString repoName = QFileInfo(repoPath).fileName();
    setWindowTitle(QString("Git Repository Manager - %1").arg(repoName));
    qDebug() << "Repository opened:" << repoPath;
    
    return true;
}
