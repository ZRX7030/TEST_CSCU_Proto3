#ifndef PASSWORDLOGIN_H
#define PASSWORDLOGIN_H

#include <QDialog>

namespace Ui {
class PasswordLogin;
}

class PasswordLogin : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordLogin(QWidget *parent = 0);
    ~PasswordLogin();

    QString getInputPassword();
private slots:
    void on_pushButtonLogin_clicked();

    void on_pushButtonReturn_clicked();
    void timeoutchange();

private:
    Ui::PasswordLogin *ui;
};

#endif // PASSWORDLOGIN_H
