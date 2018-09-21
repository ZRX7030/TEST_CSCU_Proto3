#include "TimeLimit.h"
#include "ui_TimeLimit.h"
#include <QtCore>
#include <QTimer>

TimeLimit::TimeLimit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeLimit)
{
    ui->setupUi(this);
    secondCount = 0;
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerDownDate()));
}

void TimeLimit::startDownCout(int seconds)
{
    timer->start();
    secondCount = seconds;
    ui->labelTimer->setText(QString::number(secondCount, 10));
}
void TimeLimit::stopDownCout()
{
    timer->stop();
    secondCount = 0;
    ui->labelTimer->setText(QString(""));
}

void TimeLimit::timerDownDate()
{
    if(0 == secondCount)
    {
        timer->stop();
        ui->labelTimer->setText(QString(""));
        emit timeout();
    }
    else
    {
        secondCount--;
        ui->labelTimer->setText(QString::number(secondCount, 10));
    }

}

TimeLimit::~TimeLimit()
{
    delete timer;
    delete ui;
}
