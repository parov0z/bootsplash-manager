#include "installdialog.h"
#include "ui_installdialog.h"

#include <QProcess>
#include <QMessageBox>
#include <QProgressDialog>

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
                                       << ( ui->checkBox->isChecked()? "-a":"--no-aur")
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
    setWindowFlags( windowFlags() & (~Qt::WindowContextHelpButtonHint) );
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
    const QList<QListWidgetItem* > l = ui->listWidget->selectedItems();
    QStringList toInstall;

    for ( QListWidgetItem *i : l )
        toInstall.append( "bootsplash-theme-"+i->text() );

    QMessageBox b;
    b.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    b.setIcon( QMessageBox::Question );
    b.setText( "To install: " + QString::number( toInstall.size() )+ " themes \nContinue?"  );

    if( b.exec() == 0x00004000 ){

        QProcess *install = new QProcess;
        install->setEnvironment( QStringList("LANG=\"en_AU.UTF-8\"") );
        install->setProgram( "pamac" );

        install->setArguments( QStringList() << "install"
                                             << "--no-confirm"
                                             << "--no-upgrade"
                                             << toInstall     );




        QProgressDialog *d = new QProgressDialog("Installing "+ QString::number( toInstall.size() ) + " themes\nThis may take a few minutes",
                                                 nullptr, 0, 0, this);
        d->setWindowModality( Qt::WindowModal );
        d->setWindowFlags( Qt::Dialog | Qt::FramelessWindowHint );

        d->open();

        install->start();

        connect( install,
                 static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                 [=](){ d->close(); refresh(); }                     );

    }
}



void InstallDialog::on_checkBox_stateChanged(int arg1)
{
    refresh();
}
