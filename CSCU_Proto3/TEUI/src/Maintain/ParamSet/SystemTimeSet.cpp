#include "SystemTimeSet.h"
#include "ui_SystemTimeSet.h"
#include "StatusRemindWindow.h"
#include <QDateTime>

SystemTimeSet::SystemTimeSet(QWidget *parent, CBus *bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::SystemTimeSet)
{
    ui->setupUi(this);
}

SystemTimeSet::~SystemTimeSet()
{
    delete ui;
}

void SystemTimeSet::on_pushButtonSave_clicked()
{
    QDateTime dateTime = QDateTime::fromString(ui->lineEditSystemTime->text(), "yyyy-MM-dd hh:mm:ss");

    if(dateTime.isValid())
    {
        QString cmdStr="date -s \"";
        cmdStr.append(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
        cmdStr.append("\"; hwclock -w");
        system(cmdStr.toLatin1().data());
        StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 0, 5, QObject::tr("时间设置成功！"));
        statusDialog->exec();
        delete statusDialog;
    }
    else
    {
        StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QObject::tr("时间格式输入错误！"));
        statusDialog->exec();
        delete statusDialog;
    }
}
