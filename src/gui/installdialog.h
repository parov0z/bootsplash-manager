#ifndef INSTALLDIALOG_H
#define INSTALLDIALOG_H

#include <QDialog>

namespace Ui {
class InstallDialog;
}

class InstallDialog : public QDialog
{
    Q_OBJECT

public:
    void refresh();
    explicit InstallDialog(QWidget *parent = nullptr);
    ~InstallDialog();

private Q_SLOTS:
    void on_pushButton_clicked();

    void on_listWidget_itemSelectionChanged();

    void on_checkBox_stateChanged(int arg1);

private:
    Ui::InstallDialog *ui;
};

#endif // INSTALLDIALOG_H
