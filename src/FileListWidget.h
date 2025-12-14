#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QMap>
#include "GitDiffProvider.h"

class QVBoxLayout;
class QLabel;

class FileListWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit FileListWidget(QWidget *parent = nullptr);
    ~FileListWidget();
    
    // Set the list of changed files
    void setFiles(const QList<FileChange> &files);
    
    // Set a message for special cases (e.g., initial commit)
    void setMessage(const QString &message);
    
    // Get currently selected file path
    QString currentFilePath() const;
    
    // Clear the list
    void clear();
    
signals:
    void fileSelected(const QString &filePath);
    
private slots:
    void onCurrentRowChanged(int currentRow);
    
private:
    void setupUI();
    
    QListWidget *m_fileList;
    QMap<int, QString> m_filePathMap;  // Map row index to file path
};

#endif // FILELISTWIDGET_H
