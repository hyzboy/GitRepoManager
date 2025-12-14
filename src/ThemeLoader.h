#ifndef THEMELOADER_H
#define THEMELOADER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QColor>
#include <QTextCharFormat>

// 文本样式定义
struct TextStyle {
    QColor textColor;
    QColor selectedTextColor;
    QColor backgroundColor;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikeThrough = false;
    
    QTextCharFormat toTextCharFormat() const;
};

// 编辑器颜色定义
struct EditorColors {
    QColor backgroundColor;
    QColor textColor;
    QColor currentLine;
    QColor lineNumbers;
    QColor selection;
    // 可以根据需要添加更多颜色
};

class ThemeLoader : public QObject
{
    Q_OBJECT
public:
    explicit ThemeLoader(QObject *parent = nullptr);
    
    // 加载主题文件
    bool loadTheme(const QString &themeFilePath);
    
    // 获取文本样式
    TextStyle getTextStyle(const QString &styleName) const;
    
    // 获取编辑器颜色
    EditorColors getEditorColors() const;
    
    // 获取自定义样式（如果需要）
    TextStyle getCustomStyle(const QString &language, const QString &styleName) const;
    
    QString themeName() const { return m_themeName; }
    
private:
    QString m_themeName;
    QMap<QString, TextStyle> m_textStyles;
    QMap<QString, QMap<QString, TextStyle>> m_customStyles;
    EditorColors m_editorColors;
    
    // 解析颜色字符串
    QColor parseColor(const QString &colorStr) const;
};

#endif // THEMELOADER_H
