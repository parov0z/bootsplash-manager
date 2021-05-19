#include "root-actions.h"

#include <QCoreApplication>
#include <stdio.h>
#include <QCommandLineParser>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <unistd.h>
#include <QProcess>

int help();
int list();
int status();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream out( stdout );

    if ( argc>3 ){
        out << "too many arguments\n\n";
        return help();
    }
    if ( argc==2 ){
        if ( strcmp( argv[1], "--status"  ) == 0 )    return status();

        if ( strcmp( argv[1], "-l"        ) == 0
          || strcmp( argv[1], "--list"    ) == 0 )    return list();

        if ( strcmp( argv[1], "-d"        ) == 0
          || strcmp( argv[1], "--disable" ) == 0 )    return rootActions::disable();

        if ( strcmp( argv[1], "--set-log" ) == 0 )    return rootActions::setLog();

        if ( strcmp( argv[1], "-h"        ) == 0
          || strcmp( argv[1], "--help"    ) == 0 )    return help();
    }
    if ( argc==3 ){
        if ( strcmp( argv[1], "-s"        ) == 0
          || strcmp( argv[1], "--set"     ) == 0 )    return rootActions::setTheme( argv[2] );
    }
    out << "unknown command\n\n";
    out.flush();
    return help();
}

int help(){
    QTextStream out( stdout );
    out << "Available commands:\n"
        << " -h, " << "--help    "      << "\t\t" << "Show this message\n"
        << " -l, " << "--list    "      << "\t\t" << "List installed themes\n"
        << " -s, " << "--set <theme_name>"<< "\t" << "Enable bootsplash if it's not, change current theme,\n"
                                                     <<"\t\t\t   e.g. for \"bootsplash-theme-manjaro\" correct <theme_name> will be \"manjaro\"\n"
        << " -d, " << "--disable "      << "\t\t" << "Disable bootsplash\n"
                   << "     --set-log " << "\t\t" << "Disable bootsplash, \"quiet\" option will not be added. You will see log during boot\n"
                   << "     --status  " << "\t\t" << "Check if bootsplash is enabled and supported by kernel; show selected theme\n";

    return 0;
}

int list(){
    QTextStream out( stdout );
    QDir d( "/usr/lib/firmware/bootsplash-themes/" );
    d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    if ( d.count() == 0 )
        out << "There are no installed themes\n";
    else{
        QStringList themes = d.entryList();
        out << "Found " << themes.count() << " themes:\n";
        for ( const QString& n : qAsConst(themes) ) out << n << "\n";
    }
    return 0;
}

int status(){
    QTextStream out( stdout );

    bool kernelFlag = false, quietFlag = false, bootfileFlag = false, hooksFlag = false;

    QProcess readconf;
    readconf.start( "zgrep", QStringList() << "CONFIG_BOOTSPLASH" << "/proc/config.gz" );
    readconf.waitForFinished();
    kernelFlag = QString(readconf.readAllStandardOutput()) == "CONFIG_BOOTSPLASH=y\n";


    QFile grub( "/etc/default/grub" );
    if ( !grub.open( QFile::ReadOnly | QFile::Text ) )
        return -1;
    QString cmdline;
    while ( !cmdline.contains( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT=") ) ) cmdline = grub.readLine();

    QStringList cmdlineList = cmdline.replace( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT="), "" )
                                     .replace( "\"", "" )
                                     .split( ' ' );
    QString theme;
    for ( const QString& opt : qAsConst(cmdlineList) ){
        if ( opt=="quiet" ) quietFlag = true;
        if ( opt.contains( "bootsplash.bootfile=" ) ){
            bootfileFlag = true;
            theme=opt;
        }
    }
    theme.replace( "bootsplash.bootfile=/bootsplash-themes/", "" ).replace( "/bootsplash", "" ).replace( "\n", "" );

    QString ok = "[\033[42m ok \033[00m]",
          warn = "[\033[43mwarn\033[00m]";

    out << "Bootsplash status:\n"
        << ( ( bootfileFlag&!theme.isEmpty() )?
                   ok+" Bootsplash is enabled, current theme: "+theme+"\n"
                :warn+" Bootsplash is disabled or theme is not set\n" )
        << ( kernelFlag?
                   ok+" Current kernel supports bootsplash\n"
                :warn+" Seems like your current kernel doesn't have bootsplash support\n" );

    if ( !theme.isEmpty() ){
        QFile initcpio( "/etc/mkinitcpio.conf" );
        if ( !initcpio.open( QFile::ReadOnly | QFile::Text ) )
            return -1;
        QString hooks;
        while ( !hooks.contains( QRegularExpression("^HOOKS=") ) )
            hooks = initcpio.readLine();
        hooksFlag = hooks.contains( QRegularExpression("[\\s,\",(]bootsplash-"+theme+"[\\s,\",)]") );

        out << ( hooksFlag?
                   ok+" current theme is in hooks list\n"
                :warn+" current theme not found in hooks list, you can add it manually or run:\nsudo bootsplash-manager -s "+theme+"\n" )
            << ( !quietFlag?
                   ok+" No \'quiet\' option\n"
                :warn+" Found \'quiet\' option, you'll see black screen instead of bootsplash\n" );
    }
    return 0;
}
