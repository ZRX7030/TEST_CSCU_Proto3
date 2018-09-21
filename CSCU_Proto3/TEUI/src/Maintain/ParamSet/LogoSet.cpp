#include "LogoSet.h"
#include "ui_LogoSet.h"
#include "StatusRemindWindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QTextStream>

LogoSet::LogoSet(QWidget *parent, CBus * bus, ProtocolBase *protocol) :
    QWidget(parent),
    ui(new Ui::LogoSet)
{
    ui->setupUi(this);

    Readfile();
}

LogoSet::~LogoSet()
{
    delete ui;
}
void LogoSet::Readfile()   //by songqb 2017-4-20
{
    int currentnum = 0;
    QFile file("/mnt/nandflash/etc/teuilogo.conf");
    if(!file.open(QFile::ReadOnly | QFile::Text)) //只读方式打开文件
    {
        qDebug()<<"Can't open the file!";
        //return;
    }
    QTextStream in(&file);                //读文件内容
    QString str = in.readAll();
    bool ok;
    currentnum = str.toInt(&ok,10);  //转int型
    ui->comboBoxSinggleGroup->setCurrentIndex(currentnum-1);
    file.close();
}
void LogoSet::on_buttonSave_clicked()    //by songqb 2017-4-20
{
    int number = 0;
    if(ui->comboBoxSinggleGroup->currentIndex() == 0)
        number = 1;
    else
        number = 2;
    QFile file("/mnt/nandflash/etc/teuilogo.conf");
    StatusRemindWindow  *statusDialog = NULL;
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        statusDialog = new StatusRemindWindow(this, 1, 5, QObject::tr("系统标题设置失败!"));
    }
    else
    {
        QTextStream out(&file);        //写文件内容
        out << number << "\n";
        statusDialog = new StatusRemindWindow(this, 0, 5, QObject::tr("系统标题设置成功,正在切换!"));
    }

    file.close();
    statusDialog->exec();
    delete statusDialog;
	QCoreApplication::exit();
}
