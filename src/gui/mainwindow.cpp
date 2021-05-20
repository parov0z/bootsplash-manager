#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "installdialog.h"

#include <QListWidget>
#include <QDir>
#include <QMessageBox>
#include <QProgressDialog>
#include <KAuth>
#include "pamac.h"
#include <gio/gio.h>

extern "C" int bootsplashViewer( const char* arg );


void MainWindow::refresh(){
    // get list
    QDir d( "/usr/lib/firmware/bootsplash-themes/" );
    d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    if ( !d.exists() || d.count() == 0 ){
        QMessageBox m;
        m.setText( tr("No installed themes detected") );
        m.setIcon( QMessageBox::Information );
        m.exec();
    }
    else {
        themes.clear();
        themes = d.entryList();
    }

    //get current
    QFile grub( "/etc/default/grub" );
    grub.open( QFile::ReadOnly | QFile::Text );
    QString cmdline;
    while ( !cmdline.contains( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT=") ) )
        cmdline = grub.readLine();

    QStringList cmdlineList = cmdline.replace( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT="), "" )
                                     .replace( "\"", "" )
                                     .split( ' ' );
    bool bootfileFlag = false;

    if ( cmdlineList.indexOf( QRegExp("quiet($|\n)") ) != -1 ) CurrentTheme = "black screen";
    else {
        for ( const QString& opt : qAsConst(cmdlineList) ){
            if ( opt.contains( "bootsplash.bootfile=" ) ){
                bootfileFlag = true;
                CurrentTheme=opt;
            }
        }
        CurrentTheme.replace( "bootsplash.bootfile=/bootsplash-themes/", "" )
                    .replace( "/bootsplash", "" )
                    .replace( "\n", "" );

    if ( !bootfileFlag ) CurrentTheme = "boot log";
    }
    ui->listWidget->clear();

    new QListWidgetItem( QIcon(":/icons/black.svg"),
                         "black screen",
                         ui->listWidget            );
    new QListWidgetItem( QIcon(":/icons/log.svg"),
                         "boot log",
                         ui->listWidget            );

    int position = 0;
    if ( CurrentTheme == "boot log" ) position = 1;
    for ( int i = 0; i<themes.size(); i++ ){
        if ( themes.at( i ) == CurrentTheme )
            position = i+2;
        new QListWidgetItem( QIcon( themes.at( i ).contains("manjaro")?
                                                ":/icons/manjaro.svg"
                                               :":/icons/theme.svg"  ),
                             themes[i],
                             ui->listWidget                             );

    }

    ui->listWidget->setCurrentRow( position );

}



MainWindow::MainWindow( QWidget *parent )
    : QMainWindow( parent )
    , ui( new Ui::MainWindow )
{ 
    ui->setupUi( this );
    refresh();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_listWidget_currentItemChanged()
{
    if ( ui->listWidget->currentRow() != -1 ){

        if ( ui->listWidget->currentRow() == 0
          || ui->listWidget->currentRow() == 1 )
        {
            ui->RemoveButton  -> setEnabled( false );
            ui->previewButton -> setEnabled( false );
        } else {
            ui->RemoveButton  -> setEnabled(  true );
            ui->previewButton -> setEnabled(  true );
        }

        if ( ui->listWidget->currentItem()->text() == CurrentTheme ){
            ui->buttonBox     -> setEnabled( false );
            ui->RemoveButton  -> setEnabled( false );
        }
        else
            ui->buttonBox     -> setEnabled(  true );

    }
}
void MainWindow::on_buttonBox_clicked(QAbstractButton *button)
{
    Q_UNUSED(button);

    QVariantMap args;
    args["theme"] = QVariant( ui->listWidget->currentItem()->text() );
    KAuth::Action changeAction("dev.android7890.bootsplashmanager.changetheme");
    changeAction.setHelperId("dev.android7890.bootsplashmanager");
    changeAction.setArguments( args );
    changeAction.setTimeout( 180000 );
    KAuth::ExecuteJob *job = changeAction.execute();

    QProgressDialog *d = new QProgressDialog( tr("Please, wait..."), nullptr, 0, 0, this );
    d->setWindowModality( Qt::WindowModal );
    d->setWindowFlags( Qt::Dialog | Qt::FramelessWindowHint );

    connect( job,
             &KAuth::ExecuteJob::finished,
             this,
             [=](){ d->close(); refresh(); }     );
    job->start();
    d->open();
}

void MainWindow::on_InstallButton_clicked()
{
    InstallDialog d;
    d.exec();
    refresh();
}

void MainWindow::on_RemoveButton_clicked()
{
    ui->centralwidget -> setEnabled( false );
    QApplication::setOverrideCursor( Qt::WaitCursor );


    PamacConfig *conf = pamac_config_new( "/etc/pamac.conf" );
    PamacDatabase *database = pamac_database_new( conf );

    QByteArray file = QString("/usr/lib/firmware/bootsplash-themes/" + ui->listWidget->currentItem()->text() + "/bootsplash").toUtf8();

    char* f[] = { file.data() };

    GHashTable *pkg = pamac_database_search_files( database, f, 1 );

    QString result;

    GHashTableIter iter;
    gpointer pkgname, filelist;

    g_hash_table_iter_init (&iter, pkg);
    while (g_hash_table_iter_next (&iter, &pkgname, &filelist))
    {
        if( (int)( pamac_database_search_installed_pkgs( database, (char*)pkgname )->len ) != 0 )
            result = (char*)pkgname;
    }


    QApplication::restoreOverrideCursor();
    ui->centralwidget -> setEnabled(  true );

    QMessageBox b;
    b.setText( tr("Do you really want to remove\n") + result + "?" );
    b.setIcon( QMessageBox::Question );
    b.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

    if ( b.exec() == 0x00004000 ){

        QVariantMap args;
        args["theme"] = QVariant( result );
        KAuth::Action removeAction("dev.android7890.bootsplashmanager.remove");
        removeAction.setHelperId("dev.android7890.bootsplashmanager");
        removeAction.setArguments( args );
        removeAction.setTimeout( 180000 );
        KAuth::ExecuteJob *job = removeAction.execute();

        QProgressDialog *d = new QProgressDialog( tr("Please, wait..."), nullptr, 0, 0, this );
        d->setWindowModality( Qt::WindowModal );
        d->setWindowFlags( Qt::Dialog | Qt::FramelessWindowHint );

        connect( job,
                 &KAuth::ExecuteJob::finished,
                 this,
                 [=](){ d->close(); refresh(); }     );

        job->start();
        d->open();


    }

    refresh();
}

void MainWindow::on_previewButton_clicked()
{
    QString path = "/usr/lib/firmware/bootsplash-themes/" +
                    ui->listWidget->currentItem()->text() +
                    "/bootsplash";
    bootsplashViewer( path.toUtf8() );
}
