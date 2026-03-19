#pragma once

#include <QPlainTextEdit>

class QEvent;
class QMouseEvent;

class LinkAwarePlainTextEdit final : public QPlainTextEdit
{
public:
    enum class TargetKind
    {
        None,
        Url,
        WindowsPath,
        LinuxPath,
    };

    struct Match
    {
        TargetKind kind = TargetKind::None;
        int start = -1;
        int length = 0;
        QString text;

        [[nodiscard]] bool isValid() const;
        [[nodiscard]] bool operator==(const Match &other) const;
    };

    explicit LinkAwarePlainTextEdit(QWidget *parent = nullptr);

protected:
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    [[nodiscard]] Match matchAt(const QPoint &position) const;
    void applyHoverSelection();
    void clearHover();
    void updateHover(const QPoint &position);

    Match hoveredMatch_;
    Match pressedMatch_;
};
