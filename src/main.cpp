#include "notewindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("NullNote"));
    app.setOrganizationName(QStringLiteral("NullNote"));

    NoteWindow window;
    window.show();

    return app.exec();
}
