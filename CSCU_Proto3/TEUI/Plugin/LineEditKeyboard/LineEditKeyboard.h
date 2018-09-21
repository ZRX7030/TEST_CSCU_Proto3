#ifndef LINEEDITKEYBOARD_H
#define LINEEDITKEYBOARD_H

#include <QWidget>

//#include <QtGui/QLineEdit>
#include <QLineEdit>
#include <QWidget>
#include <QMouseEvent>
#include "Keyboard.h"

class LineEditKeyboard : public QLineEdit
{
    Q_OBJECT

public:
    explicit LineEditKeyboard(QWidget *parent = 0);
	~LineEditKeyboard();
	void enableKeyboard();
	void disableKeyboard();

private:
//    Keyboard *p_input;

private slots:
	void callKeyboard(QWidget *p_widget);

signals:
	void clicked(QWidget *p_widget);
    void lineEnterEnd(QString);

protected:
	void mousePressEvent(QMouseEvent *event);
};





#endif
