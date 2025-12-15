#include "DiffSyntaxHighlighter.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QDebug>

DiffSyntaxHighlighter::DiffSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_tempDoc(nullptr)
    , m_codeHighlighter(nullptr)
    , m_themeLoader(nullptr)
{
    // 创建持久的临时文档用于代码高亮
    m_tempDoc = new QTextDocument(this);
    
    initDiffFormats();
    
    qDebug() << "DiffSyntaxHighlighter constructed";
}

void DiffSyntaxHighlighter::setCodeSyntaxDefinition(const SyntaxDefinition &definition)
{
    qDebug() << "DiffSyntaxHighlighter::setCodeSyntaxDefinition:" << definition.name;
    
    // 如果还没有创建代码高亮器，现在创建
    if (!m_codeHighlighter) {
        if (!m_tempDoc) {
            m_tempDoc = new QTextDocument(this);
        }
        m_codeHighlighter = new SyntaxHighlighter(m_tempDoc);
        qDebug() << "  Created SyntaxHighlighter with temp doc";
    }
    
    m_codeHighlighter->setSyntaxDefinition(definition);
    qDebug() << "  Applied syntax definition:" << definition.name;
    
    // ========== 添加详细的调试日志 =========
    qDebug() << "=== Syntax Definition Details ===";
    qDebug() << "  Name:" << definition.name;
    qDebug() << "  File:" << definition.fileName;  // 输出原始文件名
    qDebug() << "  Section:" << definition.section;
    qDebug() << "  Extensions:" << definition.extensions;
    qDebug() << "  Case Sensitive:" << definition.caseSensitive;
    qDebug() << "  Priority:" << definition.priority;
    
    // 输出所有上下文及其规则
    qDebug() << "  Contexts count:" << definition.contexts.size();
    for (auto contextIt = definition.contexts.constBegin(); 
         contextIt != definition.contexts.constEnd(); ++contextIt) {
        const Context &context = contextIt.value();
        qDebug() << "    Context:" << context.name 
                 << "rules:" << context.rules.size()
                 << "attribute:" << context.attribute;
        
        // 只输出前10个规则，避免日志过多
        int ruleCount = qMin(10, context.rules.size());
        for (int i = 0; i < ruleCount; ++i) {
            const ContextRule &rule = context.rules[i];
            
            QString ruleTypeStr;
            switch (rule.type) {
                case ContextRule::DetectChar: ruleTypeStr = "DetectChar"; break;
                case ContextRule::Detect2Chars: ruleTypeStr = "Detect2Chars"; break;
                case ContextRule::AnyChar: ruleTypeStr = "AnyChar"; break;
                case ContextRule::StringDetect: ruleTypeStr = "StringDetect"; break;
                case ContextRule::WordDetect: ruleTypeStr = "WordDetect"; break;
                case ContextRule::RegExpr: ruleTypeStr = "RegExpr"; break;
                case ContextRule::Keyword: ruleTypeStr = "Keyword"; break;
                case ContextRule::Int: ruleTypeStr = "Int"; break;
                case ContextRule::Float: ruleTypeStr = "Float"; break;
                case ContextRule::HlCHex: ruleTypeStr = "HlCHex"; break;
                case ContextRule::DetectSpaces: ruleTypeStr = "DetectSpaces"; break;
                case ContextRule::DetectIdentifier: ruleTypeStr = "DetectIdentifier"; break;
                case ContextRule::IncludeRules: ruleTypeStr = "IncludeRules"; break;
                case ContextRule::LineContinue: ruleTypeStr = "LineContinue"; break;
                case ContextRule::RangeDetect: ruleTypeStr = "RangeDetect"; break;
                default: ruleTypeStr = "Unknown"; break;
            }
            
            qDebug() << "      Rule" << i << ":" << ruleTypeStr
                     << "attr:" << rule.attribute
                     << "ctx:" << (rule.context.isEmpty() ? "#stay" : rule.context);
            
            // 根据规则类型输出额外信息
            if (rule.type == ContextRule::Keyword) {
                qDebug() << "        -> Keyword list:" << rule.keywordList;
            } else if (rule.type == ContextRule::StringDetect || 
                       rule.type == ContextRule::WordDetect) {
                qDebug() << "        -> String:" << rule.string.left(30);
            } else if (rule.type == ContextRule::RegExpr) {
                qDebug() << "        -> Pattern:" << rule.regex.pattern().left(40);
            } else if (rule.type == ContextRule::DetectChar) {
                qDebug() << "        -> Char:" << rule.char0;
            } else if (rule.type == ContextRule::Detect2Chars) {
                qDebug() << "        -> Chars:" << rule.char0 << rule.char1;
            }
        }
        
        if (context.rules.size() > 10) {
            qDebug() << "      ... and" << (context.rules.size() - 10) << "more rules";
        }
    }
    
    // 输出关键字列表
    qDebug() << "  Keyword lists count:" << definition.keywords.size();
    for (auto kwIt = definition.keywords.constBegin(); 
         kwIt != definition.keywords.constEnd(); ++kwIt) {
        const KeywordList &kwList = kwIt.value();
        qDebug() << "    List:" << kwList.name 
                 << "keywords:" << kwList.keywords.size();
        
        // 只输出前5个关键字
        int kwCount = qMin(5, kwList.keywords.size());
        QStringList sample = kwList.keywords.mid(0, kwCount);
        qDebug() << "      Sample:" << sample.join(", ");
        if (kwList.keywords.size() > 5) {
            qDebug() << "      ... and" << (kwList.keywords.size() - 5) << "more";
        }
    }
    
    // 输出样式映射
    qDebug() << "  ItemDatas (style mappings) count:" << definition.itemDatas.size();
    for (auto itemIt = definition.itemDatas.constBegin(); 
         itemIt != definition.itemDatas.constEnd(); ++itemIt) {
        qDebug() << "    " << itemIt.key() << "=>" << itemIt.value();
    }
    
    qDebug() << "=== End of Syntax Definition Details ===";
}

void DiffSyntaxHighlighter::setTheme(ThemeLoader *themeLoader)
{
    qDebug() << "DiffSyntaxHighlighter::setTheme:" << (themeLoader ? themeLoader->themeName() : "null");
    
    m_themeLoader = themeLoader;
    
    if (m_codeHighlighter && themeLoader) {
        m_codeHighlighter->setTheme(themeLoader);
        qDebug() << "  Applied theme to code highlighter";
    } else {
        qWarning() << "  Cannot apply theme:"
                   << "codeHighlighter=" << (m_codeHighlighter != nullptr)
                   << "themeLoader=" << (themeLoader != nullptr);
    }
    
    initDiffFormats();
}

void DiffSyntaxHighlighter::initDiffFormats()
{
    // 浅绿色背景 - 添加的行
    m_addedLineFormat.setBackground(QColor(230, 255, 230));  // 很浅的绿色
    
    // 浅红色背景 - 删除的行
    m_removedLineFormat.setBackground(QColor(255, 230, 230));  // 很浅的红色
    
    // 普通行 - 使用默认背景
    m_contextLineFormat.setBackground(Qt::transparent);
    
    // 头部行 - 灰色背景
    m_headerFormat.setBackground(QColor(240, 240, 240));
    m_headerFormat.setForeground(QColor(100, 100, 100));
    m_headerFormat.setFontWeight(QFont::Bold);
}

DiffSyntaxHighlighter::LineType DiffSyntaxHighlighter::detectLineType(const QString &text) const
{
    if (text.isEmpty()) {
        return EmptyLine;
    }
    
    QChar firstChar = text.at(0);
    
    // 检测 diff 头部
    if (text.startsWith("diff ") || 
        text.startsWith("index ") ||
        text.startsWith("--- ") ||
        text.startsWith("+++ ") ||
        text.startsWith("@@ ")) {
        return HeaderLine;
    }
    
    // 检测添加/删除行
    if (firstChar == '+') {
        return AddedLine;
    } else if (firstChar == '-') {
        return RemovedLine;
    }
    
    return ContextLine;
}

void DiffSyntaxHighlighter::applyDiffHighlight(const QString &text, LineType lineType)
{
    qDebug() << "applyDiffHighlight: Line:" << text.left(50) << "Type:" << lineType;
    
    // 首先应用整行的背景色
    QTextCharFormat bgFormat;
    QString codeText = text;  // 去除前缀后的代码
    int codeStartPos = 0;     // 代码开始的位置
    
    switch (lineType) {
        case AddedLine:
            bgFormat = m_addedLineFormat;
            if (!text.isEmpty() && text.at(0) == '+') {
                codeText = text.mid(1);  // 去除 + 前缀
                codeStartPos = 1;
            }
            // 设置整行背景
            setFormat(0, text.length(), bgFormat);
            qDebug() << "  Added line, code:" << codeText.left(30);
            break;
            
        case RemovedLine:
            bgFormat = m_removedLineFormat;
            if (!text.isEmpty() && text.at(0) == '-') {
                codeText = text.mid(1);  // 去除 - 前缀
                codeStartPos = 1;
            }
            // 设置整行背景
            setFormat(0, text.length(), bgFormat);
            qDebug() << "  Removed line, code:" << codeText.left(30);
            break;
            
        case HeaderLine:
            // Diff 头部行，只设置格式，不做代码高亮
            setFormat(0, text.length(), m_headerFormat);
            qDebug() << "  Header line";
            return;
            
        case ContextLine:
            bgFormat = m_contextLineFormat;
            codeText = text;
            codeStartPos = 0;
            qDebug() << "  Context line, code:" << codeText.left(30);
            break;
            
        case EmptyLine:
            qDebug() << "  Empty line";
            return;
    }
    
    // 检查代码高亮器状态
    if (!m_codeHighlighter) {
        qWarning() << "  No code highlighter available!";
        return;
    }
    
    if (codeText.isEmpty()) {
        qDebug() << "  Code text is empty, skipping highlight";
        return;
    }
    
    qDebug() << "  Applying code highlight to:" << codeText.left(30);
    qDebug() << "  m_codeHighlighter=" << m_codeHighlighter << "m_tempDoc=" << m_tempDoc;
    
    // 设置临时文档的内容
    m_tempDoc->setPlainText(codeText);
    
    // 应用代码高亮
    m_codeHighlighter->setDocument(m_tempDoc);
    m_codeHighlighter->rehighlight();
    
    // 获取高亮后的格式并应用到原文档
    QTextBlock block = m_tempDoc->firstBlock();
    if (block.isValid()) {
        qDebug() << "  Processing block with" << block.length() << "chars";
        
        QTextBlock::iterator it;
        int fragmentCount = 0;
        for (it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid()) {
                fragmentCount++;
                QTextCharFormat format = fragment.charFormat();
                
                // 保留背景色，只应用前景色和字体样式
                format.setBackground(bgFormat.background());
                
                int fragmentStart = fragment.position();
                int fragmentLength = fragment.length();
                
                // 映射到原文档的位置（加上前缀偏移）
                setFormat(codeStartPos + fragmentStart, fragmentLength, format);
                
                qDebug() << "    Fragment" << fragmentCount 
                         << "[" << fragmentStart << "," << (fragmentStart + fragmentLength) << "]"
                         << "text:'" << fragment.text().left(10) << "'"
                         << "fg:" << format.foreground().color().name();
            }
        }
        
        qDebug() << "  Applied" << fragmentCount << "fragments";
    } else {
        qWarning() << "  Block is invalid!";
    }
    
    // 清理：不需要设置回 nullptr，保持 m_tempDoc 关联
}

void DiffSyntaxHighlighter::highlightBlock(const QString &text)
{
    static int blockCounter = 0;
    int currentBlockId = ++blockCounter;
    
    // 只在前10个block和每100个block输出详细日志
    bool verbose = (currentBlockId <= 10 || currentBlockId % 100 == 0);
    
    if (verbose) {
        qDebug() << "=== DiffSyntaxHighlighter Block" << currentBlockId << "===" << text.left(60);
        qDebug() << "  m_codeHighlighter=" << m_codeHighlighter;
    }
    
    // 检测行类型
    LineType lineType = detectLineType(text);
    
    // 应用 Diff 格式和代码高亮
    applyDiffHighlight(text, lineType);
    
    if (verbose) {
        qDebug() << "=== Block" << currentBlockId << "END ===";
    }
}
