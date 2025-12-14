#ifndef SYNTAXMANAGER_H
#define SYNTAXMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QStringList>
#include "SyntaxLoader.h"

class SyntaxManager : public QObject
{
    Q_OBJECT
public:
    explicit SyntaxManager(QObject *parent = nullptr);
    
    // 加载指定目录下的所有语法定义文件
    bool loadSyntaxDirectory(const QString &directoryPath);
    
    // 根据文件扩展名获取语法定义
    SyntaxDefinition getSyntaxByExtension(const QString &extension) const;
    
    // 根据文件名获取语法定义（会自动提取扩展名）
    SyntaxDefinition getSyntaxByFilename(const QString &filename) const;
    
    // 根据语法名称获取语法定义
    SyntaxDefinition getSyntaxByName(const QString &name) const;
    
    // 获取所有已加载的语法定义名称列表
    QStringList getAvailableSyntaxNames() const;
    
    // 检查是否已加载语法定义
    bool hasSyntaxForExtension(const QString &extension) const;
    
    // 获取已加载的语法定义数量
    int syntaxCount() const { return m_syntaxMap.size(); }
    
private:
    // 存储语法定义：语法名称 -> SyntaxDefinition
    QMap<QString, SyntaxDefinition> m_syntaxMap;
    
    // 存储扩展名映射：扩展名 -> 语法名称列表（按优先级排序）
    QMap<QString, QStringList> m_extensionMap;
    
    // 存储完整文件名映射：文件名 -> 语法名称列表（按优先级排序）
    QMap<QString, QStringList> m_filenameMap;
    
    // 加载单个语法文件
    bool loadSyntaxFile(const QString &filePath);
    
    // 注册扩展名映射
    void registerExtensions(const SyntaxDefinition &definition);
    
    // 规范化扩展名（去除前导点号，转小写）
    QString normalizeExtension(const QString &extension) const;
    
    // 规范化文件名（转小写）
    QString normalizeFilename(const QString &filename) const;
    
    // 根据优先级选择最佳语法定义
    SyntaxDefinition selectByPriority(const QStringList &syntaxNames) const;
};

#endif // SYNTAXMANAGER_H
