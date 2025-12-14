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
    qDebug() << "SyntaxHighlighter::setSyntaxDefinition called with:" << definition.name;
    qDebug() << "  Contexts count:" << definition.contexts.size();
    qDebug() << "  Keywords count:" << definition.keywords.size();
    qDebug() << "  ItemDatas count:" << definition.itemDatas.size();
    
    initFormats();
    rehighlight();
}

void SyntaxHighlighter::setTheme(ThemeLoader *themeLoader)
{
    m_themeLoader = themeLoader;
    qDebug() << "SyntaxHighlighter::setTheme called with:" << (themeLoader ? themeLoader->themeName() : "null");
    
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
    
    qDebug() << "SyntaxHighlighter::initFormats - creating formats for" << m_definition.itemDatas.size() << "items";
    
    // 为每个itemData创建格式
    for (auto it = m_definition.itemDatas.constBegin(); 
         it != m_definition.itemDatas.constEnd(); ++it) {
        QString attributeName = it.key();
        QString styleName = it.value();
        
        TextStyle style = m_themeLoader->getTextStyle(styleName);
        m_formats[attributeName] = style.toTextCharFormat();
        
        qDebug() << "  Format created:" << attributeName << "->" << styleName;
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    
    if (m_definition.contexts.isEmpty()) {
        return;
    }
    
    // C++ 语法定义使用 "Normal" 作为默认上下文
    QString currentContext = "Normal";
    
    // 如果找不到 "Normal"，使用第一个可用的上下文
    if (!m_definition.contexts.contains(currentContext) && !m_definition.contexts.isEmpty()) {
        currentContext = m_definition.contexts.firstKey();
    }
    
    Context context = m_definition.getContext(currentContext);
    if (context.name.isEmpty()) {
        return;
    }
    
    int position = 0;
    
    while (position < text.length()) {
        bool matched = false;
        
        // 尝试应用每个规则
        for (const ContextRule &rule : context.rules) {
            // 处理 IncludeRules
            if (rule.type == ContextRule::IncludeRules) {
                // 解析包含的上下文名称
                QString includeContext = rule.context;
                // 处理 ##ISO C++ 这样的外部引用（暂时跳过）
                if (includeContext.startsWith("##")) {
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
            }
            position++;
        }
    }
    
    // 保存当前状态
    setCurrentBlockState(0); // 简化处理
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
    if (context == "#stay") {
        return currentContext;
    } else if (context == "#pop") {
        // 简化处理：返回Main
        return "Main";
    } else if (context.startsWith("#pop!")) {
        // 弹出并切换
        return context.mid(5);
    } else {
        return context;
    }
}
