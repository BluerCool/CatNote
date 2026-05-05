#pragma once

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QRegularExpression>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat headingFormat;
    QTextCharFormat boldFormat;
    QTextCharFormat italicFormat;
    QTextCharFormat boldItalicFormat;
    QTextCharFormat codeFormat;
    QTextCharFormat codeBlockFormat;
    QTextCharFormat linkFormat;
    QTextCharFormat listFormat;
    QTextCharFormat quoteFormat;
    QTextCharFormat strikeFormat;
    QTextCharFormat htmlTagFormat;

    QRegularExpression codeBlockStartExpression;
    QRegularExpression codeBlockEndExpression;
};