#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString CurrentTheme;
    QStringList themes;
    void refresh();
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private Q_SLOTS:
    void on_listWidget_currentItemChanged();

    void on_InstallButton_clicked();

    void on_RemoveButton_clicked();

    void on_previewButton_clicked();

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
