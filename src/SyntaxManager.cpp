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
    qDebug() << "=== SyntaxManager::loadSyntaxDirectory ===" << directoryPath;
    
    QDir syntaxDir(directoryPath);
    if (!syntaxDir.exists()) {
        qWarning() << "Syntax directory does not exist:" << directoryPath;
        return false;
    }

    // ✅ 关键修复：只初始化一次语法加载器
    qDebug() << "Initializing SyntaxLoader with directory:" << directoryPath;
    m_syntaxLoader.setSyntaxDirectory(directoryPath);

    QStringList xmlFiles = syntaxDir.entryList(QStringList() << "*.xml", QDir::Files);
    
    qDebug() << "Found" << xmlFiles.size() << "XML files in syntax directory";
    
    int successCount = 0;
    int fileIndex = 0;
    for (const QString &filename : xmlFiles) {
        fileIndex++;
        QString filePath = syntaxDir.filePath(filename);
        
        qDebug() << "Loading syntax file [" << fileIndex << "/" << xmlFiles.size() << "]:" << filename;
        
        if (loadSyntaxFile(filePath)) {
            successCount++;
        }
    }
    
    qDebug() << "=== Load Summary ===" 
             << "Loaded" << successCount << "out of" << xmlFiles.size() << "syntax definitions";
    qDebug() << "Total syntaxes in map:" << m_syntaxMap.size();
    qDebug() << "Extension mappings:" << m_extensionMap.size();
    
    // 打印扩展名映射详情（只打印部分）
    qDebug() << "=== Extension Mappings (first 20) ===";
    int mapCount = 0;
    for (auto it = m_extensionMap.constBegin(); it != m_extensionMap.constEnd() && mapCount < 20; ++it, ++mapCount) {
        qDebug() << "  ." + it.key() << "=>" << it.value();
    }
    if (m_extensionMap.size() > 20) {
        qDebug() << "  ... and" << (m_extensionMap.size() - 20) << "more extensions";
    }
    
    return successCount > 0;
}

bool SyntaxManager::loadSyntaxFile(const QString &filePath)
{
    qDebug() << "  Loading syntax from:" << filePath;
    
    // ✅ 使用成员变量，不再重新创建 SyntaxLoader
    SyntaxDefinition definition = m_syntaxLoader.loadSyntaxFromFile(filePath);
    
    if (definition.name.isEmpty()) {
        qWarning() << "  Failed to load syntax from:" << filePath;
        return false;
    }
    
    qDebug() << "  Loaded syntax:" << definition.name 
             << "priority:" << definition.priority
             << "extensions:" << definition.extensions
             << "contexts:" << definition.contexts.size()
             << "keywords:" << definition.keywords.size();
    
    // 存储语法定义
    m_syntaxMap[definition.name] = definition;
    
    // 注册扩展名映射
    registerExtensions(definition);
    
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
    
    qDebug() << "=== SyntaxManager::getSyntaxByFilename ===" 
             << "filename:" << filename
             << "extension:" << extension;
    
    SyntaxDefinition def = getSyntaxByExtension(extension);
    
    if (!def.name.isEmpty()) {
        qDebug() << "  Found syntax:" << def.name;
    } else {
        qWarning() << "  No syntax found for extension:" << extension;
    }
    
    return def;
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
