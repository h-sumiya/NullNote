#pragma once

#include <QWidget>

class LinkAwarePlainTextEdit;

class NoteWindow final : public QWidget
{
public:
    explicit NoteWindow(QWidget *parent = nullptr);

private:
    LinkAwarePlainTextEdit *editor_ = nullptr;
};

