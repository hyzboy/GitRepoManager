#include "DiffViewWidget.h"
#include "SyntaxManager.h"
#include "ThemeManager.h"
#include "SyntaxHighlighter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

DiffViewWidget::DiffViewWidget(QWidget *parent)
    : QWidget(parent)
    , m_diffDisplay(nullptr)
    , m_themeComboBox(nullptr)
    , m_syntaxComboBox(nullptr)
    , m_syntaxManager(nullptr)
    , m_themeManager(nullptr)
    , m_highlighter(nullptr)
{
    setupUI();
}

DiffViewWidget::~DiffViewWidget()
{
}

void DiffViewWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Header row with diff label and selectors
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(4, 4, 4, 4);
    
    QLabel *diffLabel = new QLabel("File Diff:", this);
    diffLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    headerLayout->addWidget(diffLabel);
    
    headerLayout->addStretch();
    
    // Theme selector
    QLabel *themeLabel = new QLabel("Theme:", this);
    headerLayout->addWidget(themeLabel);
    
    m_themeComboBox = new QComboBox(this);
    m_themeComboBox->setMinimumWidth(150);
    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DiffViewWidget::onThemeComboChanged);
    headerLayout->addWidget(m_themeComboBox);
    
    // Syntax selector
    QLabel *syntaxLabel = new QLabel("Syntax:", this);
    headerLayout->addWidget(syntaxLabel);
    
    m_syntaxComboBox = new QComboBox(this);
    m_syntaxComboBox->setMinimumWidth(150);
    connect(m_syntaxComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DiffViewWidget::onSyntaxComboChanged);
    headerLayout->addWidget(m_syntaxComboBox);
    
    mainLayout->addLayout(headerLayout);
    
    // Diff display
    m_diffDisplay = new QTextEdit(this);
    m_diffDisplay->setReadOnly(true);
    m_diffDisplay->setFont(QFont("Consolas", 10));
    m_diffDisplay->setLineWrapMode(QTextEdit::NoWrap);
    m_diffDisplay->setPlaceholderText("Select a file to view changes...");
    mainLayout->addWidget(m_diffDisplay);
}

void DiffViewWidget::setSyntaxManager(SyntaxManager *manager)
{
    m_syntaxManager = manager;
    
    // Initialize syntax highlighter
    if (m_syntaxManager && !m_highlighter) {
        m_highlighter = new SyntaxHighlighter(m_diffDisplay->document());
    }
    
    populateSyntaxComboBox();
}

void DiffViewWidget::setThemeManager(ThemeManager *manager)
{
    m_themeManager = manager;
    populateThemeComboBox();
}

void DiffViewWidget::setContent(const QString &content, const QString &filePath)
{
    m_currentFilePath = filePath;
    m_diffDisplay->setPlainText(content);
    
    // Auto-select syntax based on file extension if in Auto mode
    if (m_syntaxComboBox && m_syntaxManager && m_syntaxComboBox->currentIndex() == 0) {
        if (!filePath.isEmpty()) {
            SyntaxDefinition syntaxDef = m_syntaxManager->getSyntaxByFilename(filePath);
            if (!syntaxDef.name.isEmpty()) {
                // Find and select the syntax in the combo box
                int syntaxIndex = m_syntaxComboBox->findText(syntaxDef.name);
                if (syntaxIndex > 0) { // > 0 to skip "Auto" at index 0
                    // Temporarily block signals to avoid triggering onSyntaxComboChanged
                    m_syntaxComboBox->blockSignals(true);
                    m_syntaxComboBox->setCurrentIndex(syntaxIndex);
                    m_syntaxComboBox->blockSignals(false);
                    
                    qDebug() << "Auto-selected syntax:" << syntaxDef.name << "for file:" << filePath;
                }
            }
        }
    }
    
    // Apply syntax highlighting
    applySyntaxHighlighting();
}

void DiffViewWidget::clear()
{
    m_diffDisplay->clear();
    m_currentFilePath.clear();
}

QString DiffViewWidget::currentTheme() const
{
    return m_themeComboBox ? m_themeComboBox->currentText() : QString();
}

QString DiffViewWidget::currentSyntax() const
{
    return m_syntaxComboBox ? m_syntaxComboBox->currentText() : QString();
}

void DiffViewWidget::populateThemeComboBox()
{
    if (!m_themeComboBox || !m_themeManager) {
        return;
    }
    
    // Populate theme combo box
    QStringList themes = m_themeManager->getAvailableThemeNames();
    m_themeComboBox->addItems(themes);
    
    // Set current theme
    QString currentTheme = m_themeManager->getActiveThemeName();
    if (!currentTheme.isEmpty()) {
        int index = m_themeComboBox->findText(currentTheme);
        if (index >= 0) {
            m_themeComboBox->setCurrentIndex(index);
        }
    }
}

void DiffViewWidget::populateSyntaxComboBox()
{
    if (!m_syntaxComboBox || !m_syntaxManager) {
        return;
    }
    
    // Add "Auto" as first option for automatic syntax detection
    m_syntaxComboBox->addItem("Auto");
    
    // Populate syntax combo box with available syntax definitions
    QStringList syntaxNames = m_syntaxManager->getAvailableSyntaxNames();
    syntaxNames.sort(Qt::CaseInsensitive);
    m_syntaxComboBox->addItems(syntaxNames);
    
    // Set "Auto" as default
    m_syntaxComboBox->setCurrentIndex(0);
}

void DiffViewWidget::onThemeComboChanged(int index)
{
    if (index < 0 || !m_themeComboBox || !m_themeManager) {
        return;
    }
    
    QString themeName = m_themeComboBox->currentText();
    if (!themeName.isEmpty()) {
        m_themeManager->setActiveTheme(themeName);
        qDebug() << "Theme changed to:" << themeName;
        
        // Apply theme
        applySyntaxHighlighting();
        
        emit themeChanged(themeName);
    }
}

void DiffViewWidget::onSyntaxComboChanged(int index)
{
    if (index < 0 || !m_syntaxComboBox) {
        return;
    }
    
    QString syntaxName = m_syntaxComboBox->currentText();
    qDebug() << "Syntax changed to:" << syntaxName;
    
    // Apply syntax highlighting
    applySyntaxHighlighting();
    
    emit syntaxChanged(syntaxName);
}

void DiffViewWidget::applySyntaxHighlighting()
{
    if (!m_highlighter || !m_syntaxManager || !m_themeManager) {
        return;
    }
    
    // Get current syntax from combo box
    QString syntaxName = m_syntaxComboBox ? m_syntaxComboBox->currentText() : "";
    
    // If "Auto" is selected or no syntax selected, try to detect from current file
    if (syntaxName.isEmpty() || syntaxName == "Auto") {
        if (!m_currentFilePath.isEmpty()) {
            SyntaxDefinition syntaxDef = m_syntaxManager->getSyntaxByFilename(m_currentFilePath);
            if (!syntaxDef.name.isEmpty()) {
                m_highlighter->setSyntaxDefinition(syntaxDef);
                qDebug() << "Applied auto-detected syntax:" << syntaxDef.name;
            }
        }
    } else {
        // Use manually selected syntax
        SyntaxDefinition syntaxDef = m_syntaxManager->getSyntaxByName(syntaxName);
        if (!syntaxDef.name.isEmpty()) {
            m_highlighter->setSyntaxDefinition(syntaxDef);
            qDebug() << "Applied selected syntax:" << syntaxDef.name;
        }
    }
    
    // Apply current theme
    QString themeName = m_themeComboBox ? m_themeComboBox->currentText() : "";
    if (!themeName.isEmpty()) {
        ThemeLoader *themeLoader = m_themeManager->getTheme(themeName);
        if (themeLoader) {
            m_highlighter->setTheme(themeLoader);
            
            // Also apply editor background color
            EditorColors colors = themeLoader->getEditorColors();
            QPalette palette = m_diffDisplay->palette();
            palette.setColor(QPalette::Base, colors.backgroundColor);
            palette.setColor(QPalette::Text, colors.textColor);
            m_diffDisplay->setPalette(palette);
            
            qDebug() << "Applied theme:" << themeName;
        }
    }
    
    // Force rehighlight
    m_highlighter->rehighlight();
}
