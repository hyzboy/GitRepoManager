#include "SyntaxLoader.h"
#include <QFile>
#include <QDomDocument>
#include <QDebug>

KeywordList SyntaxDefinition::getKeywordList(const QString &name) const
{
    return keywords.value(name, KeywordList());
}

Context SyntaxDefinition::getContext(const QString &name) const
{
    return contexts.value(name, Context());
}

QString SyntaxDefinition::getDefaultStyle(const QString &attribute) const
{
    return itemDatas.value(attribute, "Normal");
}

SyntaxLoader::SyntaxLoader(QObject *parent)
    : QObject(parent)
{
}

bool SyntaxLoader::loadSyntax(const QString &syntaxFilePath)
{
    QFile file(syntaxFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开语法文件:" << syntaxFilePath;
        return false;
    }
    
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        qWarning() << "XML解析错误:" << errorMsg 
                   << "行:" << errorLine << "列:" << errorColumn;
        file.close();
        return false;
    }
    
    file.close();
    
    QDomElement root = doc.documentElement();
    if (root.tagName() != "language") {
        qWarning() << "无效的语法文件格式";
        return false;
    }
    
    // 读取语言属性
    m_definition.name = root.attribute("name");
    m_definition.section = root.attribute("section");
    m_definition.style = root.attribute("style");
    m_definition.indenter = root.attribute("indenter");
    
    QString caseSensitiveStr = root.attribute("casesensitive", "1");
    m_definition.caseSensitive = (caseSensitiveStr == "1" || caseSensitiveStr == "true");
    
    // 解析扩展名
    QString extensions = root.attribute("extensions");
    if (!extensions.isEmpty()) {
        m_definition.extensions = extensions.split(';', Qt::SkipEmptyParts);
    }
    
    // 解析MIME类型
    QString mimetypes = root.attribute("mimetype");
    if (!mimetypes.isEmpty()) {
        m_definition.mimeTypes = mimetypes.split(';', Qt::SkipEmptyParts);
    }
    
    // 解析highlighting部分
    QDomNodeList highlightingNodes = root.elementsByTagName("highlighting");
    if (highlightingNodes.count() > 0) {
        parseHighlighting(highlightingNodes.at(0).toElement());
    }
    
    qDebug() << "成功加载语法定义:" << m_definition.name;
    return true;
}

void SyntaxLoader::parseHighlighting(const QDomElement &element)
{
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        QDomElement childElement = child.toElement();
        if (!childElement.isNull()) {
            if (childElement.tagName() == "list") {
                parseList(childElement);
            } else if (childElement.tagName() == "contexts") {
                parseContexts(childElement);
            } else if (childElement.tagName() == "itemDatas") {
                parseItemDatas(childElement);
            }
        }
        child = child.nextSibling();
    }
}

void SyntaxLoader::parseList(const QDomElement &element)
{
    KeywordList list;
    list.name = element.attribute("name");
    
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        QDomElement itemElement = child.toElement();
        if (!itemElement.isNull() && itemElement.tagName() == "item") {
            QString keyword = itemElement.text();
            if (!keyword.isEmpty()) {
                list.keywords.append(keyword);
            }
        }
        child = child.nextSibling();
    }
    
    m_definition.keywords[list.name] = list;
}

void SyntaxLoader::parseContexts(const QDomElement &element)
{
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        QDomElement contextElement = child.toElement();
        if (!contextElement.isNull() && contextElement.tagName() == "context") {
            parseContext(contextElement);
        }
        child = child.nextSibling();
    }
}

void SyntaxLoader::parseContext(const QDomElement &element)
{
    Context context;
    context.name = element.attribute("name");
    context.attribute = element.attribute("attribute", "Normal Text");
    context.lineEndContext = element.attribute("lineEndContext", "#stay");
    context.fallthroughContext = element.attribute("fallthroughContext");
    
    // 解析规则
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        QDomElement ruleElement = child.toElement();
        if (!ruleElement.isNull()) {
            ContextRule rule = parseRule(ruleElement);
            if (rule.type != ContextRule::DetectSpaces || !ruleElement.tagName().isEmpty()) {
                context.rules.append(rule);
            }
        }
        child = child.nextSibling();
    }
    
    m_definition.contexts[context.name] = context;
}

ContextRule SyntaxLoader::parseRule(const QDomElement &element)
{
    ContextRule rule;
    QString tagName = element.tagName();
    
    rule.attribute = element.attribute("attribute");
    rule.context = element.attribute("context", "#stay");
    rule.lookAhead = (element.attribute("lookAhead") == "true" || element.attribute("lookAhead") == "1");
    rule.firstNonSpace = (element.attribute("firstNonSpace") == "true");
    
    if (tagName == "DetectChar") {
        rule.type = ContextRule::DetectChar;
        QString charStr = element.attribute("char");
        if (!charStr.isEmpty()) {
            rule.char0 = charStr.at(0);
        }
    } else if (tagName == "Detect2Chars") {
        rule.type = ContextRule::Detect2Chars;
        QString char0Str = element.attribute("char");
        QString char1Str = element.attribute("char1");
        if (!char0Str.isEmpty()) rule.char0 = char0Str.at(0);
        if (!char1Str.isEmpty()) rule.char1 = char1Str.at(0);
    } else if (tagName == "AnyChar") {
        rule.type = ContextRule::AnyChar;
        rule.string = element.attribute("String");
    } else if (tagName == "StringDetect") {
        rule.type = ContextRule::StringDetect;
        rule.string = element.attribute("String");
        rule.caseSensitive = m_definition.caseSensitive;
    } else if (tagName == "WordDetect") {
        rule.type = ContextRule::WordDetect;
        rule.string = element.attribute("String");
    } else if (tagName == "RegExpr") {
        rule.type = ContextRule::RegExpr;
        QString pattern = element.attribute("String");
        rule.regex = QRegularExpression(pattern);
    } else if (tagName == "keyword") {
        rule.type = ContextRule::Keyword;
        rule.keywordList = element.attribute("String");
    } else if (tagName == "Int") {
        rule.type = ContextRule::Int;
    } else if (tagName == "Float") {
        rule.type = ContextRule::Float;
    } else if (tagName == "HlCOct") {
        rule.type = ContextRule::HlCOct;
    } else if (tagName == "HlCHex") {
        rule.type = ContextRule::HlCHex;
    } else if (tagName == "LineContinue") {
        rule.type = ContextRule::LineContinue;
    } else if (tagName == "RangeDetect") {
        rule.type = ContextRule::RangeDetect;
        QString char0Str = element.attribute("char");
        QString char1Str = element.attribute("char1");
        if (!char0Str.isEmpty()) rule.char0 = char0Str.at(0);
        if (!char1Str.isEmpty()) rule.char1 = char1Str.at(0);
    } else if (tagName == "IncludeRules") {
        rule.type = ContextRule::IncludeRules;
        rule.context = element.attribute("context");
    } else if (tagName == "DetectSpaces") {
        rule.type = ContextRule::DetectSpaces;
    } else if (tagName == "DetectIdentifier") {
        rule.type = ContextRule::DetectIdentifier;
    }
    
    // 检查区域标记
    if (element.hasAttribute("beginRegion")) {
        rule.beginRegion = true;
        rule.regionName = element.attribute("beginRegion");
    }
    if (element.hasAttribute("endRegion")) {
        rule.endRegion = true;
        rule.regionName = element.attribute("endRegion");
    }
    
    return rule;
}

void SyntaxLoader::parseItemDatas(const QDomElement &element)
{
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        QDomElement itemElement = child.toElement();
        if (!itemElement.isNull() && itemElement.tagName() == "itemData") {
            QString name = itemElement.attribute("name");
            QString defStyleNum = itemElement.attribute("defStyleNum");
            
            // 映射defStyleNum到样式名称
            QString styleName = "Normal";
            if (defStyleNum == "dsKeyword") styleName = "Keyword";
            else if (defStyleNum == "dsControlFlow") styleName = "ControlFlow";
            else if (defStyleNum == "dsDataType") styleName = "DataType";
            else if (defStyleNum == "dsDecVal" || defStyleNum == "dsBaseN") styleName = "DecVal";
            else if (defStyleNum == "dsFloat") styleName = "Float";
            else if (defStyleNum == "dsChar") styleName = "Char";
            else if (defStyleNum == "dsString") styleName = "String";
            else if (defStyleNum == "dsComment") styleName = "Comment";
            else if (defStyleNum == "dsOthers") styleName = "Others";
            else if (defStyleNum == "dsPreprocessor") styleName = "Preprocessor";
            else if (defStyleNum == "dsOperator") styleName = "Operator";
            else if (defStyleNum == "dsBuiltIn") styleName = "BuiltIn";
            else if (defStyleNum == "dsExtension") styleName = "Extension";
            else if (defStyleNum == "dsAttribute") styleName = "Attribute";
            else if (defStyleNum == "dsImport") styleName = "Import";
            else if (defStyleNum == "dsVariable") styleName = "Variable";
            else if (defStyleNum == "dsError") styleName = "Error";
            
            m_definition.itemDatas[name] = styleName;
        }
        child = child.nextSibling();
    }
}

bool SyntaxLoader::matchesFile(const QString &filename) const
{
    for (const QString &pattern : m_definition.extensions) {
        QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(pattern));
        if (regex.match(filename).hasMatch()) {
            return true;
        }
    }
    return false;
}
