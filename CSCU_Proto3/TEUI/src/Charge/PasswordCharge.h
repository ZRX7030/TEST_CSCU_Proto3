#ifndef PASSWORDCHARGE_H
#define PASSWORDCHARGE_H

#include <QDialog>

namespace Ui {
class PasswordCharge;
}

class PasswordCharge : public QDialog
{
    Q_OBJECT
    
public:
    explicit PasswordCharge(QWidget *parent = 0);
    ~PasswordCharge();

    QString getInputPassword();
    
private:
    Ui::PasswordCharge *ui;

private slots:
     void timeoutchange();

     void on_pushButtonReturn_clicked();
     void on_pushButtonPasswordCharge_clicked();
};

#endif // PASSWORDCHARGE_H
