#include <QtGui/QApplication>
#include <QtDeclarative>
#include <QWebSettings>

#include "qmlapplicationviewer.h"
#include "networkaccessmanagerfactory.h"
#include "qmlapi.h"
#include "musicfetcher.h"
#include "musiccollector.h"
#include "blurreditem.h"
#include "musicdownloader.h"
#include "musicdownloadmodel.h"
#include "musicdownloaddatabase.h"
#include "lyricloader.h"

#if defined(Q_OS_HARMATTAN) || defined(SIMULATE_HARMATTAN)
#include "harmattanbackgroundprovider.h"
#endif

//#define PROXY_HOST "192.168.1.64"

#ifdef PROXY_HOST
#include <QNetworkProxy>
#endif

#define RegisterPlugin(Plugin) \
    qmlRegisterType<Plugin>("com.yeatse.cloudmusic", 1, 0, #Plugin)

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef Q_OS_SYMBIAN
    QApplication::setAttribute((Qt::ApplicationAttribute)11);   //Qt::AA_CaptureMultimediaKeys
#endif
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    app->setApplicationName("CloudMusic");
    app->setOrganizationName("Yeatse");
    app->setApplicationVersion(VER);

#ifdef PROXY_HOST
    QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::HttpProxy, PROXY_HOST, 8888));
#endif

    RegisterPlugin(MusicInfo);
    RegisterPlugin(MusicFetcher);
    RegisterPlugin(BlurredItem);
    RegisterPlugin(MusicDownloadModel);
    RegisterPlugin(LyricLoader);

    QWebSettings::globalSettings()->setUserStyleSheetUrl(QUrl::fromLocalFile("qml/js/default_theme.css"));

    QScopedPointer<QmlApplicationViewer> viewer(new QmlApplicationViewer);
    viewer->setAttribute(Qt::WA_OpaquePaintEvent);
    viewer->setAttribute(Qt::WA_NoSystemBackground);
    viewer->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    viewer->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationLockPortrait);

    QScopedPointer<NetworkAccessManagerFactory> factory(new NetworkAccessManagerFactory);
    viewer->engine()->setNetworkAccessManagerFactory(factory.data());

#if defined(Q_OS_HARMATTAN) || defined(SIMULATE_HARMATTAN)
    HarmattanBackgroundProvider* provider = new HarmattanBackgroundProvider;
    viewer->engine()->addImageProvider("appBackground", provider);
    viewer->rootContext()->setContextProperty("bgProvider", provider);
#endif

    viewer->rootContext()->setContextProperty("qmlApi", new QmlApi(viewer.data()));
    viewer->rootContext()->setContextProperty("collector", new MusicCollector(viewer.data()));
    viewer->rootContext()->setContextProperty("appVersion", app->applicationVersion());
    viewer->rootContext()->setContextProperty("qtVersion", qVersion());

    MusicDownloader* downloader = MusicDownloader::Instance();
    viewer->rootContext()->setContextProperty("downloader", downloader);
    downloader->pause();
    QObject::connect(qApp, SIGNAL(aboutToQuit()), downloader, SLOT(pause()));
    QObject::connect(qApp, SIGNAL(aboutToQuit()), MusicDownloadDatabase::Instance(), SLOT(freeResource()));

#if defined(Q_OS_HARMATTAN) || defined(SIMULATE_HARMATTAN)
    viewer->setMainQmlFile(QLatin1String("qml/harmattan/main.qml"));
#else
    viewer->setMainQmlFile(QLatin1String("qml/cloudmusicqt/main.qml"));
#endif
    viewer->showExpanded();

    return app->exec();
}
