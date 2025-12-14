#include "SyntaxLoader.h"
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include <QFileInfo>

// SyntaxDefinition methods
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

// SyntaxLoader implementation
SyntaxLoader::SyntaxLoader(QObject *parent)
    : QObject(parent)
{
}

SyntaxDefinition SyntaxLoader::loadSyntaxFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open syntax file:" << filePath;
        return SyntaxDefinition();
    }

    QXmlStreamReader xml(&file);
    SyntaxDefinition definition;
    
    // 记录当前文件的目录，用于解析相对路径的外部引用
    QFileInfo fileInfo(filePath);
    QString syntaxDir = fileInfo.absolutePath();
    QString syntaxBaseName = fileInfo.completeBaseName(); // 不含扩展名的文件名

    // 防止无限递归：检查是否正在加载中
    if (m_loadingStack.contains(syntaxBaseName)) {
        qWarning() << "Circular reference detected! Already loading:" << syntaxBaseName;
        qWarning() << "Loading stack:" << m_loadingStack;
        return SyntaxDefinition(); // 返回空定义，避免无限递归
    }
    
    // 添加到加载栈
    m_loadingStack.insert(syntaxBaseName);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QString("language")) {
                definition.name = xml.attributes().value("name").toString();
                definition.section = xml.attributes().value("section").toString();
                definition.extensions = xml.attributes().value("extensions").toString();
                definition.mimetypes = xml.attributes().value("mimetype").toString();
                
                // 读取大小写敏感属性
                if (xml.attributes().hasAttribute("casesensitive")) {
                    QString caseSensitiveStr = xml.attributes().value("casesensitive").toString();
                    definition.caseSensitive = (caseSensitiveStr == "1" || 
                                               caseSensitiveStr.toLower() == "true");
                } else {
                    // 默认区分大小写
                    definition.caseSensitive = true;
                }
                
                qDebug() << "Loading syntax:" << definition.name 
                         << "case sensitive:" << definition.caseSensitive;
            }
            else if (xml.name() == QString("list")) {
                QString listName = xml.attributes().value("name").toString();
                KeywordList list;
                list.name = listName;

                while (!(xml.isEndElement() && xml.name() == QString("list"))) {
                    xml.readNext();
                    if (xml.isStartElement() && xml.name() == QString("item")) {
                        QString keyword = xml.readElementText();
                        list.keywords.append(keyword);
                    }
                }

                definition.keywords[listName] = list;
            }
            else if (xml.name() == QString("contexts")) {
                parseContexts(xml, definition, syntaxDir);
            }
            else if (xml.name() == QString("itemDatas")) {
                parseItemDatas(xml, definition);
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "XML parsing error:" << xml.errorString();
    }

    // 从加载栈中移除
    m_loadingStack.remove(syntaxBaseName);

    qDebug() << "Loaded syntax:" << definition.name << "with priority:" << definition.priority;
    
    return definition;
}

void SyntaxLoader::parseContexts(QXmlStreamReader &xml, SyntaxDefinition &definition, const QString &syntaxDir)
{
    // 用于跟踪需要加载的外部语法
    QSet<QString> externalSyntaxNames;
    QMap<QString, SyntaxDefinition> externalDefinitions;
    
    while (!(xml.isEndElement() && xml.name() == QString("contexts"))) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QString("context")) {
            Context context;
            context.name = xml.attributes().value("name").toString();
            context.attribute = xml.attributes().value("attribute").toString();
            context.lineEndContext = xml.attributes().value("lineEndContext").toString();
            context.fallthroughContext = xml.attributes().value("fallthroughContext").toString();

            // 解析上下文中的规则
            while (!(xml.isEndElement() && xml.name() == QString("context"))) {
                xml.readNext();

                if (xml.isStartElement()) {
                    ContextRule rule;
                    QString elementName = xml.name().toString();

                    if (elementName == "DetectChar") {
                        rule.type = ContextRule::DetectChar;
                        QString charStr = xml.attributes().value("char").toString();
                        if (!charStr.isEmpty()) {
                            rule.char0 = charStr.at(0);
                        }
                    }
                    else if (elementName == "Detect2Chars") {
                        rule.type = ContextRule::Detect2Chars;
                        QString char0Str = xml.attributes().value("char").toString();
                        QString char1Str = xml.attributes().value("char1").toString();
                        if (!char0Str.isEmpty()) rule.char0 = char0Str.at(0);
                        if (!char1Str.isEmpty()) rule.char1 = char1Str.at(0);
                    }
                    else if (elementName == "AnyChar") {
                        rule.type = ContextRule::AnyChar;
                        rule.string = xml.attributes().value("String").toString();
                    }
                    else if (elementName == "StringDetect") {
                        rule.type = ContextRule::StringDetect;
                        rule.string = xml.attributes().value("String").toString();
                    }
                    else if (elementName == "WordDetect") {
                        rule.type = ContextRule::WordDetect;
                        rule.string = xml.attributes().value("String").toString();
                    }
                    else if (elementName == "RegExpr") {
                        rule.type = ContextRule::RegExpr;
                        QString pattern = xml.attributes().value("String").toString();
                        rule.regex = QRegularExpression(pattern);
                        
                        if (!rule.regex.isValid()) {
                            qWarning() << "Invalid regex pattern:" << pattern 
                                      << "Error:" << rule.regex.errorString();
                        }
                    }
                    else if (elementName == "keyword") {
                        rule.type = ContextRule::Keyword;
                        rule.keywordList = xml.attributes().value("String").toString();
                    }
                    else if (elementName == "Int") {
                        rule.type = ContextRule::Int;
                    }
                    else if (elementName == "Float") {
                        rule.type = ContextRule::Float;
                    }
                    else if (elementName == "HlCOct") {
                        rule.type = ContextRule::HlCOct;
                    }
                    else if (elementName == "HlCHex") {
                        rule.type = ContextRule::HlCHex;
                    }
                    else if (elementName == "HlCStringChar") {
                        rule.type = ContextRule::HlCStringChar;
                    }
                    else if (elementName == "LineContinue") {
                        rule.type = ContextRule::LineContinue;
                    }
                    else if (elementName == "RangeDetect") {
                        rule.type = ContextRule::RangeDetect;
                        QString char0Str = xml.attributes().value("char").toString();
                        QString char1Str = xml.attributes().value("char1").toString();
                        if (!char0Str.isEmpty()) rule.char0 = char0Str.at(0);
                        if (!char1Str.isEmpty()) rule.char1 = char1Str.at(0);
                    }
                    else if (elementName == "DetectSpaces") {
                        rule.type = ContextRule::DetectSpaces;
                    }
                    else if (elementName == "DetectIdentifier") {
                        rule.type = ContextRule::DetectIdentifier;
                    }
                    else if (elementName == "IncludeRules") {
                        rule.type = ContextRule::IncludeRules;
                        rule.context = xml.attributes().value("context").toString();
                        
                        // 检测外部语法引用
                        if (rule.context.startsWith("##")) {
                            QString externalSyntax = rule.context.mid(2); // 去掉 "##"
                            externalSyntaxNames.insert(externalSyntax);
                            
                            qDebug() << "Found external syntax reference:" << externalSyntax 
                                    << "in context:" << context.name;
                        }
                    }

                    // 通用属性
                    if (xml.attributes().hasAttribute("attribute")) {
                        rule.attribute = xml.attributes().value("attribute").toString();
                    }
                    if (xml.attributes().hasAttribute("context")) {
                        rule.context = xml.attributes().value("context").toString();
                    }
                    if (xml.attributes().hasAttribute("lookAhead")) {
                        rule.lookAhead = (xml.attributes().value("lookAhead").toString() == "true" ||
                                         xml.attributes().value("lookAhead").toString() == "1");
                    }

                    if (rule.type != ContextRule::Unknown) {
                        context.rules.append(rule);
                    }
                }
            }

            definition.contexts[context.name] = context;
        }
    }
    
    // 加载所有外部语法定义
    for (const QString &externalName : externalSyntaxNames) {
        QString externalFilePath = findSyntaxFile(externalName, syntaxDir);
        
        if (!externalFilePath.isEmpty()) {
            qDebug() << "Loading external syntax:" << externalName << "from:" << externalFilePath;
            SyntaxDefinition externalDef = loadSyntaxFromFile(externalFilePath);
            
            if (!externalDef.name.isEmpty()) {
                externalDefinitions[externalName] = externalDef;
            } else {
                qWarning() << "Failed to load external syntax:" << externalName;
            }
        } else {
            qWarning() << "Could not find syntax file for:" << externalName;
        }
    }
    
    // 展开所有外部引用
    if (!externalDefinitions.isEmpty()) {
        expandExternalReferences(definition, externalDefinitions);
    }
}

QString SyntaxLoader::findSyntaxFile(const QString &syntaxName, const QString &searchDir)
{
    // 可能的文件名格式
    QStringList possibleNames;
    possibleNames << syntaxName.toLower() + ".xml";
    possibleNames << syntaxName + ".xml";
    
    QString cleaned = syntaxName;
    cleaned.replace(" ", "");
    possibleNames << cleaned.toLower() + ".xml";
    
    // 首先在当前目录搜索
    for (const QString &fileName : possibleNames) {
        QString filePath = searchDir + "/" + fileName;
        if (QFile::exists(filePath)) {
            return filePath;
        }
    }
    
    return QString(); // 未找到
}

void SyntaxLoader::expandExternalReferences(SyntaxDefinition &definition, 
                                            const QMap<QString, SyntaxDefinition> &externalDefs)
{
    qDebug() << "=== Expanding external references ===";
    
    // 遍历所有上下文
    for (QMap<QString, Context>::iterator contextIt = definition.contexts.begin(); 
         contextIt != definition.contexts.end(); ++contextIt) {
        
        Context &context = contextIt.value();
        QList<ContextRule> expandedRules;
        
        for (const ContextRule &rule : context.rules) {
            if (rule.type == ContextRule::IncludeRules && rule.context.startsWith("##")) {
                // 这是一个外部引用
                QString externalName = rule.context.mid(2);
                
                if (externalDefs.contains(externalName)) {
                    const SyntaxDefinition &externalDef = externalDefs[externalName];
                    
                    // 查找外部语法中的对应上下文
                    // 通常是 "Normal" 或 "Main" 上下文
                    QString targetContextName = "Normal";
                    if (!externalDef.contexts.contains(targetContextName)) {
                        targetContextName = "Main";
                    }
                    
                    if (externalDef.contexts.contains(targetContextName)) {
                        const Context &externalContext = externalDef.contexts[targetContextName];
                        
                        qDebug() << "Expanding" << externalName << "context:" << targetContextName
                                << "(" << externalContext.rules.size() << "rules) into context:" 
                                << context.name;
                        
                        // 将外部上下文的规则复制到当前位置
                        for (const ContextRule &externalRule : externalContext.rules) {
                            expandedRules.append(externalRule);
                        }
                        
                        // 也需要合并关键字列表
                        for (QMap<QString, KeywordList>::const_iterator kwIt = externalDef.keywords.constBegin();
                             kwIt != externalDef.keywords.constEnd(); ++kwIt) {
                            
                            if (!definition.keywords.contains(kwIt.key())) {
                                definition.keywords[kwIt.key()] = kwIt.value();
                                
                                qDebug() << "  Imported keyword list:" << kwIt.key()
                                        << "with" << kwIt.value().keywords.size() << "keywords";
                            }
                        }
                        
                        // 也需要合并 itemDatas
                        for (QMap<QString, QString>::const_iterator itemIt = externalDef.itemDatas.constBegin();
                             itemIt != externalDef.itemDatas.constEnd(); ++itemIt) {
                            
                            if (!definition.itemDatas.contains(itemIt.key())) {
                                definition.itemDatas[itemIt.key()] = itemIt.value();
                            }
                        }
                        
                    } else {
                        qWarning() << "External syntax" << externalName 
                                  << "does not have Normal or Main context";
                    }
                } else {
                    qWarning() << "External syntax not loaded:" << externalName;
                }
            } else {
                // 普通规则，直接保留
                expandedRules.append(rule);
            }
        }
        
        // 替换原有的规则列表
        context.rules = expandedRules;
    }
    
    qDebug() << "=== External references expansion complete ===";
}

void SyntaxLoader::parseItemDatas(QXmlStreamReader &xml, SyntaxDefinition &definition)
{
    while (!(xml.isEndElement() && xml.name() == QString("itemDatas"))) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QString("itemData")) {
            QString name = xml.attributes().value("name").toString();
            QString defStyleNum = xml.attributes().value("defStyleNum").toString();

            definition.itemDatas[name] = defStyleNum;
        }
    }
}

bool SyntaxLoader::matchesFile(const QString &filename, const SyntaxDefinition &def) const
{
    QStringList extensions = def.extensions.split(';', Qt::SkipEmptyParts);
    
    for (const QString &pattern : extensions) {
        QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(pattern));
        if (regex.match(filename).hasMatch()) {
            return true;
        }
    }
    return false;
}
