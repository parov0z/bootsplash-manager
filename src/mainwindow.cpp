#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QListWidget>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>


void refresh( QStringList& themes, QString& CurrentTheme ){
    // get list
    QDir d( "/usr/lib/firmware/bootsplash-themes/" );
    d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    if ( !d.exists() || d.count() == 0 ){
        QMessageBox m;
        m.setText("No installed themes detected");
        m.setIcon(QMessageBox::Information);
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
    while ( !cmdline.contains( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT=") ) )cmdline = grub.readLine();

    QStringList cmdlineList = cmdline.replace( QRegularExpression("^GRUB_CMDLINE_LINUX_DEFAULT="), "" )
                                     .replace( "\"", "" )
                                     .split( ' ' );
    bool bootfileFlag = false;

    if ( cmdlineList.indexOf(QRegExp("quiet($|\n)"))!=-1 ) CurrentTheme = "black screen";
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
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Manjaro Bootsplash Manager");
    new QListWidgetItem(QIcon(":/icons/black.svg"),
                        "black screen",
                        ui->listWidget);
    new QListWidgetItem(QIcon(":/icons/log.svg"),
                        "boot log",
                        ui->listWidget);

    refresh( themes, CurrentTheme );

    int position = 0;
    if ( CurrentTheme == "boot log" ) position = 1;
    for ( int i = 0; i<themes.size(); i++ ){
        if ( themes.at(i) == CurrentTheme ) position = i+2;
        new QListWidgetItem( QIcon(themes.at(i).contains("manjaro")?
                                     ":/icons/manjaro.svg"
                                    :":/icons/theme.svg"),
                             themes[i],
                             ui->listWidget );

    }

    ui->listWidget->setCurrentRow(position);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_listWidget_currentItemChanged()
{
    if ( ui->listWidget->currentItem()->text() == CurrentTheme )
        ui->ApplyButton->setEnabled(false);
    else ui->ApplyButton->setEnabled(true);
}
void MainWindow::on_ApplyButton_clicked()
{
    QProcess pkexec;
    pkexec.setProgram("pkexec");

    if ( ui->listWidget->currentRow() == 0 )
        pkexec.setArguments( QStringList() << "bootsplash-manager" << "-d" );
    else if ( ui->listWidget->currentRow() == 1 )
        pkexec.setArguments( QStringList() << "/usr/bin/bootsplash-manager" << "--set-log" );
    else{
        QString theme = ui->listWidget->currentItem()->text();
        pkexec.setArguments( QStringList() << "/usr/bin/bootsplash-manager" << "-s" << theme );
    }

    ui->ApplyButton->setEnabled(false);
    ui->centralwidget->setEnabled(false);   
    QApplication::setOverrideCursor(Qt::WaitCursor);
    pkexec.start();


    pkexec.waitForFinished(-1);
    QApplication::restoreOverrideCursor();
    ui->centralwidget->setEnabled(true);
    refresh( themes, CurrentTheme );
    int position = 0;
    if ( CurrentTheme == "boot log" ) position = 1;
    for ( int i = 0; i < themes.size(); i++ )
        if ( themes.at(i) == CurrentTheme ) position = i+2;
    ui->listWidget->setCurrentRow(position);
}
