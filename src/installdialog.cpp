#include "installdialog.h"
#include "ui_installdialog.h"

#include <QProcess>
#include <QProgressBar>

void InstallDialog::refresh(){
    QStringList themes, installed;

    ui->verticalLayout -> setEnabled( false );
    QApplication::setOverrideCursor( Qt::WaitCursor );

    ui->listWidget -> clear();

    QProcess search;
    search.setEnvironment( QStringList("LANG=\"en_AU.UTF-8\"") );
    search.setProgram( "pamac" );
    // search all
    search.setArguments( QStringList() << "search"
                                       << "-a"
                                       << "-q"
                                       << "bootsplash-theme-" );
    search.start();
    search.waitForFinished( -1 );

    themes = QString( search.readAll() ).split( '\n' );
    // search installed
    search.setArguments( QStringList() << "search"
                                       << "-a"
                                       << "-q"
                                       << "-i"
                                       << "bootsplash-theme-" );
    search.start();
    search.waitForFinished( -1 );

    installed = QString( search.readAll() ).split( '\n' );
    // remove installed
    for ( const QString& s : qAsConst( installed ) )
        themes.removeAll( s );

    for ( const QString& s : qAsConst( themes ) ){
        new QListWidgetItem( QIcon(s.contains("manjaro")?
                                     ":/icons/manjaro.svg"
                                    :":/icons/theme.svg" ),
                             QString( s ).remove( 0, 17 ),
                             ui->listWidget                );

    }

    QApplication::restoreOverrideCursor();
    ui->verticalLayout -> setEnabled(  true );
    ui->pushButton     -> setEnabled( false );
}

InstallDialog::InstallDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::InstallDialog )
{
    ui->setupUi( this );
    refresh();
}

InstallDialog::~InstallDialog()
{
    delete ui;
}

void InstallDialog::on_listWidget_itemSelectionChanged()
{
    if ( ui->listWidget->selectedItems().size() != 0 )
        ui->pushButton -> setEnabled(  true );
    else
        ui->pushButton -> setEnabled( false );
}

void InstallDialog::on_pushButton_clicked()
{
    ui->verticalLayout -> setEnabled( false );
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QStringList toInstall;

    const QList<QListWidgetItem* > l = ui->listWidget->selectedItems();
    for ( QListWidgetItem *i : l )
        toInstall.append( "bootsplash-theme-"+i->text() );

    QProcess install;
    install.setEnvironment( QStringList("LANG=\"en_AU.UTF-8\"") );
    install.setProgram( "pamac" );

    install.setArguments( QStringList() << "install"
                                        << "--no-confirm"
                                        << "--no-upgrade"
                                        << toInstall     );


    install.start();
    install.waitForFinished( -1 );

    refresh();

    QApplication::restoreOverrideCursor();
    ui->verticalLayout -> setEnabled(  true );
}


