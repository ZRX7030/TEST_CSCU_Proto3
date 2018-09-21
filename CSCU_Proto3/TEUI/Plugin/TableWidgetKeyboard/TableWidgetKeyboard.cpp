#include <QDebug>
#include "TableWidgetKeyboard.h"

TableWidgetKeyboard::TableWidgetKeyboard(QWidget *parent) :
	QTableWidget(parent)
{
	this->row = 0;
	this->column = 0;
	enableKeyboard();

    connect(this,SIGNAL(cellPressed(int , int) ), this, SLOT(tableCellEntered(int , int)));
}

void TableWidgetKeyboard::tableCellEntered(int row, int column)
{
	this->row = row;
	this->column = column;

	emit userClicked(this);
	qDebug() << "TableWidgetKeyboard::itemChanged" << row << column;
}

void TableWidgetKeyboard::callKeyboard(QWidget *p_widget)
{
    //Keyboard *p_input=new Keyboard(p_widget, 1);
    Keyboard *p_input=new Keyboard(p_widget);
    if(p_input->exec()<0)
	{
		disableKeyboard();
	}

	if(p_input->queryInputString() != QString(""))
		this->item(row,column)->setText(p_input->queryInputString());
	
	delete p_input;
}

//开启5
void TableWidgetKeyboard::enableKeyboard()
{
	connect(this,SIGNAL(userClicked(QWidget*)),this,SLOT(callKeyboard(QWidget*)));
}

void TableWidgetKeyboard::disableKeyboard()
{
	disconnect(this,SIGNAL(userClicked(QWidget*)),this,SLOT(callKeyboard(QWidget*)));
}

/**
 * @brief 用户结束输入
 * @param str
 */
void TableWidgetKeyboard::enterStringEnd(QString str)
{
	if(this->item(row, column))
	{
		this->item(row, column)->setText(str);
		qDebug() << "item text" << str;
	}
	else
		qDebug() << "item is null";
}
