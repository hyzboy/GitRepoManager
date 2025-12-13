#include "MainWindow.h"
#include "DLG_About.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QStandardItemModel>
#include <QPushButton>
#include <QToolBar>
#include <QFontMetrics>

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
    mainSplitter->addWidget(treeView);

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
    
    // Set initial sizes: left panel ~20 character widths, right panel fills remaining space
    QFontMetrics fm(treeView->font());
    int charWidth = fm.averageCharWidth();
    int leftPanelWidth = charWidth * 30;
    mainSplitter->setSizes(QList<int>() << leftPanelWidth << (800 - leftPanelWidth));

    mainLayout->addWidget(mainSplitter);

    // Create toolbar
    QToolBar *toolBar = addToolBar("Tools");
    QPushButton *aboutButton = new QPushButton("About", this);
    connect(aboutButton, &QPushButton::clicked, this, &MainWindow::showAbout);
    toolBar->addWidget(aboutButton);

    setCentralWidget(centralWidget);
}

void MainWindow::createLeftPanel()
{
    // Create TreeView for left panel
    treeView = new QTreeView(this);
    
    // Create a simple model for the tree
    QStandardItemModel *model = new QStandardItemModel(this);
    QStandardItem *rootItem = model->invisibleRootItem();
    
    // Add some example items
    QStandardItem *item1 = new QStandardItem("Repositories");
    QStandardItem *item2 = new QStandardItem("Recent");
    QStandardItem *item3 = new QStandardItem("Favorites");
    
    rootItem->appendRow(item1);
    rootItem->appendRow(item2);
    rootItem->appendRow(item3);
    
    treeView->setModel(model);
    treeView->expandAll();
}

void MainWindow::showAbout()
{
    DLG_About about(this);
    about.exec();
}
