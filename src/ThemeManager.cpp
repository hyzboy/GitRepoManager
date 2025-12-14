#include "ThemeManager.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_activeTheme(nullptr)
{
}

ThemeManager::~ThemeManager()
{
    // 清理所有主题加载器
    qDeleteAll(m_themes);
    m_themes.clear();
}

bool ThemeManager::loadThemeDirectory(const QString &directoryPath)
{
    QDir directory(directoryPath);
    if (!directory.exists()) {
        qWarning() << "主题目录不存在:" << directoryPath;
        return false;
    }
    
    // 获取所有主题文件（.theme 扩展名）
    QStringList filters;
    filters << "*.theme" << "*.json";
    QFileInfoList fileList = directory.entryInfoList(filters, QDir::Files);
    
    if (fileList.isEmpty()) {
        qWarning() << "目录中没有找到主题文件:" << directoryPath;
        return false;
    }
    
    int successCount = 0;
    int failCount = 0;
    
    for (const QFileInfo &fileInfo : fileList) {
        if (loadThemeFile(fileInfo.absoluteFilePath())) {
            successCount++;
        } else {
            failCount++;
        }
    }
    
    qDebug() << "主题加载完成: 成功" << successCount << "个，失败" << failCount << "个";
    
    // 如果没有设置活动主题且至少加载了一个主题，设置第一个为活动主题
    if (m_activeTheme == nullptr && !m_themes.isEmpty()) {
        QString firstName = m_themes.firstKey();
        setActiveTheme(firstName);
    }
    
    return successCount > 0;
}

bool ThemeManager::loadThemeFile(const QString &filePath)
{
    ThemeLoader *loader = new ThemeLoader(this);
    
    if (!loader->loadTheme(filePath)) {
        delete loader;
        return false;
    }
    
    QString themeName = loader->themeName();
    
    // 如果主题没有名称，从文件名提取
    if (themeName.isEmpty()) {
        themeName = extractThemeName(filePath);
    }
    
    if (themeName.isEmpty()) {
        qWarning() << "无法确定主题名称:" << filePath;
        delete loader;
        return false;
    }
    
    // 如果已存在同名主题，删除旧的
    if (m_themes.contains(themeName)) {
        ThemeLoader *oldLoader = m_themes.take(themeName);
        if (oldLoader == m_activeTheme) {
            m_activeTheme = nullptr;
            m_activeThemeName.clear();
        }
        delete oldLoader;
    }
    
    // 存储新主题
    m_themes[themeName] = loader;
    
    qDebug() << "成功加载主题:" << themeName;
    return true;
}

QString ThemeManager::extractThemeName(const QString &filename) const
{
    QFileInfo fileInfo(filename);
    QString baseName = fileInfo.completeBaseName();
    
    // 移除可能的前缀或后缀，规范化名称
    baseName = baseName.trimmed();
    
    return baseName;
}

ThemeLoader* ThemeManager::getTheme(const QString &themeName) const
{
    return m_themes.value(themeName, nullptr);
}

QStringList ThemeManager::getAvailableThemeNames() const
{
    return m_themes.keys();
}

bool ThemeManager::hasTheme(const QString &themeName) const
{
    return m_themes.contains(themeName);
}

void ThemeManager::setActiveTheme(const QString &themeName)
{
    if (!m_themes.contains(themeName)) {
        qWarning() << "主题不存在:" << themeName;
        return;
    }
    
    ThemeLoader *newTheme = m_themes[themeName];
    
    if (newTheme != m_activeTheme) {
        m_activeTheme = newTheme;
        m_activeThemeName = themeName;
        
        qDebug() << "切换到主题:" << themeName;
        emit activeThemeChanged(themeName);
    }
}
