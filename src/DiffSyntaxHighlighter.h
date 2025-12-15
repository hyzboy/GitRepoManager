#ifndef DIFFSYNTAXHIGHLIGHTER_H
#define DIFFSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include "SyntaxHighlighter.h"
#include "SyntaxLoader.h"
#include "ThemeLoader.h"

/**
 * @brief Git Diff 语法高亮器
 * 
 * 处理 Git Diff 格式的特殊需求：
 * 1. 识别 +/- 前缀
 * 2. 根据前缀设置行背景色
 * 3. 对去除前缀后的代码进行语法高亮
 */
class DiffSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit DiffSyntaxHighlighter(QTextDocument *parent = nullptr);
    
    // 设置底层代码的语法定义（如 C++）
    void setCodeSyntaxDefinition(const SyntaxDefinition &definition);
    void setTheme(ThemeLoader *themeLoader);
    
protected:
    void highlightBlock(const QString &text) override;
    
private:
    // 保持临时文档用于代码高亮
    QTextDocument *m_tempDoc;
    SyntaxHighlighter *m_codeHighlighter;  // 用于高亮底层代码
    ThemeLoader *m_themeLoader;
    
    // Diff 特殊格式
    QTextCharFormat m_addedLineFormat;      // + 行格式（浅绿色背景）
    QTextCharFormat m_removedLineFormat;    // - 行格式（浅红色背景）
    QTextCharFormat m_contextLineFormat;    // 普通行格式
    QTextCharFormat m_headerFormat;         // diff 头部格式
    
    // 初始化格式
    void initDiffFormats();
    
    // 检测行类型
    enum LineType {
        AddedLine,      // + 开头
        RemovedLine,    // - 开头
        ContextLine,    // 普通代码行
        HeaderLine,     // diff/@@@ 等头部
        EmptyLine       // 空行
    };
    
    LineType detectLineType(const QString &text) const;
    
    // 应用 Diff 背景色和代码高亮
    void applyDiffHighlight(const QString &text, LineType lineType);
};

#endif // DIFFSYNTAXHIGHLIGHTER_H
