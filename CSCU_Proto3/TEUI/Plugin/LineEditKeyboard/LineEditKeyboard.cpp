#include <QDebug>
#include "LineEditKeyboard.h"

LineEditKeyboard::LineEditKeyboard(QWidget *parent) :
	QLineEdit(parent)
{
	//p_input = NULL;
	enableKeyboard();
}

LineEditKeyboard::~LineEditKeyboard()
{
    //qDebug() << "keyboard exit................";
//	if(p_input)
//		delete p_input;
}

void LineEditKeyboard::mousePressEvent(QMouseEvent * event)
{
	emit clicked(this);
	QLineEdit::mousePressEvent(event);
}
void LineEditKeyboard::callKeyboard(QWidget *p_widget)
{
    //Keyboard *p_input = new Keyboard(p_widget, 0);
    Keyboard *p_input = new Keyboard(p_widget);

	if(p_input->exec()<0)
	{
		disableKeyboard();
	}
    /*触发输入结束信号*/
    QString enterStr = p_input->queryInputString();
	
	//qDebug() << "enter value is .........." << enterStr;
	if(enterStr != QString(""))
	{
        this->setText(enterStr);
		emit lineEnterEnd(enterStr);
		//qDebug() << "enter value is 2 .........." << enterStr;
	}

	delete p_input;
}
//开启
void LineEditKeyboard::enableKeyboard()
{
	connect(this,SIGNAL(clicked(QWidget*)),this,SLOT(callKeyboard(QWidget*)));
}

void LineEditKeyboard::disableKeyboard()
{
	disconnect(this,SIGNAL(clicked(QWidget*)),this,SLOT(callKeyboard(QWidget*)));
}


