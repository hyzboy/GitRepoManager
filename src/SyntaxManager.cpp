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
    QDir directory(directoryPath);
    if (!directory.exists()) {
        qWarning() << "语法定义目录不存在:" << directoryPath;
        return false;
    }
    
    // 获取所有 XML 文件
    QStringList filters;
    filters << "*.xml";
    QFileInfoList fileList = directory.entryInfoList(filters, QDir::Files);
    
    if (fileList.isEmpty()) {
        qWarning() << "目录中没有找到 XML 文件:" << directoryPath;
        return false;
    }
    
    int successCount = 0;
    int failCount = 0;
    
    for (const QFileInfo &fileInfo : fileList) {
        if (loadSyntaxFile(fileInfo.absoluteFilePath())) {
            successCount++;
        } else {
            failCount++;
        }
    }
    
    qDebug() << "语法定义加载完成: 成功" << successCount << "个，失败" << failCount << "个";
    return successCount > 0;
}

bool SyntaxManager::loadSyntaxFile(const QString &filePath)
{
    SyntaxLoader loader;
    if (!loader.loadSyntax(filePath)) {
        return false;
    }
    
    SyntaxDefinition definition = loader.getSyntaxDefinition();
    
    if (definition.name.isEmpty()) {
        qWarning() << "语法定义名称为空:" << filePath;
        return false;
    }
    
    // 存储语法定义
    m_syntaxMap[definition.name] = definition;
    
    // 注册扩展名映射
    registerExtensions(definition);
    
    return true;
}

void SyntaxManager::registerExtensions(const SyntaxDefinition &definition)
{
    for (const QString &pattern : definition.extensions) {
        // 处理通配符模式，例如 "*.cpp", "*.h"
        if (pattern.startsWith("*.")) {
            QString extension = normalizeExtension(pattern.mid(2));
            if (!extension.isEmpty()) {
                // 添加到扩展名映射列表中
                if (!m_extensionMap[extension].contains(definition.name)) {
                    m_extensionMap[extension].append(definition.name);
                }
            }
        } else if (pattern.startsWith("*")) {
            QString extension = normalizeExtension(pattern.mid(1));
            if (!extension.isEmpty()) {
                // 添加到扩展名映射列表中
                if (!m_extensionMap[extension].contains(definition.name)) {
                    m_extensionMap[extension].append(definition.name);
                }
            }
        } else {
            // 处理完整文件名（例如 "CMakeLists.txt", "Makefile", ".gitignore"）
            QString normalizedName = normalizeFilename(pattern);
            if (!normalizedName.isEmpty()) {
                // 添加到文件名映射列表中
                if (!m_filenameMap[normalizedName].contains(definition.name)) {
                    m_filenameMap[normalizedName].append(definition.name);
                }
            }
        }
    }
}

QString SyntaxManager::normalizeExtension(const QString &extension) const
{
    QString normalized = extension.trimmed().toLower();
    
    // 移除前导点号
    while (normalized.startsWith('.')) {
        normalized = normalized.mid(1);
    }
    
    return normalized;
}

QString SyntaxManager::normalizeFilename(const QString &filename) const
{
    return filename.trimmed().toLower();
}

SyntaxDefinition SyntaxManager::selectByPriority(const QStringList &syntaxNames) const
{
    if (syntaxNames.isEmpty()) {
        return SyntaxDefinition();
    }
    
    // 如果只有一个，直接返回
    if (syntaxNames.size() == 1) {
        return m_syntaxMap.value(syntaxNames.first(), SyntaxDefinition());
    }
    
    // 找出优先级最高的语法定义
    QString bestSyntax;
    int highestPriority = -1;
    
    for (const QString &syntaxName : syntaxNames) {
        const SyntaxDefinition &def = m_syntaxMap.value(syntaxName);
        if (def.priority > highestPriority) {
            highestPriority = def.priority;
            bestSyntax = syntaxName;
        }
    }
    
    if (!bestSyntax.isEmpty()) {
        return m_syntaxMap.value(bestSyntax, SyntaxDefinition());
    }
    
    // 如果都没有优先级，返回第一个
    return m_syntaxMap.value(syntaxNames.first(), SyntaxDefinition());
}

SyntaxDefinition SyntaxManager::getSyntaxByExtension(const QString &extension) const
{
    QString normalized = normalizeExtension(extension);
    
    if (m_extensionMap.contains(normalized)) {
        const QStringList &syntaxNames = m_extensionMap[normalized];
        return selectByPriority(syntaxNames);
    }
    
    return SyntaxDefinition();
}

SyntaxDefinition SyntaxManager::getSyntaxByFilename(const QString &filename) const
{
    QFileInfo fileInfo(filename);
    QString baseFilename = fileInfo.fileName();
    
    // 首先检查完整文件名匹配（例如 CMakeLists.txt, Makefile）
    QString normalizedFilename = normalizeFilename(baseFilename);
    if (m_filenameMap.contains(normalizedFilename)) {
        const QStringList &syntaxNames = m_filenameMap[normalizedFilename];
        return selectByPriority(syntaxNames);
    }
    
    // 如果没有完整文件名匹配，尝试通过扩展名匹配
    QString extension = fileInfo.suffix();
    if (!extension.isEmpty()) {
        return getSyntaxByExtension(extension);
    }
    
    return SyntaxDefinition();
}

SyntaxDefinition SyntaxManager::getSyntaxByName(const QString &name) const
{
    return m_syntaxMap.value(name, SyntaxDefinition());
}

QStringList SyntaxManager::getAvailableSyntaxNames() const
{
    return m_syntaxMap.keys();
}

bool SyntaxManager::hasSyntaxForExtension(const QString &extension) const
{
    QString normalized = normalizeExtension(extension);
    return m_extensionMap.contains(normalized);
}
