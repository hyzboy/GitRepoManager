#include "ThemeLoader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFont>

QTextCharFormat TextStyle::toTextCharFormat() const
{
    QTextCharFormat format;
    
    if (textColor.isValid()) {
        format.setForeground(textColor);
    }
    if (backgroundColor.isValid()) {
        format.setBackground(backgroundColor);
    }
    
    QFont font;
    if (bold) {
        font.setBold(true);
    }
    if (italic) {
        font.setItalic(true);
    }
    if (underline) {
        font.setUnderline(true);
    }
    if (strikeThrough) {
        font.setStrikeOut(true);
    }
    
    format.setFont(font);
    
    return format;
}

ThemeLoader::ThemeLoader(QObject *parent)
    : QObject(parent)
{
}

bool ThemeLoader::loadTheme(const QString &themeFilePath)
{
    QFile file(themeFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开主题文件:" << themeFilePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误:" << error.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qWarning() << "主题文件格式错误";
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // 读取元数据
    if (root.contains("metadata")) {
        QJsonObject metadata = root["metadata"].toObject();
        m_themeName = metadata["name"].toString();
    }
    
    // 读取文本样式
    if (root.contains("text-styles")) {
        QJsonObject textStyles = root["text-styles"].toObject();
        
        for (auto it = textStyles.constBegin(); it != textStyles.constEnd(); ++it) {
            QString styleName = it.key();
            QJsonObject styleObj = it.value().toObject();
            
            TextStyle style;
            if (styleObj.contains("text-color")) {
                style.textColor = parseColor(styleObj["text-color"].toString());
            }
            if (styleObj.contains("selected-text-color")) {
                style.selectedTextColor = parseColor(styleObj["selected-text-color"].toString());
            }
            if (styleObj.contains("background-color")) {
                style.backgroundColor = parseColor(styleObj["background-color"].toString());
            }
            if (styleObj.contains("bold")) {
                style.bold = styleObj["bold"].toBool();
            }
            if (styleObj.contains("italic")) {
                style.italic = styleObj["italic"].toBool();
            }
            if (styleObj.contains("underline")) {
                style.underline = styleObj["underline"].toBool();
            }
            if (styleObj.contains("strike-through")) {
                style.strikeThrough = styleObj["strike-through"].toBool();
            }
            
            m_textStyles[styleName] = style;
        }
    }
    
    // 读取编辑器颜色
    if (root.contains("editor-colors")) {
        QJsonObject editorColors = root["editor-colors"].toObject();
        
        if (editorColors.contains("BackgroundColor")) {
            m_editorColors.backgroundColor = parseColor(editorColors["BackgroundColor"].toString());
        }
        if (editorColors.contains("TextSelection")) {
            m_editorColors.selection = parseColor(editorColors["TextSelection"].toString());
        }
        if (editorColors.contains("CurrentLine")) {
            m_editorColors.currentLine = parseColor(editorColors["CurrentLine"].toString());
        }
        if (editorColors.contains("LineNumbers")) {
            m_editorColors.lineNumbers = parseColor(editorColors["LineNumbers"].toString());
        }
    }
    
    // 读取自定义样式
    if (root.contains("custom-styles")) {
        QJsonObject customStyles = root["custom-styles"].toObject();
        
        for (auto langIt = customStyles.constBegin(); langIt != customStyles.constEnd(); ++langIt) {
            QString language = langIt.key();
            QJsonObject langStyles = langIt.value().toObject();
            
            QMap<QString, TextStyle> styleMap;
            for (auto styleIt = langStyles.constBegin(); styleIt != langStyles.constEnd(); ++styleIt) {
                QString styleName = styleIt.key();
                QJsonObject styleObj = styleIt.value().toObject();
                
                TextStyle style;
                if (styleObj.contains("text-color")) {
                    style.textColor = parseColor(styleObj["text-color"].toString());
                }
                if (styleObj.contains("selected-text-color")) {
                    style.selectedTextColor = parseColor(styleObj["selected-text-color"].toString());
                }
                
                styleMap[styleName] = style;
            }
            
            m_customStyles[language] = styleMap;
        }
    }
    
    qDebug() << "成功加载主题:" << m_themeName;
    return true;
}

TextStyle ThemeLoader::getTextStyle(const QString &styleName) const
{
    return m_textStyles.value(styleName, TextStyle());
}

EditorColors ThemeLoader::getEditorColors() const
{
    return m_editorColors;
}

TextStyle ThemeLoader::getCustomStyle(const QString &language, const QString &styleName) const
{
    if (m_customStyles.contains(language)) {
        return m_customStyles[language].value(styleName, TextStyle());
    }
    return TextStyle();
}

QColor ThemeLoader::parseColor(const QString &colorStr) const
{
    if (colorStr.isEmpty()) {
        return QColor();
    }
    
    // 处理 #RRGGBB 格式
    if (colorStr.startsWith("#")) {
        return QColor(colorStr);
    }
    
    return QColor();
}
