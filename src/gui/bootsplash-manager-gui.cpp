#include "mainwindow.h"

#include <QApplication>
#include <QProcess>
#include <QMessageBox>

#include <QStyle>
#include <QDesktopWidget>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QProcess readconf;
    readconf.start( "zgrep", QStringList() << "CONFIG_BOOTSPLASH" << "/proc/config.gz" );
    readconf.waitForFinished();
    if( QString( readconf.readAllStandardOutput() ) != "CONFIG_BOOTSPLASH=y\n" ){
        QMessageBox m;
        m.setWindowTitle( "Warning" );
        m.setText( "Cannot detect kernel bootsplash support\n"
                   "Is it a custom kernel?\n"
                   "Themes will not work, but you can still enable/disable the displaying of the boot log" );
        m.setIcon( QMessageBox::Warning );
        m.exec();
    }
    readconf.close();

    MainWindow w;
    w.setGeometry( QStyle::alignedRect( Qt::LeftToRight,
                                        Qt::AlignCenter,
                                        w.size(),
                                        qApp->desktop()->availableGeometry() )
                                                                                );
    w.show();
    return a.exec();
}
