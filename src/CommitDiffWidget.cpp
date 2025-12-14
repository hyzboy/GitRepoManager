#include "CommitDiffWidget.h"
#include "SyntaxManager.h"
#include "ThemeManager.h"
#include "GitDiffProvider.h"
#include "FileListWidget.h"
#include "DiffViewWidget.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFontMetrics>

CommitDiffWidget::CommitDiffWidget(QWidget *parent)
    : QDockWidget("Commit Changes", parent)
    , m_mainSplitter(nullptr)
    , m_fileListWidget(nullptr)
    , m_diffViewWidget(nullptr)
    , m_diffProvider(nullptr)
    , m_currentRepo(nullptr)
    , m_syntaxManager(nullptr)
    , m_themeManager(nullptr)
{
    // Initialize Git diff provider
    m_diffProvider = new GitDiffProvider(this);
    
    // Initialize syntax manager
    m_syntaxManager = new SyntaxManager(this);
    loadSyntaxDefinitions();
    
    // Initialize theme manager
    m_themeManager = new ThemeManager(this);
    loadThemeDefinitions();
    
    // Setup UI
    setupUI();
    
    // Connect managers to view
    m_diffViewWidget->setSyntaxManager(m_syntaxManager);
    m_diffViewWidget->setThemeManager(m_themeManager);
}

CommitDiffWidget::~CommitDiffWidget()
{
}

void CommitDiffWidget::loadSyntaxDefinitions()
{
    QString appDir = QCoreApplication::applicationDirPath() + "/syntax";
    QDir dir(appDir);

    if (dir.exists())
    {
        qDebug() << "Attempting to load syntax definitions from:" << dir.absolutePath();
        if (m_syntaxManager->loadSyntaxDirectory(dir.absolutePath())) {
            qDebug() << "Successfully loaded" << m_syntaxManager->syntaxCount() 
                        << "syntax definitions from:" << dir.absolutePath();
        }
    }
    else
    {
        qWarning() << "Failed to load syntax definitions from:" << appDir;
    }
}

void CommitDiffWidget::loadThemeDefinitions()
{
    QString appDir = QCoreApplication::applicationDirPath() + "/themes";
    QDir dir(appDir);

    if (dir.exists())
    {
        qDebug() << "Attempting to load theme definitions from:" << dir.absolutePath();
        if (m_themeManager->loadThemeDirectory(dir.absolutePath())) {
            qDebug() << "Successfully loaded" << m_themeManager->themeCount() 
                        << "theme definitions from:" << dir.absolutePath();
        }
    }
    else
    {
        qWarning() << "Failed to load theme definitions from:" << appDir;
    }
}

void CommitDiffWidget::setupUI()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    
    // Create horizontal splitter for file list and diff display
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Create file list widget
    m_fileListWidget = new FileListWidget(this);
    connect(m_fileListWidget, &FileListWidget::fileSelected,
            this, &CommitDiffWidget::onFileSelected);
    m_mainSplitter->addWidget(m_fileListWidget);
    
    // Create diff view widget
    m_diffViewWidget = new DiffViewWidget(this);
    m_mainSplitter->addWidget(m_diffViewWidget);
    
    // Set stretch factors: file list gets less space, diff display gets more
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 4);
    
    // Set initial sizes based on character width
    QFontMetrics fm(m_fileListWidget->font());
    int charWidth = fm.averageCharWidth();
    int fileListWidth = charWidth * 32;
    m_mainSplitter->setSizes(QList<int>() << fileListWidth << (fileListWidth * 4));
    
    mainLayout->addWidget(m_mainSplitter);
    setWidget(content);
}

void CommitDiffWidget::setRepository(git_repository *repo)
{
    m_currentRepo = repo;
    m_diffProvider->setRepository(repo);
}

void CommitDiffWidget::displayCommitFiles(const git_oid &oid)
{
    clearDisplay();
    
    if (!m_currentRepo) {
        return;
    }
    
    // Store current commit OID
    git_oid_cpy(&m_currentCommitOid, &oid);
    
    // Check if this is initial commit
    if (m_diffProvider->isInitialCommit(oid)) {
        m_fileListWidget->setMessage("(Initial commit - all files added)");
        return;
    }
    
    // Get changed files
    QList<FileChange> changes = m_diffProvider->getChangedFiles(oid);
    m_fileListWidget->setFiles(changes);
}

void CommitDiffWidget::onFileSelected(const QString &filePath)
{
    if (filePath.isEmpty() || !m_currentRepo) {
        m_diffViewWidget->clear();
        return;
    }
    
    // Get diff content
    QString content = m_diffProvider->getFileDiff(m_currentCommitOid, filePath);
    
    // Display in diff view
    m_diffViewWidget->setContent(content, filePath);
}

void CommitDiffWidget::clearDisplay()
{
    m_fileListWidget->clear();
    m_diffViewWidget->clear();
}
