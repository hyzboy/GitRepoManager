#include "SyntaxHighlighter.h"
#include <QDebug>

// 调试开关：设置为 true 启用详细日志，false 禁用
#define SYNTAX_DEBUG_ENABLED true
#define SYNTAX_DEBUG_VERBOSE false  // 更详细的规则匹配日志

#if SYNTAX_DEBUG_ENABLED
    #define SYNTAX_DEBUG(msg) qDebug() << msg
    #define SYNTAX_WARNING(msg) qWarning() << msg
    #define SYNTAX_CRITICAL(msg) qCritical() << msg
#else
    #define SYNTAX_DEBUG(msg)
    #define SYNTAX_WARNING(msg)
    #define SYNTAX_CRITICAL(msg) qCritical() << msg  // 保留严重错误
#endif

#if SYNTAX_DEBUG_VERBOSE
    #define SYNTAX_DEBUG_VERBOSE_LOG(msg) qDebug() << msg
#else
    #define SYNTAX_DEBUG_VERBOSE_LOG(msg)
#endif

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_themeLoader(nullptr)
{
    SYNTAX_DEBUG("=== SyntaxHighlighter constructed ===");
}

void SyntaxHighlighter::setSyntaxDefinition(const SyntaxDefinition &definition)
{
    m_definition = definition;
    SYNTAX_DEBUG("=== SyntaxHighlighter::setSyntaxDefinition ===" 
             << "name:" << definition.name
             << "contexts:" << definition.contexts.size()
             << "keywords:" << definition.keywords.size());
    
    // 打印所有可用的上下文
    SYNTAX_DEBUG("Available contexts:");
    for (auto it = definition.contexts.constBegin(); it != definition.contexts.constEnd(); ++it) {
        SYNTAX_DEBUG("  -" << it.key() << "with" << it.value().rules.size() << "rules");
    }
    
    // 打印所有关键字列表
    SYNTAX_DEBUG("Available keyword lists:");
    for (auto it = definition.keywords.constBegin(); it != definition.keywords.constEnd(); ++it) {
        SYNTAX_DEBUG("  -" << it.key() << "with" << it.value().keywords.size() << "keywords");
    }
    
    // 打印所有样式映射
    SYNTAX_DEBUG("Available itemDatas (styles):");
    for (auto it = definition.itemDatas.constBegin(); it != definition.itemDatas.constEnd(); ++it) {
        SYNTAX_DEBUG("  -" << it.key() << "=>" << it.value());
    }
    
    initFormats();
    rehighlight();
}

void SyntaxHighlighter::setTheme(ThemeLoader *themeLoader)
{
    m_themeLoader = themeLoader;
    SYNTAX_DEBUG("=== SyntaxHighlighter::setTheme ===" 
             << "theme:" << (themeLoader ? themeLoader->themeName() : "null"));
    
    initFormats();
    rehighlight();
}

void SyntaxHighlighter::initFormats()
{
    if (!m_themeLoader) {
        SYNTAX_WARNING("initFormats called but no theme loader set!");
        return;
    }
    
    m_formats.clear();
    
    SYNTAX_DEBUG("=== Initializing formats ===");
    int formatCount = 0;
    
    // 为每个itemData创建格式
    for (auto it = m_definition.itemDatas.constBegin(); 
         it != m_definition.itemDatas.constEnd(); ++it) {
        QString attributeName = it.key();
        QString styleName = it.value();
        
        TextStyle style = m_themeLoader->getTextStyle(styleName);
        QTextCharFormat format = style.toTextCharFormat();
        
        m_formats[attributeName] = format;
        
        SYNTAX_DEBUG("  Format:" << attributeName << "=> style:" << styleName 
                    << "color:" << format.foreground().color().name()
                    << "bold:" << (format.fontWeight() == QFont::Bold));
        
        formatCount++;
    }
    
    SYNTAX_DEBUG("Initialized" << formatCount << "formats");
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    static int blockCounter = 0;
    int currentBlockId = ++blockCounter;
    
    // 每10个block输出一次详细日志
    bool verboseThisBlock = (currentBlockId % 10 == 0) || (currentBlockId <= 5);
    
    if (verboseThisBlock) {
        SYNTAX_DEBUG(">>> [Block" << currentBlockId << "] highlightBlock START"
                    << "- text length:" << text.length() 
                    << "- text:" << text.left(50));  // 打印前50个字符
    }
    
    if (text.isEmpty()) {
        return;
    }
    
    if (m_definition.contexts.isEmpty()) {
        if (verboseThisBlock) {
            SYNTAX_CRITICAL("!!! [Block" << currentBlockId << "] No contexts available!");
        }
        return;
    }
    
    // C++ 语法定义使用 "Normal" 作为默认上下文
    QString currentContext = "Normal";
    
    // 如果找不到 "Normal"，使用第一个可用的上下文
    if (!m_definition.contexts.contains(currentContext)) {
        if (!m_definition.contexts.isEmpty()) {
            currentContext = m_definition.contexts.firstKey();
            if (verboseThisBlock) {
                SYNTAX_WARNING("[Block" << currentBlockId << "] 'Normal' not found, using:" << currentContext);
            }
        } else {
            SYNTAX_CRITICAL("!!! [Block" << currentBlockId << "] No contexts at all!");
            return;
        }
    }
    
    Context context = m_definition.getContext(currentContext);
    if (context.name.isEmpty()) {
        if (verboseThisBlock) {
            SYNTAX_CRITICAL("!!! [Block" << currentBlockId << "] Failed to get context:" << currentContext);
        }
        return;
    }
    
    if (verboseThisBlock) {
        SYNTAX_DEBUG("[Block" << currentBlockId << "] Context:" << currentContext 
                    << "with" << context.rules.size() << "rules");
    }
    
    int position = 0;
    int iterationCount = 0;
    const int maxIterations = text.length() * 10;
    int matchedRulesCount = 0;
    int appliedFormatsCount = 0;
    
    while (position < text.length() && iterationCount < maxIterations) {
        iterationCount++;
        
        // 每 100 次迭代检查一次
        if (iterationCount % 100 == 0) {
            SYNTAX_WARNING("[Block" << currentBlockId << "] High iteration:" << iterationCount 
                     << "pos:" << position << "/" << text.length());
        }
        
        bool matched = false;
        
        // 输出当前位置的字符（用于调试）
        if (verboseThisBlock && (position % 10 == 0 || position < 5)) {
            QString charStr = (position < text.length()) ? QString(text.at(position)) : "EOF";
            SYNTAX_DEBUG_VERBOSE_LOG("  Pos" << position << "char:'" << charStr << "'");
        }
        
        // 尝试应用每个规则
        for (int ruleIdx = 0; ruleIdx < context.rules.size(); ++ruleIdx) {
            const ContextRule &rule = context.rules[ruleIdx];
            
            // 处理 IncludeRules
            if (rule.type == ContextRule::IncludeRules) {
                QString includeContext = rule.context;
                
                // 处理外部语法引用
                if (includeContext.startsWith("##")) {
                    continue;
                }
                
                // 检测循环包含
                if (includeContext == currentContext) {
                    continue;
                }
                
                // 获取要包含的上下文
                Context includedContext = m_definition.getContext(includeContext);
                if (!includedContext.name.isEmpty()) {
                    // 递归尝试包含上下文的规则
                    for (const ContextRule &includedRule : includedContext.rules) {
                        int oldPosition = position;
                        if (applyRule(includedRule, text, position, currentContext)) {
                            matched = true;
                            matchedRulesCount++;
                            
                            // 处理上下文切换
                            if (!includedRule.context.isEmpty() && includedRule.context != "#stay") {
                                QString newContext = resolveContext(includedRule.context, currentContext);
                                
                                if (!newContext.isEmpty() && m_definition.contexts.contains(newContext)) {
                                    currentContext = newContext;
                                    context = m_definition.getContext(currentContext);
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
                matchedRulesCount++;
                
                if (position == oldPosition && !rule.lookAhead) {
                    if (verboseThisBlock) {
                        SYNTAX_WARNING("[Block" << currentBlockId << "] Rule matched but pos didn't advance!");
                    }
                }
                
                // 处理上下文切换
                if (!rule.context.isEmpty() && rule.context != "#stay") {
                    QString newContext = resolveContext(rule.context, currentContext);
                    
                    if (!newContext.isEmpty() && m_definition.contexts.contains(newContext)) {
                        currentContext = newContext;
                        context = m_definition.getContext(currentContext);
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
                appliedFormatsCount++;
            }
            
            if (verboseThisBlock && (position < 10 || position % 20 == 0)) {
                SYNTAX_DEBUG_VERBOSE_LOG("  No match at pos" << position 
                    << ", applying default format, char:'" << text.at(position) << "'");
            }
            
            position++;
        }
    }
    
    if (iterationCount >= maxIterations) {
        SYNTAX_CRITICAL("!!! [Block" << currentBlockId << "] INFINITE LOOP!"
                    << "iterations:" << iterationCount
                    << "pos:" << position << "/" << text.length());
    }
    
    if (verboseThisBlock) {
        SYNTAX_DEBUG("<<< [Block" << currentBlockId << "] END"
                 << "- iterations:" << iterationCount
                 << "- matched:" << matchedRulesCount
                 << "- formats:" << appliedFormatsCount);
    }
    
    // 保存当前状态
    setCurrentBlockState(0);
}

bool SyntaxHighlighter::applyRule(const ContextRule &rule, const QString &text, 
                                   int &position, const QString &currentContext)
{
    int start = position;
    int length = 0;
    bool matched = false;
    
    switch (rule.type) {
        case ContextRule::DetectChar: {
            if (position < text.length() && text.at(position) == rule.char0) {
                length = 1;
                matched = true;
                SYNTAX_DEBUG_VERBOSE_LOG("  [DetectChar] Matched '" << rule.char0 << "' at" << position);
            }
            break;
        }
        
        case ContextRule::Detect2Chars: {
            if (position + 1 < text.length() && 
                text.at(position) == rule.char0 && 
                text.at(position + 1) == rule.char1) {
                length = 2;
                matched = true;
                SYNTAX_DEBUG_VERBOSE_LOG("  [Detect2Chars] Matched at" << position);
            }
            break;
        }
        
        case ContextRule::AnyChar: {
            if (position < text.length() && rule.string.contains(text.at(position))) {
                length = 1;
                matched = true;
                SYNTAX_DEBUG_VERBOSE_LOG("  [AnyChar] Matched '" << text.at(position) << "' at" << position);
            }
            break;
        }
        
        case ContextRule::StringDetect: {
            if (text.mid(position).startsWith(rule.string)) {
                length = rule.string.length();
                matched = true;
                SYNTAX_DEBUG_VERBOSE_LOG("  [StringDetect] Matched '" << rule.string << "' at" << position);
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
                    SYNTAX_DEBUG_VERBOSE_LOG("  [WordDetect] Matched '" << rule.string << "' at" << position);
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
                SYNTAX_DEBUG_VERBOSE_LOG("  [RegExpr] Matched at" << position 
                    << "text:'" << match.captured(0).left(20) << "'");
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
                    SYNTAX_DEBUG_VERBOSE_LOG("  [Keyword] Matched '" << word 
                        << "' from list '" << rule.keywordList << "'");
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
                SYNTAX_DEBUG_VERBOSE_LOG("  [Int] Matched at" << intStart);
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
                SYNTAX_DEBUG_VERBOSE_LOG("  [DetectIdentifier] Matched '" 
                    << text.mid(identStart, qMin(length, 20)) << "' at" << identStart);
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
            
            SYNTAX_DEBUG_VERBOSE_LOG("  >>> Format '" << rule.attribute 
                    << "' [" << start << "-" << (start + length) << "] '"
                    << text.mid(start, qMin(length, 20)) << "'");
        }
    }
    
    return matched;
}

bool SyntaxHighlighter::isKeyword(const QString &word, const QString &listName) const
{
    KeywordList list = m_definition.getKeywordList(listName);
    
    if (list.keywords.isEmpty()) {
        SYNTAX_DEBUG_VERBOSE_LOG("  Keyword list" << listName << "is empty or not found");
        return false;
    }
    
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
    if (!m_formats.contains(attribute)) {
        SYNTAX_DEBUG_VERBOSE_LOG("  Format not found for attribute:" << attribute);
        return;
    }
    
    setFormat(start, length, m_formats[attribute]);
}

QString SyntaxHighlighter::resolveContext(const QString &context, 
                                           const QString &currentContext) const
{
    if (context == "#stay") {
        return currentContext;
    } else if (context == "#pop") {
        // 简化处理：返回Normal
        return "Normal";
    } else if (context.startsWith("#pop!")) {
        // 弹出并切换
        return context.mid(5);
    } else {
        return context;
    }
}
