#ifndef SYNTAXLOADER_H
#define SYNTAXLOADER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QRegularExpression>
#include <QXmlStreamReader>

// 关键字列表
struct KeywordList {
    QString name;
    QStringList keywords;
};

// 上下文规则
struct ContextRule {
    enum RuleType {
        Unknown,
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
    
    RuleType type = Unknown;
    QString attribute;          // 应用的样式名称
    QString context;            // 跳转到的上下文或包含的规则
    QString string;             // 匹配的字符串
    QString string1;            // 第二个字符串（用于某些规则）
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
    QString extensions;  // 扩展名字符串
    QString mimetypes;   // MIME类型字符串
    int priority = 0;
    
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
    SyntaxDefinition loadSyntaxFromFile(const QString &filePath);
    
    // 根据文件扩展名判断是否匹配
    bool matchesFile(const QString &filename, const SyntaxDefinition &def) const;
    
private:
    // 解析XML的辅助函数
    void parseContexts(QXmlStreamReader &xml, SyntaxDefinition &definition, const QString &syntaxDir);
    void parseItemDatas(QXmlStreamReader &xml, SyntaxDefinition &definition);
    
    // 外部语法支持
    QString findSyntaxFile(const QString &syntaxName, const QString &searchDir);
    void expandExternalReferences(SyntaxDefinition &definition, 
                                  const QMap<QString, SyntaxDefinition> &externalDefs);
    
    // 防止无限递归：记录正在加载的语法名称
    QSet<QString> m_loadingStack;
};

#endif // SYNTAXLOADER_H
