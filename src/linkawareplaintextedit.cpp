#include "linkawareplaintextedit.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QMouseEvent>
#include <QPalette>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QTextLayout>
#include <QUrl>

namespace
{
struct NormalizedToken
{
    QString text;
    int leadingTrim = 0;
};

const QRegularExpression &tokenRegex()
{
    static const QRegularExpression regex(
        QStringLiteral(
            R"regex((https?://[^\s<>"']+)|("(?:[A-Za-z]:\\|\\\\|/)[^"\r\n]*")|('(?:[A-Za-z]:\\|\\\\|/)[^'\r\n]*')|((?:[A-Za-z]:\\|\\\\)[^\s<>"']+)|(/[^\s<>"']+))regex"));
    return regex;
}

const QRegularExpression &urlRegex()
{
    static const QRegularExpression regex(QStringLiteral(R"regex(^https?://)regex"),
                                          QRegularExpression::CaseInsensitiveOption);
    return regex;
}

const QRegularExpression &windowsPathRegex()
{
    static const QRegularExpression regex(QStringLiteral(R"regex(^(?:[A-Za-z]:\\|\\\\).+)regex"));
    return regex;
}

int trimTrailingPunctuation(QString &text)
{
    int trimmed = 0;

    while (!text.isEmpty()) {
        const QChar last = text.back();
        const bool trimSimple = QStringLiteral(".,;:!?").contains(last);
        const bool trimParen = last == u')' && text.count(u'(') < text.count(u')');
        const bool trimBracket = last == u']' && text.count(u'[') < text.count(u']');
        const bool trimBrace = last == u'}' && text.count(u'{') < text.count(u'}');

        if (!trimSimple && !trimParen && !trimBracket && !trimBrace) {
            break;
        }

        text.chop(1);
        ++trimmed;
    }

    return trimmed;
}

NormalizedToken normalizeToken(const QString &capturedText)
{
    NormalizedToken normalized;
    normalized.text = capturedText;

    if (normalized.text.size() >= 2) {
        const QChar first = normalized.text.front();
        const QChar last = normalized.text.back();
        if ((first == u'"' && last == u'"') || (first == u'\'' && last == u'\'')) {
            normalized.text = normalized.text.mid(1, normalized.text.size() - 2);
            normalized.leadingTrim = 1;
        }
    }

    trimTrailingPunctuation(normalized.text);
    return normalized;
}

LinkAwarePlainTextEdit::TargetKind detectKind(const QString &text)
{
    if (urlRegex().match(text).hasMatch()) {
        return LinkAwarePlainTextEdit::TargetKind::Url;
    }

    if (windowsPathRegex().match(text).hasMatch()) {
        return LinkAwarePlainTextEdit::TargetKind::WindowsPath;
    }

    if (text.startsWith(u'/')) {
        return LinkAwarePlainTextEdit::TargetKind::LinuxPath;
    }

    return LinkAwarePlainTextEdit::TargetKind::None;
}

QString convertWslPathToWindows(const QString &linuxPath)
{
    QProcess process;
    process.start(QStringLiteral("wsl.exe"),
                  {QStringLiteral("wslpath"), QStringLiteral("-w"), linuxPath});

    if (!process.waitForFinished(3000) || process.exitStatus() != QProcess::NormalExit ||
        process.exitCode() != 0) {
        return {};
    }

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
}

bool openExistingPathInExplorer(const QString &path)
{
    const QFileInfo info(path);
    if (info.exists()) {
        if (info.isFile()) {
            return QProcess::startDetached(QStringLiteral("explorer.exe"),
                                           {QStringLiteral("/select,") +
                                            QDir::toNativeSeparators(info.absoluteFilePath())});
        }

        return QProcess::startDetached(QStringLiteral("explorer.exe"),
                                       {QDir::toNativeSeparators(info.absoluteFilePath())});
    }

    const QFileInfo parent(info.absolutePath());
    if (parent.exists() && parent.isDir()) {
        return QProcess::startDetached(QStringLiteral("explorer.exe"),
                                       {QDir::toNativeSeparators(parent.absoluteFilePath())});
    }

    return false;
}

bool openMatch(const LinkAwarePlainTextEdit::Match &match)
{
    switch (match.kind) {
    case LinkAwarePlainTextEdit::TargetKind::Url:
        return QDesktopServices::openUrl(QUrl::fromUserInput(match.text));
    case LinkAwarePlainTextEdit::TargetKind::WindowsPath:
        return openExistingPathInExplorer(match.text);
    case LinkAwarePlainTextEdit::TargetKind::LinuxPath: {
        const QString converted = convertWslPathToWindows(match.text);
        return !converted.isEmpty() && openExistingPathInExplorer(converted);
    }
    case LinkAwarePlainTextEdit::TargetKind::None:
        break;
    }

    return false;
}
} // namespace

LinkAwarePlainTextEdit::LinkAwarePlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    connect(this, &QPlainTextEdit::textChanged, this, [this] {
        hoveredMatch_ = {};
        pressedMatch_ = {};
        applyHoverSelection();
        viewport()->unsetCursor();
    });
}

bool LinkAwarePlainTextEdit::Match::isValid() const
{
    return kind != TargetKind::None && start >= 0 && length > 0 && !text.isEmpty();
}

bool LinkAwarePlainTextEdit::Match::operator==(const Match &other) const
{
    return kind == other.kind && start == other.start && length == other.length && text == other.text;
}

LinkAwarePlainTextEdit::Match LinkAwarePlainTextEdit::matchAt(const QPoint &position) const
{
    const QTextCursor cursor = cursorForPosition(position);
    const QTextBlock block = cursor.block();
    if (!block.isValid()) {
        return {};
    }

    const QString line = block.text();
    if (line.isEmpty()) {
        return {};
    }

    const QTextLayout *layout = block.layout();
    if (layout == nullptr) {
        return {};
    }

    const QPointF blockPoint =
        QPointF(position) - blockBoundingGeometry(block).translated(contentOffset()).topLeft();

    QTextLine hitLine;
    for (int lineIndex = 0; lineIndex < layout->lineCount(); ++lineIndex) {
        const QTextLine candidate = layout->lineAt(lineIndex);
        if (candidate.isValid() && candidate.naturalTextRect().contains(blockPoint)) {
            hitLine = candidate;
            break;
        }
    }

    if (!hitLine.isValid()) {
        return {};
    }

    const int probe = hitLine.xToCursor(blockPoint.x(), QTextLine::CursorOnCharacter);
    if (probe < 0 || probe >= line.size()) {
        return {};
    }

    QRegularExpressionMatchIterator iterator = tokenRegex().globalMatch(line);
    while (iterator.hasNext()) {
        const QRegularExpressionMatch regexMatch = iterator.next();
        if (probe < regexMatch.capturedStart(0) || probe >= regexMatch.capturedEnd(0)) {
            continue;
        }

        const NormalizedToken normalized = normalizeToken(regexMatch.captured(0));
        const TargetKind kind = detectKind(normalized.text);
        if (kind == TargetKind::None || normalized.text.isEmpty()) {
            continue;
        }

        Match match;
        match.kind = kind;
        match.start = block.position() + regexMatch.capturedStart(0) + normalized.leadingTrim;
        match.length = normalized.text.size();
        match.text = normalized.text;
        return match;
    }

    return {};
}

void LinkAwarePlainTextEdit::applyHoverSelection()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (hoveredMatch_.isValid()) {
        QTextEdit::ExtraSelection selection;
        QTextCharFormat format;
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        format.setUnderlineColor(palette().color(QPalette::Link));
        format.setForeground(palette().color(QPalette::Link));

        QTextCursor cursor(document());
        cursor.setPosition(hoveredMatch_.start);
        cursor.setPosition(hoveredMatch_.start + hoveredMatch_.length, QTextCursor::KeepAnchor);
        selection.cursor = cursor;
        selection.format = format;
        selections.append(selection);
    }

    setExtraSelections(selections);
}

void LinkAwarePlainTextEdit::clearHover()
{
    if (!hoveredMatch_.isValid()) {
        return;
    }

    hoveredMatch_ = {};
    applyHoverSelection();
    viewport()->unsetCursor();
}

void LinkAwarePlainTextEdit::updateHover(const QPoint &position)
{
    const Match nextMatch = matchAt(position);
    if (nextMatch == hoveredMatch_) {
        return;
    }

    hoveredMatch_ = nextMatch;
    applyHoverSelection();

    if (hoveredMatch_.isValid()) {
        viewport()->setCursor(Qt::PointingHandCursor);
    } else {
        viewport()->unsetCursor();
    }
}

void LinkAwarePlainTextEdit::leaveEvent(QEvent *event)
{
    clearHover();
    QPlainTextEdit::leaveEvent(event);
}

void LinkAwarePlainTextEdit::mouseMoveEvent(QMouseEvent *event)
{
    updateHover(event->pos());
    QPlainTextEdit::mouseMoveEvent(event);
}

void LinkAwarePlainTextEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        pressedMatch_ = matchAt(event->pos());
        if (pressedMatch_.isValid()) {
            event->accept();
            return;
        }
    }

    pressedMatch_ = {};
    QPlainTextEdit::mousePressEvent(event);
}

void LinkAwarePlainTextEdit::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && pressedMatch_.isValid()) {
        const Match releasedMatch = matchAt(event->pos());
        const bool shouldOpen = releasedMatch == pressedMatch_;
        pressedMatch_ = {};

        if (shouldOpen && !openMatch(releasedMatch)) {
            QApplication::beep();
        }

        event->accept();
        return;
    }

    pressedMatch_ = {};
    QPlainTextEdit::mouseReleaseEvent(event);
}
