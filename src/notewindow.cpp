#include "notewindow.h"

#include "linkawareplaintextedit.h"

#include <QFont>
#include <QFontDatabase>
#include <QFrame>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QWidget>

NoteWindow::NoteWindow(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QStringLiteral("NullNote"));
    resize(760, 560);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    editor_ = new LinkAwarePlainTextEdit(this);
    editor_->setFrameStyle(QFrame::NoFrame);
    editor_->document()->setDocumentMargin(0);
    editor_->setPlaceholderText(
        QStringLiteral("Type anything here.\nClick URLs, C:\\\\paths, and WSL-style /paths to open them."));

    QFont editorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (editorFont.pointSizeF() > 0.0) {
        editorFont.setPointSizeF(editorFont.pointSizeF() + 1.0);
    } else {
        editorFont.setPointSize(11);
    }
    editor_->setFont(editorFont);

    rootLayout->addWidget(editor_, 1);

    setStyleSheet(QStringLiteral(R"css(
        QWidget {
            background: #fcfbf7;
            color: #1d1b17;
        }
        QPlainTextEdit {
            background: #fcfbf7;
            border: none;
            padding-top: 0;
            padding-right: 14px;
            padding-bottom: 14px;
            padding-left: 14px;
            selection-background-color: #cbdff9;
        }
    )css"));
}
