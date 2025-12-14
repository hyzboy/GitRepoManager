#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "SyntaxLoader.h"
#include "ThemeLoader.h"

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    
    // 设置语法定义和主题
    void setSyntaxDefinition(const SyntaxDefinition &definition);
    void setTheme(ThemeLoader *themeLoader);
    
protected:
    void highlightBlock(const QString &text) override;
    
private:
    SyntaxDefinition m_definition;
    ThemeLoader *m_themeLoader;
    
    QMap<QString, QTextCharFormat> m_formats;
    
    // 当前状态栈（用于跟踪上下文）
    struct State {
        QString contextName;
        // 可以添加更多状态信息
    };
    
    // 初始化格式
    void initFormats();
    
    // 应用规则进行高亮
    bool applyRule(const ContextRule &rule, const QString &text, int &position, 
                   const QString &currentContext);
    
    // 检查关键字
    bool isKeyword(const QString &word, const QString &listName) const;
    
    // 应用格式
    void applyFormat(int start, int length, const QString &attribute);
    
    // 上下文管理
    QString resolveContext(const QString &context, const QString &currentContext) const;
};

#endif // SYNTAXHIGHLIGHTER_H
