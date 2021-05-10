#ifndef ROOTACTIONS_H
#define ROOTACTIONS_H

#include <QString>
#include <QProcess>

namespace rootActions {

    int setTheme( const QString& theme );
    int setLog();
    int disable();

}
#endif // ROOTACTIONS_H
