#include "linkmodel.h"

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <QScopedPointer>
#include <sailfishapp.h>

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    app->setOrganizationName(QStringLiteral("OfficeFake"));
    app->setApplicationName(QStringLiteral("harbour-sailfish-link"));

    LinkModel model;

    QScopedPointer<QQuickView> view(SailfishApp::createView());
    view->rootContext()->setContextProperty(QStringLiteral("linkModel"), &model);
    view->setSource(SailfishApp::pathTo(QStringLiteral("qml/harbour-sailfish-link.qml")));
    view->showFullScreen();

    return app->exec();
}
