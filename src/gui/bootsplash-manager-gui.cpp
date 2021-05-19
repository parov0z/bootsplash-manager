#include "mainwindow.h"

#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <QObject>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>

#include <QStyle>
#include <QDesktopWidget>
#include <QScreen>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;

    translator.load( "bootsplash-manager_" + QLocale::system().name(),
                     QStandardPaths::locate( QStandardPaths::GenericDataLocation,
                                             QStringLiteral("bootsplash-manager/translations/"),
                                             QStandardPaths::LocateDirectory )                   );
    a.installTranslator( &translator );

    QProcess readconf;
    readconf.start( "zgrep", QStringList() << "CONFIG_BOOTSPLASH" << "/proc/config.gz" );
    readconf.waitForFinished();
    if( QString( readconf.readAllStandardOutput() ) != "CONFIG_BOOTSPLASH=y\n" ){
        QMessageBox m;
        m.setWindowTitle( "Warning" );
        m.setText( QObject::tr("Cannot detect kernel bootsplash support\n"
                   "Is it a custom kernel?\n"
                   "Themes will not work, but you can still enable/disable the displaying of the boot log") );
        m.setIcon( QMessageBox::Warning );
        m.exec();
    }
    readconf.close();

    MainWindow w;
    w.setGeometry( QStyle::alignedRect( Qt::LeftToRight,
                                        Qt::AlignCenter,
                                        w.size(),
                                        QGuiApplication::screens().at(0)->availableGeometry() )  );
    w.show();
    return a.exec();
}
