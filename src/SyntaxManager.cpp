#include "SyntaxManager.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>

SyntaxManager::SyntaxManager(QObject *parent)
    : QObject(parent)
{
}

bool SyntaxManager::loadSyntaxDirectory(const QString &directoryPath)
{
    QDir syntaxDir(directoryPath);
    if (!syntaxDir.exists()) {
        qWarning() << "Syntax directory does not exist:" << directoryPath;
        return false;
    }

    QStringList xmlFiles = syntaxDir.entryList(QStringList() << "*.xml", QDir::Files);
    
    int successCount = 0;
    for (const QString &filename : xmlFiles) {
        QString filePath = syntaxDir.filePath(filename);
        if (loadSyntaxFile(filePath)) {
            successCount++;
        }
    }
    
    qDebug() << "Loaded" << successCount << "out of" << xmlFiles.size() << "syntax definitions";
    return successCount > 0;
}

bool SyntaxManager::loadSyntaxFile(const QString &filePath)
{
    SyntaxLoader loader;
    SyntaxDefinition definition = loader.loadSyntaxFromFile(filePath);
    
    if (definition.name.isEmpty()) {
        qWarning() << "Failed to load syntax from:" << filePath;
        return false;
    }
    
    // 存储语法定义
    m_syntaxMap[definition.name] = definition;
    
    // 注册扩展名映射
    registerExtensions(definition);
    
    qDebug() << "Loaded syntax:" << definition.name << "with priority:" << definition.priority;
    return true;
}

void SyntaxManager::registerExtensions(const SyntaxDefinition &definition)
{
    // 解析扩展名字符串
    QStringList extensions = definition.extensions.split(';', Qt::SkipEmptyParts);
    
    for (const QString &ext : extensions) {
        QString normalizedExt = normalizeExtension(ext.trimmed());
        
        // 如果这个扩展名还没有注册，创建新的列表
        if (!m_extensionMap.contains(normalizedExt)) {
            m_extensionMap[normalizedExt] = QStringList();
        }
        
        // 按优先级插入
        QStringList &syntaxNames = m_extensionMap[normalizedExt];
        
        // 查找插入位置（按优先级降序）
        int insertIndex = 0;
        for (int i = 0; i < syntaxNames.size(); ++i) {
            const SyntaxDefinition &existingDef = m_syntaxMap[syntaxNames[i]];
            if (definition.priority > existingDef.priority) {
                insertIndex = i;
                break;
            }
            insertIndex = i + 1;
        }
        
        syntaxNames.insert(insertIndex, definition.name);
    }
}

QString SyntaxManager::normalizeExtension(const QString &extension) const
{
    QString normalized = extension.toLower().trimmed();
    
    // 移除通配符 "*."
    if (normalized.startsWith("*.")) {
        normalized = normalized.mid(2);
    }
    // 移除前导点号
    else if (normalized.startsWith(".")) {
        normalized = normalized.mid(1);
    }
    
    return normalized;
}

QString SyntaxManager::normalizeFilename(const QString &filename) const
{
    QFileInfo fileInfo(filename);
    return fileInfo.fileName().toLower();
}

SyntaxDefinition SyntaxManager::getSyntaxByExtension(const QString &extension) const
{
    QString normalizedExt = normalizeExtension(extension);
    
    if (m_extensionMap.contains(normalizedExt)) {
        return selectByPriority(m_extensionMap[normalizedExt]);
    }
    
    // 返回空定义
    return SyntaxDefinition();
}

SyntaxDefinition SyntaxManager::getSyntaxByFilename(const QString &filename) const
{
    QFileInfo fileInfo(filename);
    QString extension = fileInfo.suffix();
    
    return getSyntaxByExtension(extension);
}

SyntaxDefinition SyntaxManager::getSyntaxByName(const QString &name) const
{
    return m_syntaxMap.value(name, SyntaxDefinition());
}

SyntaxDefinition SyntaxManager::selectByPriority(const QStringList &syntaxNames) const
{
    if (syntaxNames.isEmpty()) {
        return SyntaxDefinition();
    }
    
    // 第一个就是优先级最高的（因为我们在插入时已经排序）
    return m_syntaxMap[syntaxNames.first()];
}

QStringList SyntaxManager::getAvailableSyntaxNames() const
{
    return m_syntaxMap.keys();
}

bool SyntaxManager::hasSyntaxForExtension(const QString &extension) const
{
    QString normalizedExt = normalizeExtension(extension);
    return m_extensionMap.contains(normalizedExt);
}
