#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString CurrentTheme;
    QStringList themes;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_listWidget_currentItemChanged();


    void on_ApplyButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
