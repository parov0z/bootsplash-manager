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
int setTheme( const QString& theme );
int setLog();
int disable();
int list();
int status();
int initcpioClear(); // removes all the themes from /etc/mkinitcpio.conf

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
          || strcmp( argv[1], "--disable" ) == 0 )    return disable();

        if ( strcmp( argv[1], "--set-log" ) == 0 )    return setLog();

        if ( strcmp( argv[1], "-h"        ) == 0
          || strcmp( argv[1], "--help"    ) == 0 )    return help();
    }
    if ( argc==3 ){
        if ( strcmp( argv[1], "-s"        ) == 0
          || strcmp( argv[1], "--set"     ) == 0 )    return setTheme( argv[2] );
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

int setTheme( const QString& theme ){
    QTextStream out( stdout );

    if ( !QDir( "/usr/lib/firmware/bootsplash-themes/"+theme ).exists() ) {
        out << "Incorrect theme name \""+theme+"\"\n";
        return 0;
    }
    if ( !( getuid()==0 ) ){
        out << "you should use this command with sudo\n";
        return 0;
    }

    QStringList data;

    // check grub
    QFile grub( "/etc/default/grub" );
    if ( !grub.open( QFile::ReadWrite | QFile::Text ) )
        return -1;
    data = QString( grub.readAll() ).split( "\n" );

    QString cmdline;
    int position = data.indexOf( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT=.*") );
    cmdline = data.at( position );

    QStringList cmdlineList = cmdline.replace( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT="), "" )
                                     .replace( "\"", "" )
                                     .split( ' ' );
    cmdlineList.removeAll( "quiet" );



    bool bootfileFlag = false;
    for ( int i=0; i<cmdlineList.size(); i++ ){
        if ( cmdlineList.at( i ).contains( "bootsplash.bootfile" ) ){
            cmdlineList.replace( i, "bootsplash.bootfile=/bootsplash-themes/"+theme+"/bootsplash" );
            bootfileFlag = true;
            break;
        }
    }
    if ( !bootfileFlag ) cmdlineList.append( "bootsplash.bootfile=/bootsplash-themes/"+theme+"/bootsplash" );

    cmdline.clear();
    cmdline="GRUB_CMDLINE_LINUX_DEFAULT=\"";
    cmdline += cmdlineList.join(' ');
    cmdline += "\"";

    data.replace( position, cmdline );

    // write grub
    out << "Writing GRUB...\nbackup will be saved to /etc/default/grub.bak\n\n";
    out.flush();

    QFile::copy( "/etc/default/grub", "/etc/default/grub.bak" );

    grub.resize( 0 );
    for ( const QString& s : qAsConst(data) ) grub.write( ( s + '\n' ).toUtf8() );
    grub.close();

    out << "Updating GRUB...\n\n";
    out.flush();
    QProcess::execute( "grub-mkconfig", { "-o", "/boot/grub/grub.cfg" } );

    data.clear();

    // check initcpio

    QFile initcpio( "/etc/mkinitcpio.conf" );
    if ( !initcpio.open( QFile::ReadWrite | QFile::Text ) )
        return -1;
    data = QString( initcpio.readAll() ).split("\n");

    QString hooks;
    position = data.indexOf( QRegularExpression("^HOOKS=.*") );
    hooks = data.at( position );

    bool bracketsFlag = hooks.contains( QRegularExpression("^HOOKS=[(].*") );

    hooks.replace( QRegularExpression("^HOOKS="), "" );
    if ( bracketsFlag ) hooks.replace( '(', "" )
                             .replace( ')', "" );
    else                hooks.replace( '"', "" );    

    if ( !hooks.contains( "bootsplash-"+theme ) || hooks.count( QRegularExpression("bootsplash-") ) > 1 ){
        QStringList hooksList = hooks.split(' ');

        auto last_it = std::remove_if( hooksList.begin(),
                                       hooksList.end(),
                                       [](const QString& s){return s.contains("bootsplash-");} );
        hooksList.erase(last_it, hooksList.end());

        hooksList.append( "bootsplash-"+theme );

        hooks.clear();
        hooks  = bracketsFlag?"HOOKS=(":"HOOKS=\"";
        hooks += hooksList.join(' ');
        hooks += bracketsFlag?")":"\"";
        data.replace( position, hooks );

        // write initcpio
        out << "Writing initcpio...\nbackup will be saved to /etc/mkinitcpio.conf.bak\n\n";
        out.flush();

        QFile::copy( "/etc/mkinitcpio.conf", "/etc/mkinitcpio.conf.bak" );

        initcpio.resize( 0 );
        for ( const QString& s : qAsConst(data) ) initcpio.write( ( s + '\n' ).toUtf8() );
        initcpio.close();

        out << "\n\nupdating initcpio...\n\n";
        out.flush();
        QProcess::execute( "mkinitcpio", { "-P" } );

    }
    return 0;
}
int setLog(){
    QTextStream out( stdout );

    if ( !( getuid()==0 ) ){
        out << "you should use this command with sudo\n";
        return 0;
    }
    QFile grub( "/etc/default/grub" );
    if ( !grub.open( QFile::ReadWrite | QFile::Text ) )
        return -1;
    QStringList data = QString( grub.readAll() ).split( "\n" );

    QString cmdline;

    int position = data.indexOf( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT=.*") );
    cmdline = data.at( position );

    QStringList cmdlineList = cmdline.replace( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT="), "" )
                                     .replace( "\"", "" )
                                     .split( ' ' );

    if ( cmdlineList.indexOf("quiet")!=-1 ){
        cmdlineList.removeAll( "quiet" );
        out << "removing \"quiet\"\n";
    }

    if ( data.indexOf( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT=.*") ) != -1 ){
        cmdlineList.removeAt( cmdlineList.indexOf( QRegularExpression("bootsplash.bootfile=.*") ) );
        out << "removing bootsplash...\n";
    }

    cmdline.clear();
    cmdline="GRUB_CMDLINE_LINUX_DEFAULT=\"";
    cmdline+=cmdlineList.join(' ');
    cmdline+="\"";

    data.replace( position, cmdline );

    // write grub
    out << "Writing GRUB...\nbackup will be saved to /etc/default/grub.bak\n\n";
    out.flush();

    QFile::copy( "/etc/default/grub", "/etc/default/grub.bak" );
    grub.resize( 0 );
    for ( const QString& s : qAsConst(data) ) grub.write( ( s + '\n' ).toUtf8() );
    grub.close();

    data.clear();

    out << "updating GRUB...\n\n";
    out.flush();
    QProcess::execute( "grub-mkconfig", { "-o", "/boot/grub/grub.cfg" } );

    return initcpioClear();

}
int disable(){
    QTextStream out( stdout );

    if ( !( getuid()==0 ) ){
        out << "you should use this command with sudo\n";
        return 0;
    }
    QFile grub( "/etc/default/grub" );
    if ( !grub.open( QFile::ReadWrite | QFile::Text ) )
        return -1;
    QStringList data = QString( grub.readAll() ).split( "\n" );

    if ( data.indexOf( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT=.*") ) == -1 ){
        out << "bootsplash is already disabled\n";
        return 0;
    }
    out << "removing bootsplash...\n";

    QString cmdline;

    int position = data.indexOf( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT=.*") );
    cmdline = data.at( position );

    QStringList cmdlineList = cmdline.replace( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT="), "" )
                                     .replace( "\"", "" )
                                     .split( ' ' );

    if ( cmdlineList.indexOf(QRegExp("quiet($|\n)"))==-1 )  cmdlineList.append( "quiet" );

    cmdlineList.removeAt( cmdlineList.indexOf( QRegularExpression("bootsplash.bootfile=.*") ) );

    cmdline.clear();
    cmdline="GRUB_CMDLINE_LINUX_DEFAULT=\"";
    cmdline+=cmdlineList.join(' ');
    cmdline+="\"";

    data.replace( position, cmdline );

    // write grub
    out << "Writing GRUB...\nbackup will be saved to /etc/default/grub.bak\n\n";
    out.flush();

    QFile::copy( "/etc/default/grub", "/etc/default/grub.bak" );
    grub.resize( 0 );
    for ( const QString& s : qAsConst(data) ) grub.write( ( s + '\n' ).toUtf8() );
    grub.close();

    data.clear();

    out << "updating GRUB...\n\n";
    out.flush();
    QProcess::execute( "grub-mkconfig", { "-o", "/boot/grub/grub.cfg" } );

    return initcpioClear();
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

    QString ok = "[\033[42m\ ok \033[00m\]",
          warn = "[\033[43m\warn\033[00m\]";

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

int initcpioClear(){
    QTextStream out(stdout);

    QFile initcpio( "/etc/mkinitcpio.conf" );
    if ( !initcpio.open( QFile::ReadWrite | QFile::Text ) )
        return -1;
    QStringList data = QString( initcpio.readAll() ).split("\n");

    QString hooks;
    int position = data.indexOf( QRegularExpression("^HOOKS=.*") );
    hooks = data.at( position );

    if( hooks.contains( "bootsplash-" ) ){

        bool bracketsFlag = hooks.contains( QRegularExpression("^HOOKS=[(].*") );

        hooks.replace( QRegularExpression("^HOOKS="), "" );
        if ( bracketsFlag ) hooks.replace( '(', "" )
                                 .replace( ')', "" );
        else                hooks.replace( '"', "" );

        QStringList hooksList = hooks.split(' ');

        auto last_it = std::remove_if( hooksList.begin(),
                                       hooksList.end(),
                                       [](const QString& s){return s.contains("bootsplash-");} );
        hooksList.erase(last_it, hooksList.end());

        hooks.clear();
        hooks  = bracketsFlag?"HOOKS=(":"HOOKS=\"";
        hooks += hooksList.join(' ');
        hooks += bracketsFlag?")":"\"";
        data.replace( position, hooks );

        // write initcpio
        out << "Writing initcpio...\nbackup will be saved to /etc/mkinitcpio.conf.bak\n\n";
        out.flush();

        QFile::copy( "/etc/mkinitcpio.conf", "/etc/mkinitcpio.conf.bak" );

        initcpio.resize( 0 );
        for ( const QString& s : qAsConst(data) ) initcpio.write( ( s + '\n' ).toUtf8() );

        out << "\n\nupdating initcpio...\n\n";
        out.flush();

    }

    initcpio.close();
    return 0;
}
