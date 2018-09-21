#ifndef STATUSREMINDWINDOW_H
#define STATUSREMINDWINDOW_H

#include <QDialog>
#include <QTimer>
#include <QString>

namespace Ui {
class StatusRemindWindow;
}

class StatusRemindWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StatusRemindWindow(QWidget *parent = 0, int status =0, int dispSec=2, QString dispText="");
    ~StatusRemindWindow();

private slots:
    //void slotDispTimer();
    void timeoutchange();
    void on_buttonBack_clicked();

public slots:
	void slotCloseWidget(void);

private:
    Ui::StatusRemindWindow *ui;
    QTimer dispTimer;

};

#endif // STATUSREMINDWINDOW_H
