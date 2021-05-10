#include "root-actions.h"

#include <stdio.h>
#include <QCommandLineParser>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <unistd.h>
#include <QProcess>

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
int rootActions::setTheme( const QString& theme ){
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
int rootActions::setLog(){
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
int rootActions::disable(){
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


