#include "notewindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("NullNote"));
    app.setOrganizationName(QStringLiteral("NullNote"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/icon.png")));

    NoteWindow window;
    window.show();

    return app.exec();
}
