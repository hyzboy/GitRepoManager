#include "SyntaxHighlighter.h"
#include <QDebug>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_themeLoader(nullptr)
{
}

void SyntaxHighlighter::setSyntaxDefinition(const SyntaxDefinition &definition)
{
    m_definition = definition;
    qDebug() << "=== SyntaxHighlighter::setSyntaxDefinition ===" 
             << "name:" << definition.name
             << "contexts:" << definition.contexts.size()
             << "keywords:" << definition.keywords.size();
    
    initFormats();
    rehighlight();
}

void SyntaxHighlighter::setTheme(ThemeLoader *themeLoader)
{
    m_themeLoader = themeLoader;
    qDebug() << "=== SyntaxHighlighter::setTheme ===" 
             << "theme:" << (themeLoader ? themeLoader->themeName() : "null");
    
    initFormats();
    rehighlight();
}

void SyntaxHighlighter::initFormats()
{
    if (!m_themeLoader) {
        qDebug() << "SyntaxHighlighter::initFormats - no theme loader";
        return;
    }
    
    m_formats.clear();
    
    qDebug() << "SyntaxHighlighter::initFormats - creating formats for" 
             << m_definition.itemDatas.size() << "items";
    
    // 为每个itemData创建格式
    for (auto it = m_definition.itemDatas.constBegin(); 
         it != m_definition.itemDatas.constEnd(); ++it) {
        QString attributeName = it.key();
        QString styleName = it.value();
        
        TextStyle style = m_themeLoader->getTextStyle(styleName);
        m_formats[attributeName] = style.toTextCharFormat();
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    static int blockCounter = 0;
    int currentBlockId = ++blockCounter;
    
    qDebug() << ">>> [Block" << currentBlockId << "] highlightBlock START - text length:" << text.length();
    
    if (text.isEmpty()) {
        qDebug() << "<<< [Block" << currentBlockId << "] highlightBlock END - empty text";
        return;
    }
    
    if (m_definition.contexts.isEmpty()) {
        qDebug() << "<<< [Block" << currentBlockId << "] highlightBlock END - no contexts";
        return;
    }
    
    // C++ 语法定义使用 "Normal" 作为默认上下文
    QString currentContext = "Normal";
    
    // 如果找不到 "Normal"，使用第一个可用的上下文
    if (!m_definition.contexts.contains(currentContext) && !m_definition.contexts.isEmpty()) {
        currentContext = m_definition.contexts.firstKey();
        qDebug() << "[Block" << currentBlockId << "] Using fallback context:" << currentContext;
    }
    
    Context context = m_definition.getContext(currentContext);
    if (context.name.isEmpty()) {
        qDebug() << "<<< [Block" << currentBlockId << "] highlightBlock END - no valid context";
        return;
    }
    
    qDebug() << "[Block" << currentBlockId << "] Starting context:" << currentContext 
             << "rules:" << context.rules.size();
    
    int position = 0;
    int iterationCount = 0;
    const int maxIterations = text.length() * 10;  // 增加限制以便更容易检测死循环
    
    // 跟踪上下文切换
    QStringList contextHistory;
    contextHistory << currentContext;
    
    while (position < text.length() && iterationCount < maxIterations) {
        iterationCount++;
        
        if (iterationCount % 100 == 0) {
            qDebug() << "[Block" << currentBlockId << "] Iteration:" << iterationCount 
                     << "position:" << position << "/" << text.length()
                     << "context:" << currentContext;
        }
        
        bool matched = false;
        
        // 尝试应用每个规则
        for (int ruleIdx = 0; ruleIdx < context.rules.size(); ++ruleIdx) {
            const ContextRule &rule = context.rules[ruleIdx];
            
            // 处理 IncludeRules
            if (rule.type == ContextRule::IncludeRules) {
                QString includeContext = rule.context;
                
                qDebug() << "[Block" << currentBlockId << "] IncludeRules:" << includeContext 
                         << "at position:" << position;
                
                // 处理 ##ISO C++ 这样的外部引用（暂时跳过）
                if (includeContext.startsWith("##")) {
                    qDebug() << "[Block" << currentBlockId << "] Skipping external reference:" << includeContext;
                    continue;
                }
                
                // 检测循环包含
                if (includeContext == currentContext) {
                    qDebug() << "!!! [Block" << currentBlockId << "] WARNING: Self-referencing IncludeRules detected!"
                             << "context:" << includeContext;
                    continue;
                }
                
                // 获取要包含的上下文
                Context includedContext = m_definition.getContext(includeContext);
                if (!includedContext.name.isEmpty()) {
                    qDebug() << "[Block" << currentBlockId << "] Processing included context:" 
                             << includeContext << "with" << includedContext.rules.size() << "rules";
                    
                    // 递归尝试包含上下文的规则
                    for (const ContextRule &includedRule : includedContext.rules) {
                        int oldPosition = position;
                        if (applyRule(includedRule, text, position, currentContext)) {
                            matched = true;
                            
                            qDebug() << "[Block" << currentBlockId << "] Matched included rule at position:" 
                                     << oldPosition << "new position:" << position;
                            
                            // 处理上下文切换
                            if (!includedRule.context.isEmpty() && includedRule.context != "#stay") {
                                QString newContext = resolveContext(includedRule.context, currentContext);
                                
                                qDebug() << "[Block" << currentBlockId << "] Context switch from:" 
                                         << currentContext << "to:" << newContext
                                         << "via rule context:" << includedRule.context;
                                
                                if (!newContext.isEmpty() && m_definition.contexts.contains(newContext)) {
                                    currentContext = newContext;
                                    context = m_definition.getContext(currentContext);
                                    contextHistory << currentContext;
                                    
                                    // 检测上下文循环
                                    if (contextHistory.count(currentContext) > 3) {
                                        qDebug() << "!!! [Block" << currentBlockId << "] WARNING: Context appears multiple times!"
                                                 << "context:" << currentContext
                                                 << "count:" << contextHistory.count(currentContext)
                                                 << "history:" << contextHistory;
                                    }
                                }
                            }
                            break;
                        }
                        position = oldPosition;
                    }
                    
                    if (matched) {
                        break;
                    }
                }
                continue;
            }
            
            // 应用普通规则
            int oldPosition = position;
            if (applyRule(rule, text, position, currentContext)) {
                matched = true;
                
                if (position == oldPosition && !rule.lookAhead) {
                    qDebug() << "!!! [Block" << currentBlockId << "] WARNING: Rule matched but position didn't advance!"
                             << "rule type:" << rule.type
                             << "position:" << position
                             << "lookAhead:" << rule.lookAhead;
                }
                
                // 处理上下文切换
                if (!rule.context.isEmpty() && rule.context != "#stay") {
                    QString newContext = resolveContext(rule.context, currentContext);
                    
                    qDebug() << "[Block" << currentBlockId << "] Context switch from:" 
                             << currentContext << "to:" << newContext
                             << "via rule context:" << rule.context;
                    
                    if (!newContext.isEmpty() && m_definition.contexts.contains(newContext)) {
                        currentContext = newContext;
                        context = m_definition.getContext(currentContext);
                        contextHistory << currentContext;
                        
                        // 检测上下文循环
                        if (contextHistory.count(currentContext) > 3) {
                            qDebug() << "!!! [Block" << currentBlockId << "] WARNING: Context appears multiple times!"
                                     << "context:" << currentContext
                                     << "count:" << contextHistory.count(currentContext);
                        }
                    }
                }
                
                break;
            }
            position = oldPosition;
        }
        
        if (!matched) {
            // 没有规则匹配，应用默认格式并前进一个字符
            if (!context.attribute.isEmpty()) {
                applyFormat(position, 1, context.attribute);
            }
            position++;
        }
    }
    
    if (iterationCount >= maxIterations) {
        qCritical() << "!!! [Block" << currentBlockId << "] INFINITE LOOP DETECTED!"
                    << "iterations:" << iterationCount
                    << "position:" << position << "/" << text.length()
                    << "current context:" << currentContext
                    << "context history:" << contextHistory;
    }
    
    qDebug() << "<<< [Block" << currentBlockId << "] highlightBlock END"
             << "- iterations:" << iterationCount
             << "final position:" << position
             << "text length:" << text.length();
    
    // 保存当前状态
    setCurrentBlockState(0); // 简化处理
}

bool SyntaxHighlighter::applyRule(const ContextRule &rule, const QString &text, 
                                   int &position, const QString &currentContext)
{
    int start = position;
    int length = 0;
    bool matched = false;
    
    // 记录规则类型（用于调试）
    QString ruleTypeName;
    switch (rule.type) {
        case ContextRule::DetectChar: ruleTypeName = "DetectChar"; break;
        case ContextRule::Detect2Chars: ruleTypeName = "Detect2Chars"; break;
        case ContextRule::AnyChar: ruleTypeName = "AnyChar"; break;
        case ContextRule::StringDetect: ruleTypeName = "StringDetect"; break;
        case ContextRule::WordDetect: ruleTypeName = "WordDetect"; break;
        case ContextRule::RegExpr: ruleTypeName = "RegExpr"; break;
        case ContextRule::Keyword: ruleTypeName = "Keyword"; break;
        case ContextRule::Int: ruleTypeName = "Int"; break;
        case ContextRule::Float: ruleTypeName = "Float"; break;
        case ContextRule::HlCOct: ruleTypeName = "HlCOct"; break;
        case ContextRule::HlCHex: ruleTypeName = "HlCHex"; break;
        case ContextRule::LineContinue: ruleTypeName = "LineContinue"; break;
        case ContextRule::RangeDetect: ruleTypeName = "RangeDetect"; break;
        case ContextRule::IncludeRules: ruleTypeName = "IncludeRules"; break;
        case ContextRule::DetectSpaces: ruleTypeName = "DetectSpaces"; break;
        case ContextRule::DetectIdentifier: ruleTypeName = "DetectIdentifier"; break;
        default: ruleTypeName = "Unknown"; break;
    }
    
    switch (rule.type) {
        case ContextRule::DetectChar: {
            if (position < text.length() && text.at(position) == rule.char0) {
                length = 1;
                matched = true;
            }
            break;
        }
        
        case ContextRule::Detect2Chars: {
            if (position + 1 < text.length() && 
                text.at(position) == rule.char0 && 
                text.at(position + 1) == rule.char1) {
                length = 2;
                matched = true;
            }
            break;
        }
        
        case ContextRule::AnyChar: {
            if (position < text.length() && rule.string.contains(text.at(position))) {
                length = 1;
                matched = true;
            }
            break;
        }
        
        case ContextRule::StringDetect: {
            if (text.mid(position).startsWith(rule.string)) {
                length = rule.string.length();
                matched = true;
            }
            break;
        }
        
        case ContextRule::WordDetect: {
            // 检查单词边界
            if (position > 0 && text.at(position - 1).isLetterOrNumber()) {
                break;
            }
            if (text.mid(position).startsWith(rule.string)) {
                int endPos = position + rule.string.length();
                if (endPos >= text.length() || !text.at(endPos).isLetterOrNumber()) {
                    length = rule.string.length();
                    matched = true;
                }
            }
            break;
        }
        
        case ContextRule::RegExpr: {
            QRegularExpressionMatch match = rule.regex.match(text, position, 
                QRegularExpression::NormalMatch, 
                QRegularExpression::AnchoredMatchOption);
            if (match.hasMatch() && match.capturedStart() == position) {
                length = match.capturedLength();
                matched = true;
            }
            break;
        }
        
        case ContextRule::Keyword: {
            // 提取单词
            int wordStart = position;
            while (position < text.length() && 
                   (text.at(position).isLetterOrNumber() || text.at(position) == '_')) {
                position++;
            }
            
            if (position > wordStart) {
                QString word = text.mid(wordStart, position - wordStart);
                if (isKeyword(word, rule.keywordList)) {
                    length = word.length();
                    matched = true;
                }
                position = wordStart; // 恢复位置
            }
            break;
        }
        
        case ContextRule::Int: {
            // 简单的整数匹配
            int intStart = position;
            while (position < text.length() && text.at(position).isDigit()) {
                position++;
            }
            if (position > intStart) {
                length = position - intStart;
                matched = true;
                position = intStart;
            }
            break;
        }
        
        case ContextRule::Float: {
            // 简单的浮点数匹配
            QRegularExpression floatRegex("\\d+\\.\\d+([eE][+-]?\\d+)?");
            QRegularExpressionMatch match = floatRegex.match(text, position, 
                QRegularExpression::NormalMatch, 
                QRegularExpression::AnchoredMatchOption);
            if (match.hasMatch()) {
                length = match.capturedLength();
                matched = true;
            }
            break;
        }
        
        case ContextRule::HlCHex: {
            // 十六进制数匹配 0x...
            if (position + 2 < text.length() && 
                text.at(position) == '0' && 
                (text.at(position + 1) == 'x' || text.at(position + 1) == 'X')) {
                int hexStart = position + 2;
                int hexEnd = hexStart;
                while (hexEnd < text.length() && text.at(hexEnd).isDigit()) {
                    hexEnd++;
                }
                if (hexEnd > hexStart) {
                    length = hexEnd - position;
                    matched = true;
                }
            }
            break;
        }
        
        case ContextRule::DetectSpaces: {
            // 匹配空白字符
            int spaceStart = position;
            while (position < text.length() && text.at(position).isSpace()) {
                position++;
            }
            if (position > spaceStart) {
                length = position - spaceStart;
                matched = true;
                position = spaceStart;
            }
            break;
        }
        
        case ContextRule::DetectIdentifier: {
            // 匹配标识符
            if (position < text.length() && 
                (text.at(position).isLetter() || text.at(position) == '_')) {
                int identStart = position;
                while (position < text.length() && 
                       (text.at(position).isLetterOrNumber() || text.at(position) == '_')) {
                    position++;
                }
                length = position - identStart;
                matched = true;
                position = identStart;
            }
            break;
        }
        
        default:
            break;
    }
    
    if (matched) {
        if (!rule.lookAhead) {
            position = start + length;
        }
        
        if (!rule.attribute.isEmpty()) {
            applyFormat(start, length, rule.attribute);
        }
        
        qDebug() << "      applyRule MATCHED - type:" << ruleTypeName
                 << "start:" << start << "length:" << length
                 << "newPos:" << position << "attribute:" << rule.attribute
                 << "context:" << rule.context;
    }
    
    return matched;
}

bool SyntaxHighlighter::isKeyword(const QString &word, const QString &listName) const
{
    KeywordList list = m_definition.getKeywordList(listName);
    
    if (m_definition.caseSensitive) {
        return list.keywords.contains(word);
    } else {
        for (const QString &keyword : list.keywords) {
            if (keyword.compare(word, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }
    }
    
    return false;
}

void SyntaxHighlighter::applyFormat(int start, int length, const QString &attribute)
{
    if (m_formats.contains(attribute)) {
        setFormat(start, length, m_formats[attribute]);
    }
}

QString SyntaxHighlighter::resolveContext(const QString &context, 
                                           const QString &currentContext) const
{
    qDebug() << "    resolveContext called - context:" << context 
             << "currentContext:" << currentContext;
    
    if (context == "#stay") {
        qDebug() << "    -> returning currentContext (stay):" << currentContext;
        return currentContext;
    } else if (context == "#pop") {
        // 简化处理：返回Normal
        qDebug() << "    -> returning Normal (pop)";
        return "Normal";
    } else if (context.startsWith("#pop!")) {
        // 弹出并切换
        QString newContext = context.mid(5);
        qDebug() << "    -> returning" << newContext << "(pop and switch)";
        return newContext;
    } else {
        qDebug() << "    -> returning" << context << "(direct)";
        return context;
    }
}
