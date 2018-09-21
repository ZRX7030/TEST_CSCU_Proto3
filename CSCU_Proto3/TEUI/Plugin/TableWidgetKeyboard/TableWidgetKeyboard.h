#ifndef TABLEWIDGETKEYBOARD_H
#define TABLEWIDGETKEYBOARD_H

#include <QTableWidget>
#include <QString>

#include <Keyboard.h>

class TableWidgetKeyboard : public QTableWidget
{
    Q_OBJECT
    
public:
    TableWidgetKeyboard(QWidget *parent = 0);

private:

	int row, column;
	void enableKeyboard();
    void disableKeyboard();

signals:
    void userClicked(QWidget *p_widget);

private slots:
    void callKeyboard(QWidget *p_widget);
	void tableCellEntered(int , int); 
	void enterStringEnd(QString str);
};

#endif
