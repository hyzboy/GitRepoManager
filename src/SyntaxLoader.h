#ifndef SYNTAXLOADER_H
#define SYNTAXLOADER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QRegularExpression>
#include <QtXml>

// 关键字列表
struct KeywordList {
    QString name;
    QStringList keywords;
};

// 上下文规则
struct ContextRule {
    enum RuleType {
        DetectChar,
        Detect2Chars,
        AnyChar,
        StringDetect,
        WordDetect,
        RegExpr,
        Keyword,
        Int,
        Float,
        HlCOct,
        HlCHex,
        HlCStringChar,
        LineContinue,
        RangeDetect,
        IncludeRules,
        DetectSpaces,
        DetectIdentifier
    };
    
    RuleType type;
    QString attribute;          // 应用的样式名称
    QString context;            // 跳转到的上下文
    QString string;             // 匹配的字符串
    QString string1;            // 第二个字符（用于Detect2Chars）
    QChar char0;                // 字符（用于DetectChar）
    QChar char1;                // 第二个字符
    QString keywordList;        // 关键字列表名称
    QRegularExpression regex;   // 正则表达式
    bool caseSensitive = true;
    bool lookAhead = false;
    bool firstNonSpace = false;
    bool beginRegion = false;
    bool endRegion = false;
    QString regionName;
};

// 上下文定义
struct Context {
    QString name;
    QString attribute;
    QString lineEndContext;
    QString fallthroughContext;
    QList<ContextRule> rules;
};

// 语法定义
class SyntaxDefinition
{
public:
    QString name;
    QString section;
    QString style;
    QStringList extensions;
    QStringList mimeTypes;
    
    QMap<QString, KeywordList> keywords;
    QMap<QString, Context> contexts;
    QMap<QString, QString> itemDatas;  // 属性名 -> 默认样式名
    
    QString indenter;
    bool caseSensitive = true;
    
    // 获取关键字列表
    KeywordList getKeywordList(const QString &name) const;
    
    // 获取上下文
    Context getContext(const QString &name) const;
    
    // 获取默认样式映射
    QString getDefaultStyle(const QString &attribute) const;
};

class SyntaxLoader : public QObject
{
    Q_OBJECT
public:
    explicit SyntaxLoader(QObject *parent = nullptr);
    
    // 加载语法定义文件
    bool loadSyntax(const QString &syntaxFilePath);
    
    // 获取语法定义
    SyntaxDefinition getSyntaxDefinition() const { return m_definition; }
    
    // 根据文件扩展名判断是否匹配
    bool matchesFile(const QString &filename) const;
    
private:
    SyntaxDefinition m_definition;
    
    // 解析XML的辅助函数
    void parseHighlighting(const QDomElement &element);
    void parseList(const QDomElement &element);
    void parseContexts(const QDomElement &element);
    void parseContext(const QDomElement &element);
    void parseItemDatas(const QDomElement &element);
    ContextRule parseRule(const QDomElement &element);
};

#endif // SYNTAXLOADER_H
