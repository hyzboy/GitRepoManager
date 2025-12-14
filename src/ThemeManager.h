#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QStringList>
#include "ThemeLoader.h"

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();
    
    // 加载指定目录下的所有主题文件
    bool loadThemeDirectory(const QString &directoryPath);
    
    // 根据主题名称获取主题加载器
    ThemeLoader* getTheme(const QString &themeName) const;
    
    // 获取所有已加载的主题名称列表
    QStringList getAvailableThemeNames() const;
    
    // 检查是否已加载指定主题
    bool hasTheme(const QString &themeName) const;
    
    // 获取已加载的主题数量
    int themeCount() const { return m_themes.size(); }
    
    // 设置当前活动主题
    void setActiveTheme(const QString &themeName);
    
    // 获取当前活动主题
    ThemeLoader* getActiveTheme() const { return m_activeTheme; }
    QString getActiveThemeName() const { return m_activeThemeName; }
    
signals:
    // 当活动主题改变时发出信号
    void activeThemeChanged(const QString &themeName);
    
private:
    // 存储主题加载器：主题名称 -> ThemeLoader*
    QMap<QString, ThemeLoader*> m_themes;
    
    // 当前活动主题
    ThemeLoader* m_activeTheme;
    QString m_activeThemeName;
    
    // 加载单个主题文件
    bool loadThemeFile(const QString &filePath);
    
    // 规范化主题名称（从文件名提取）
    QString extractThemeName(const QString &filename) const;
};

#endif // THEMEMANAGER_H
