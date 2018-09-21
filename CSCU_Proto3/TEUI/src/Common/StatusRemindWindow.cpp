#include "StatusRemindWindow.h"
#include "ui_StatusRemindWindow.h"
#include "TimeLimit.h"

#include <QDebug>

StatusRemindWindow::StatusRemindWindow(QWidget *parent, int status,int dispSec, QString dispText) :
    QDialog(parent),
    ui(new Ui::StatusRemindWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);

    ui->stackedWidgetStatusRemind->setCurrentIndex(status);
    ui->labelStatus->setText(dispText);
    if(dispSec > 0)			// -1 一直显示
        ui->timeLimit->startDownCout(dispSec);
    else
        ui->buttonBack->hide();			//屏蔽返回按钮
	QObject::connect(ui->timeLimit, SIGNAL(timeout()), this, SLOT(timeoutchange()));
}

StatusRemindWindow::~StatusRemindWindow()
{
	delete ui;
}

void StatusRemindWindow::slotCloseWidget()
{
    done(1);
    close();
}

/**
 *超时时间到了
 */
void StatusRemindWindow::timeoutchange()
{
	ui->timeLimit->stopDownCout();
    done(1);
    close();
}
void StatusRemindWindow::on_buttonBack_clicked()
{
    ui->timeLimit->stopDownCout();
    done(1);
    close();
}
