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
    QString fileName;    // 原始文件名（用于调试）
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
    
    // 设置语法文件目录并建立名称映射
    void setSyntaxDirectory(const QString &syntaxDir);
    
    // 从文件加载语法定义
    SyntaxDefinition loadSyntaxFromFile(const QString &filePath);
    
    // 根据语法名称查找文件路径
    QString findSyntaxFileByName(const QString &syntaxName);
    
    // 检查语法是否匹配指定文件
    bool matchesFile(const QString &filename, const SyntaxDefinition &def) const;
    
private:
    QString m_syntaxDirectory;
    QMap<QString, QString> m_syntaxNameToFileMap;  // 语法名称 -> 文件路径映射
    QSet<QString> m_loadingStack;  // 防止循环引用
    
    // 建立语法名称到文件的映射表
    void buildSyntaxNameMap();
    
    // 从 XML 文件中提取语法名称
    QString extractSyntaxName(const QString &filePath);
    
    // 解析 contexts 节点
    void parseContexts(QXmlStreamReader &xml, SyntaxDefinition &definition);
    
    // 展开外部语法引用
    void expandExternalReferences(SyntaxDefinition &definition, 
                                  const QMap<QString, SyntaxDefinition> &externalDefs);
    
    // 解析 itemDatas 节点
    void parseItemDatas(QXmlStreamReader &xml, SyntaxDefinition &definition);
};

#endif // SYNTAXLOADER_H
