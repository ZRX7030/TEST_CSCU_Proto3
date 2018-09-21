#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QDialog>
#include <QString>
#include <QtGui/QPalette>
//#include <QtGui/QLineEdit>
#include <QLineEdit>
#include <QtCore/QSignalMapper>
#include <QTimer>

namespace Ui {
class Keyboard;
}

class Keyboard : public QDialog
{
    Q_OBJECT
    
public:
	//explicit Keyboard(QWidget *parent = 0, int type = 0);
    explicit Keyboard(QWidget *parent = 0);
    ~Keyboard();

	QSignalMapper	*mapper;
	QSignalMapper	*mapper_num;
	QSignalMapper  *mapper_chr;

    QString queryInputString();

signals:
	void enterStringEnd(QString);

private slots:
	void insert(const QString&);
	void insert_num(const QString&);
	void insert_chr(const QString&);
	void del();
	void caps();
    //void close_key();
	void get_string();
	
    void on_inputkey_ok_button_clicked();
    void timeoutchange();

private:
    Ui::Keyboard *ui;
	
	enum
	{
        CAPITAL = 0,
		LOWERCASE
	};

    QTimer *timerQuery;
    void setcapitalletter();
    void setlowerletter();
	QString string;
    QString endString;
	QWidget *p_back_widget;
	char	cap_type;
    //int type;
};

#endif // KEYBOARD_H
