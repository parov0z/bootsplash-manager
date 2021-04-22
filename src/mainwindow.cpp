#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "installdialog.h"

#include <QListWidget>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>

extern "C" int bootsplashViewer( const char* arg );


void MainWindow::refresh(){
    // get list
    QDir d( "/usr/lib/firmware/bootsplash-themes/" );
    d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    if ( !d.exists() || d.count() == 0 ){
        QMessageBox m;
        m.setText( "No installed themes detected" );
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

                             // Don't know why, but when it's last item of QStringList, it has \n at the end
                             // Don't remember having this problem in cli (bootsplash-manager.cpp), I tested it multiple times, but added same fix it there too as a precaution
                             //               \/
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
            ui->ApplyButton   -> setEnabled( false );
            ui->RemoveButton  -> setEnabled( false );
        }
        else
            ui->ApplyButton   -> setEnabled(  true );

    }
}
void MainWindow::on_ApplyButton_clicked()
{
    QProcess *pkexec = new QProcess;
    pkexec->setProgram( "pkexec" );

    if ( ui->listWidget->currentRow() == 0 )
        pkexec->setArguments( QStringList() << "bootsplash-manager"
                                           << "-d"                  );

    else if ( ui->listWidget->currentRow() == 1 )
        pkexec->setArguments( QStringList() << "bootsplash-manager"
                                           << "--set-log"           );

    else{
        QString theme = ui->listWidget->currentItem()->text();
        pkexec->setArguments( QStringList() << "bootsplash-manager"
                                           << "-s"
                                           << theme                 );
    }

    QProgressDialog *d = new QProgressDialog("Please, wait...", nullptr, 0, 0, this);
    d->setWindowModality(Qt::WindowModal);
    d->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    d->open();

    pkexec->start();

    connect( pkexec,
             static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
             [=](){ d->close(); refresh(); }                     );
}

void MainWindow::on_InstallButton_clicked()
{
    InstallDialog d;
    d.exec();
    refresh();
}

void MainWindow::on_RemoveButton_clicked()
{
    QProcess pkgname;
    pkgname.setProgram( "pamac" );
    pkgname.setEnvironment( QStringList("LANG=\"en_AU.UTF-8\"") );

    QString theme = ui->listWidget->currentItem()->text();
    pkgname.setArguments( QStringList() << "search"
                                        << "--installed"
                                        << "--files"
                                        << "--aur"
                                        << "/usr/lib/firmware/bootsplash-themes/"+theme+"/" );

    ui->centralwidget -> setEnabled( false );
    QApplication::setOverrideCursor( Qt::WaitCursor );
    pkgname.start();

    pkgname.waitForFinished( -1 );
    QApplication::restoreOverrideCursor();
    ui->centralwidget -> setEnabled(  true );

    QString result = QString( pkgname.readAll() );
    /*
      possible pamac output
        > /usr/lib/firmware/bootsplash-themes/manjaro/bootsplash is owned by bootsplash-theme-manjaro
        > No package owns /usr/lib/firmware/bootsplash-themes/what
    */
    if ( result.contains( "No package owns" ) ){
        QMessageBox b;
        b.setWindowTitle( "Warning" );
        b.setIcon( QMessageBox::Critical );
        b.setText( "Can't find that package\n"
                   "Are you sure that theme was installed via package manager?" );
        b.exec();
    }
    else{
        result.remove( QRegularExpression("^.* is owned by ") )
              .remove( "\n" );

        QMessageBox b;
        b.setText( "Do you really want to remove "+result+"?" );
        b.setIcon( QMessageBox::Question );
        b.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

        if ( b.exec() == 0x00004000 ){

            QProcess *remove = new QProcess;
            remove->setEnvironment( QStringList("LANG=\"en_AU.UTF-8\"") );
            remove->setProgram( "pamac" );
            remove->setArguments( QStringList() << "remove"
                                                << "--no-confirm"
                                                << result        );

            QProgressDialog *d = new QProgressDialog("Please, wait...", nullptr, 0, 0, this);
            d->setWindowModality(Qt::WindowModal);
            d->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

            d->open();

            remove->start();

            connect( remove,
                     static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [=](){ d->close(); refresh();}                     );
        }
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
