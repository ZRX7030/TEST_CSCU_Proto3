#include "ChargeMangeBase.h"
#include "StatusRemindWindow.h"
#include <Common.h>
#include <QVariant>
#include <QDebug>
#include <QList>
#include <QDebug>


ChargeMangeBase::ChargeMangeBase(QWidget *parent) : QWidget(parent)
{

}

void ChargeMangeBase::switchToBase(QString result, int type, QVariant varParam)
{

    StatusRemindWindow  *statusDialog= new StatusRemindWindow(this, 1, 5, QString(result));
    statusDialog->exec();
    delete statusDialog;
    emit sigBackToMain(PAGE_CHARGEMANAGE_BASE, PAGE_CHARGEMANAGE_MAIN, varParam);
}
