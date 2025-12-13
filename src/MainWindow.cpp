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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Git Repository Manager");
    setMinimumSize(800, 600);

    setupUI();
}

MainWindow::~MainWindow()
{
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
    
    // Create top dock widget
    topDockWidget = new QDockWidget("Panel 1", this);
    QWidget *topDockContent = new QWidget();
    QVBoxLayout *topLayout = new QVBoxLayout(topDockContent);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(new QWidget());  // Empty placeholder
    topDockWidget->setWidget(topDockContent);
    
    // Create bottom dock widget
    bottomDockWidget = new QDockWidget("Panel 2", this);
    QWidget *bottomDockContent = new QWidget();
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomDockContent);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->addWidget(new QWidget());  // Empty placeholder
    bottomDockWidget->setWidget(bottomDockContent);

    // Add dock widgets to right splitter
    rightSplitter->addWidget(topDockWidget);
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
    // TODO: Implement open repository dialog
    qDebug() << "Open Repository button clicked";
}
