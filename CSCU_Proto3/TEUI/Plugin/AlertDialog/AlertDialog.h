#ifndef ALERTDIALOG_H
#define ALERTDIALOG_H

#include <QWidget>

namespace Ui {
class AlertDialog;
}

class AlertDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AlertDialog(QWidget *parent = 0);
    ~AlertDialog();

private:
    Ui::AlertDialog *ui;
};

#endif // ALERTDIALOG_H
