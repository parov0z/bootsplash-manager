#include "installdialog.h"
#include "ui_installdialog.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <KAuth>
#include "pamac.h"
#include <gio/gio.h>

void InstallDialog::refresh(){
    QStringList themes, installed;

    ui->verticalLayout -> setEnabled( false );
    QApplication::setOverrideCursor( Qt::WaitCursor );

    ui->listWidget -> clear();

    PamacConfig *conf = pamac_config_new( "/etc/pamac.conf" );
    PamacDatabase *database = pamac_database_new( conf );

    // repos search
    GPtrArray *pkg = pamac_database_search_pkgs( database, "bootsplash-theme-" );

    if( (int)pkg->len !=0 ){
        for(int i = 0; i < (int)pkg->len; i++)
             themes.append( pamac_package_get_name( (PamacPackage*)g_ptr_array_index(pkg, i) ) );
    }
    // AUR search
    if( ui->checkBox->isChecked() ){
        GPtrArray *aur = pamac_database_search_aur_pkgs( database, "bootsplash-theme-" );

        if( (int)aur->len !=0 ){
            for(int i = 0; i < (int)aur->len; i++)
                 themes.append( pamac_aur_package_get_packagebase( (PamacAURPackage*)g_ptr_array_index(aur, i) ) );
        }
    }
    // installed search
    GPtrArray *inst = pamac_database_search_installed_pkgs( database, "bootsplash-theme-" );

    if( (int)pkg->len !=0 ){
        for(int i = 0; i < (int)inst->len; i++)
             installed.append( pamac_package_get_name( (PamacPackage*)g_ptr_array_index(inst, i) ) );
    }

    // don't show installed
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
    ui->pushButton->setEnabled( ui->listWidget->selectedItems().size() != 0 );
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

        QVariantMap args;
        args["toInstall"] = QVariant( toInstall );
        KAuth::Action installAction("dev.android7890.bootsplashmanager.install");
        installAction.setHelperId("dev.android7890.bootsplashmanager");
        installAction.setArguments( args );
        installAction.setTimeout( 300000 );
        KAuth::ExecuteJob *job = installAction.execute();

        QProgressDialog *d = new QProgressDialog("Installing "+ QString::number( toInstall.size() ) + " themes\nThis may take a few minutes",
                                                 nullptr, 0, 0, this);
        d->setWindowModality( Qt::WindowModal );
        d->setWindowFlags( Qt::Dialog | Qt::FramelessWindowHint );

        connect( job,
                 &KAuth::ExecuteJob::finished,
                 [=](){ d->close(); refresh(); }     );
        job->start();
        d->open();
    }
}



void InstallDialog::on_checkBox_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    refresh();
}
