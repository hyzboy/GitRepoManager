#include "FileListWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFontMetrics>

FileListWidget::FileListWidget(QWidget *parent)
    : QWidget(parent)
    , m_fileList(nullptr)
{
    setupUI();
}

FileListWidget::~FileListWidget()
{
}

void FileListWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *label = new QLabel("Changed Files:", this);
    label->setStyleSheet("font-weight: bold; padding: 4px;");
    layout->addWidget(label);
    
    m_fileList = new QListWidget(this);
    connect(m_fileList, &QListWidget::currentRowChanged,
            this, &FileListWidget::onCurrentRowChanged);
    
    // Set minimum width based on character width
    QFontMetrics fm(m_fileList->font());
    int charWidth = fm.averageCharWidth();
    setMinimumWidth(charWidth * 16);  // Minimum 16 chars
    
    layout->addWidget(m_fileList);
}

void FileListWidget::setFiles(const QList<FileChange> &files)
{
    clear();
    
    for (const FileChange &change : files) {
        int row = m_fileList->count();
        QString displayText = change.statusString() + change.path;
        m_fileList->addItem(displayText);
        m_filePathMap[row] = change.path;
    }
}

void FileListWidget::setMessage(const QString &message)
{
    clear();
    m_fileList->addItem(message);
}

QString FileListWidget::currentFilePath() const
{
    int currentRow = m_fileList->currentRow();
    if (currentRow >= 0 && m_filePathMap.contains(currentRow)) {
        return m_filePathMap[currentRow];
    }
    
    // Try to extract from item text (for special cases like initial commit)
    QListWidgetItem *item = m_fileList->currentItem();
    if (item) {
        QString itemText = item->text();
        if (itemText.startsWith("[") && itemText.indexOf("] ") > 0) {
            return itemText.mid(itemText.indexOf("] ") + 2);
        }
    }
    
    return QString();
}

void FileListWidget::clear()
{
    m_fileList->clear();
    m_filePathMap.clear();
}

void FileListWidget::onCurrentRowChanged(int currentRow)
{
    if (currentRow < 0) {
        return;
    }
    
    QString filePath = currentFilePath();
    if (!filePath.isEmpty()) {
        emit fileSelected(filePath);
    }
}
