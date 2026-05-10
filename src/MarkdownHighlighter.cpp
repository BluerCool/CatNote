#include "MarkdownHighlighter.h"
#include "DesignSystem.h"

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // 标题格式
    headingFormat.setForeground(DesignSystem::instance()->primaryColor());
    headingFormat.setFontWeight(QFont::Bold);
    for (int i = 1; i <= 6; ++i) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(QString("^#{1,%1}\\s.*").arg(i));
        rule.format = headingFormat;
        highlightingRules.append(rule);
    }

    // 粗体 **text** 或 __text__
    boldFormat.setFontWeight(QFont::Bold);
    HighlightingRule boldRule;
    boldRule.pattern = QRegularExpression("(\\*\\*|__)(.+?)\\1");
    boldRule.format = boldFormat;
    highlightingRules.append(boldRule);

    // 斜体 *text* 或 _text_
    italicFormat.setFontItalic(true);
    italicFormat.setForeground(QColor(100, 100, 100));
    HighlightingRule italicRule;
    italicRule.pattern = QRegularExpression("(\\*|_)(.+?)\\1");
    italicRule.format = italicFormat;
    highlightingRules.append(italicRule);

    // 粗斜体 ***text***
    boldItalicFormat.setFontWeight(QFont::Bold);
    boldItalicFormat.setFontItalic(true);
    HighlightingRule boldItalicRule;
    boldItalicRule.pattern = QRegularExpression("(\\*\\*\\*|___)(.+?)\\1");
    boldItalicRule.format = boldItalicFormat;
    highlightingRules.append(boldItalicRule);

    // 删除线 ~~text~~
    strikeFormat.setFontStrikeOut(true);
    strikeFormat.setForeground(QColor(150, 150, 150));
    HighlightingRule strikeRule;
    strikeRule.pattern = QRegularExpression("~~(.+?)~~");
    strikeRule.format = strikeFormat;
    highlightingRules.append(strikeRule);

    // 行内代码 `code`
    codeFormat.setFontFamilies({"Consolas", "Monaco", "monospace"});
    codeFormat.setBackground(QColor(240, 240, 240));
    codeFormat.setForeground(QColor(200, 50, 50));
    HighlightingRule codeRule;
    codeRule.pattern = QRegularExpression("`([^`]+)`");
    codeRule.format = codeFormat;
    highlightingRules.append(codeRule);

    // 代码块 ```code```
    codeBlockFormat.setFontFamilies({"Consolas", "Monaco", "monospace"});
    codeBlockFormat.setBackground(QColor(245, 245, 245));
    codeBlockFormat.setForeground(QColor(50, 50, 50));
    codeBlockStartExpression = QRegularExpression("^\\s*```");
    codeBlockEndExpression = QRegularExpression("^\\s*```");

    // 链接 [text](url)
    linkFormat.setForeground(QColor(22, 119, 255));
    linkFormat.setFontUnderline(true);
    HighlightingRule linkRule;
    linkRule.pattern = QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)");
    linkRule.format = linkFormat;
    highlightingRules.append(linkRule);

    // 列表项 - 或 * 或 数字.
    listFormat.setForeground(QColor(22, 119, 255));
    HighlightingRule listRule;
    listRule.pattern = QRegularExpression("^\\s*([\\-\\*\\+]|[0-9]+\\.)\\s");
    listRule.format = listFormat;
    highlightingRules.append(listRule);

    // 引用 >
    quoteFormat.setForeground(QColor(100, 150, 100));
    quoteFormat.setFontItalic(true);
    HighlightingRule quoteRule;
    quoteRule.pattern = QRegularExpression("^\\s*>\\s.*");
    quoteRule.format = quoteFormat;
    highlightingRules.append(quoteRule);

    // HTML标签
    htmlTagFormat.setForeground(QColor(150, 100, 50));
    HighlightingRule htmlRule;
    htmlRule.pattern = QRegularExpression("<[^>]+>");
    htmlRule.format = htmlTagFormat;
    highlightingRules.append(htmlRule);
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        QRegularExpressionMatch match = codeBlockStartExpression.match(text);
        startIndex = match.hasMatch() ? match.capturedStart() : -1;
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = codeBlockEndExpression.match(text, startIndex + 3);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : text.length();
        int commentLength = endIndex - startIndex + (endMatch.hasMatch() ? endMatch.capturedLength() : 0);
        setFormat(startIndex, commentLength, codeBlockFormat);
        if (!endMatch.hasMatch()) {
            setCurrentBlockState(1);
            break;
        }
        QRegularExpressionMatch nextMatch = codeBlockStartExpression.match(text, endIndex);
        startIndex = nextMatch.hasMatch() ? nextMatch.capturedStart() : -1;
    }
}