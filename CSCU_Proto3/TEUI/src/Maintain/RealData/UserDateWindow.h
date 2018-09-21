#ifndef USERDATEWINDOW_H
#define USERDATEWINDOW_H

#include <QDialog>

namespace Ui {
class UserDateWindow;
}

class UserDateWindow : public QDialog
{
    Q_OBJECT

public:
    explicit UserDateWindow(QWidget *parent = 0);
    ~UserDateWindow();

private:
    Ui::UserDateWindow *ui;
};

#endif // USERDATEWINDOW_H
