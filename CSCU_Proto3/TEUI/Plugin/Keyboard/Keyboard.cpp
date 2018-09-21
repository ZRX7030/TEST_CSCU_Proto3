#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include "Keyboard.h"
#include "ui_Keyboard.h"

Keyboard::Keyboard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Keyboard)
{
    ui->setupUi(this);
    //this->type = type;

	//重定义大小、位置
	int desktopheight = QApplication::desktop()->height();
	int desktopwidth = QApplication::desktop()->width();
	this->resize(desktopwidth*2/3,desktopheight/2);
    //move(desktopwidth/3,desktopheight/2-30);
    //move(desktopwidth/5-32,desktopheight/2-6);     //下方居中
    move(desktopwidth/3,desktopheight/2-6);        //下方居右
    //move(desktopwidth/5-32,desktopheight/4);         //整个屏幕居中
	//setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
	QPalette pal = palette();
	pal.setColor(QPalette::Background, Qt::black);
	setPalette(pal);

	setFocusPolicy(Qt::NoFocus);
	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    timerQuery = new QTimer();
    timerQuery->setInterval(120000);
    timerQuery->setSingleShot(true);
    connect(timerQuery, SIGNAL(timeout()), this, SLOT(timeoutchange()));
    timerQuery->start();

//	this->setAttribute(Qt::WA_QuitOnClose, false);

	//this->setWindowOpacity(0.7);//设置透明1-全体透明
	//this->setAttribute(Qt::WA_TranslucentBackground, true);//设置透明2-窗体标题栏不透明,背景透明


	//赋初值
	cap_type = LOWERCASE;
	p_back_widget=parent;
	//	has_delete=false;
	//	inputKey_lineEdit->setText(((QLineEdit *)p_back_widget)->text());
	//	currentCursorPosition=inputKey_lineEdit->text().length();

	//按钮事件
	connect(ui->Inputkey_del_button, SIGNAL(clicked()), this, SLOT(del()));	
	connect(ui->Inputkey_Cap_button, SIGNAL(clicked()), this, SLOT(caps()));
    //connect(ui->Inputkey_close_button, SIGNAL(clicked()), this, SLOT(close_key()));
	connect(ui->inputkey_ok_button,SIGNAL(clicked()),this,SLOT(get_string()));

	//connect(inputKey_lineEdit,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(cursorPositionChanged(int,int)));
	//信号槽
	mapper = new QSignalMapper(this);
	mapper_num = new QSignalMapper(this);
	mapper_chr = new QSignalMapper(this);

	connect(mapper, SIGNAL(mapped(const QString&)), this, SLOT(insert(const QString&)));
	connect(mapper_num, SIGNAL(mapped(const QString&)), this, SLOT(insert_num(const QString&)));
	connect(mapper_chr, SIGNAL(mapped(const QString&)), this, SLOT(insert_chr(const QString&)));

	mapper->setMapping(ui->Inputkey_A_button, "A");
	connect(ui->Inputkey_A_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_B_button, "B");
	connect(ui->Inputkey_B_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_C_button,"C");
	connect(ui->Inputkey_C_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_D_button, "D");
	connect(ui->Inputkey_D_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_E_button, "E");
	connect(ui->Inputkey_E_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_F_button, "F");
	connect(ui->Inputkey_F_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_G_button, "G");
	connect(ui->Inputkey_G_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_H_button, "H");
	connect(ui->Inputkey_H_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_I_button, "I");
	connect(ui->Inputkey_I_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_J_button, "J");
	connect(ui->Inputkey_J_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_K_button, "K");
	connect(ui->Inputkey_K_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_L_button, "L");
	connect(ui->Inputkey_L_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_M_button, "M");
	connect(ui->Inputkey_M_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_N_button, "N");
	connect(ui->Inputkey_N_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_O_button, "O");
	connect(ui->Inputkey_O_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_P_button, "P");
	connect(ui->Inputkey_P_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_Q_button, "Q");
	connect(ui->Inputkey_Q_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_R_button, "R");
	connect(ui->Inputkey_R_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_S_button, "S");
	connect(ui->Inputkey_S_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_T_button, "T");
	connect(ui->Inputkey_T_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_U_button, "U");
	connect(ui->Inputkey_U_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_V_button, "V");
	connect(ui->Inputkey_V_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_W_button, "W");
	connect(ui->Inputkey_W_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_X_button, "X");
	connect(ui->Inputkey_X_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_Y_button, "Y");
	connect(ui->Inputkey_Y_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper->setMapping(ui->Inputkey_Z_button, "Z");
	connect(ui->Inputkey_Z_button, SIGNAL(clicked()), mapper, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_1_button, "1");
	connect(ui->Inputkey_1_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_2_button, "2");
	connect(ui->Inputkey_2_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_3_button, "3");
	connect(ui->Inputkey_3_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_4_button, "4");
	connect(ui->Inputkey_4_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_5_button, "5");
	connect(ui->Inputkey_5_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_6_button, "6");
	connect(ui->Inputkey_6_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_7_button, "7");
	connect(ui->Inputkey_7_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_8_button, "8");
	connect(ui->Inputkey_8_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_9_button, "9");
	connect(ui->Inputkey_9_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_num->setMapping(ui->Inputkey_0_button, "0");
	connect(ui->Inputkey_0_button, SIGNAL(clicked()), mapper_num, SLOT(map()));

	mapper_chr->setMapping(ui->InputD_add_button, "+");
	connect(ui->InputD_add_button, SIGNAL(clicked()), mapper_chr, SLOT(map()));

	mapper_chr->setMapping(ui->InputD_sub_button, "-");
	connect(ui->InputD_sub_button, SIGNAL(clicked()), mapper_chr, SLOT(map()));

	mapper_chr->setMapping(ui->InputD_mul_button, "*");
	connect(ui->InputD_mul_button, SIGNAL(clicked()), mapper_chr, SLOT(map()));

	mapper_chr->setMapping(ui->InputD_div_button, "/");
	connect(ui->InputD_div_button, SIGNAL(clicked()), mapper_chr, SLOT(map()));

	mapper_chr->setMapping(ui->InputD_point_button, ".");
	connect(ui->InputD_point_button, SIGNAL(clicked()), mapper_chr, SLOT(map()));

    mapper_chr->setMapping(ui->InputD_space_button, " ");
    connect(ui->InputD_space_button, SIGNAL(clicked()), mapper_chr, SLOT(map()));

    mapper_chr->setMapping(ui->InputD_colon_button, ":");
    connect(ui->InputD_colon_button, SIGNAL(clicked()), mapper_chr, SLOT(map()));
}

Keyboard::~Keyboard()
{
	if(mapper != NULL)
		delete mapper;
	if(mapper_num != NULL)
		delete mapper_num;
	if(mapper_chr!= NULL)
		delete mapper_chr;

	delete ui;
}
void Keyboard::setcapitalletter()
{
    ui->Inputkey_A_button->setText(QObject::tr("A"));
    ui->Inputkey_B_button->setText(QObject::tr("B"));
    ui->Inputkey_C_button->setText(QObject::tr("C"));
    ui->Inputkey_D_button->setText(QObject::tr("D"));
    ui->Inputkey_E_button->setText(QObject::tr("E"));
    ui->Inputkey_F_button->setText(QObject::tr("F"));
    ui->Inputkey_G_button->setText(QObject::tr("G"));
    ui->Inputkey_H_button->setText(QObject::tr("H"));
    ui->Inputkey_I_button->setText(QObject::tr("I"));
    ui->Inputkey_J_button->setText(QObject::tr("J"));
    ui->Inputkey_K_button->setText(QObject::tr("K"));
    ui->Inputkey_L_button->setText(QObject::tr("L"));
    ui->Inputkey_M_button->setText(QObject::tr("M"));
    ui->Inputkey_N_button->setText(QObject::tr("N"));
    ui->Inputkey_O_button->setText(QObject::tr("O"));
    ui->Inputkey_P_button->setText(QObject::tr("P"));
    ui->Inputkey_Q_button->setText(QObject::tr("Q"));
    ui->Inputkey_R_button->setText(QObject::tr("R"));
    ui->Inputkey_S_button->setText(QObject::tr("S"));
    ui->Inputkey_T_button->setText(QObject::tr("T"));
    ui->Inputkey_U_button->setText(QObject::tr("U"));
    ui->Inputkey_V_button->setText(QObject::tr("V"));
    ui->Inputkey_W_button->setText(QObject::tr("W"));
    ui->Inputkey_X_button->setText(QObject::tr("X"));
    ui->Inputkey_Y_button->setText(QObject::tr("Y"));
    ui->Inputkey_Z_button->setText(QObject::tr("Z"));
}
void Keyboard::setlowerletter()
{
    ui->Inputkey_A_button->setText(QObject::tr("a"));
    ui->Inputkey_B_button->setText(QObject::tr("b"));
    ui->Inputkey_C_button->setText(QObject::tr("c"));
    ui->Inputkey_D_button->setText(QObject::tr("d"));
    ui->Inputkey_E_button->setText(QObject::tr("e"));
    ui->Inputkey_F_button->setText(QObject::tr("f"));
    ui->Inputkey_G_button->setText(QObject::tr("g"));
    ui->Inputkey_H_button->setText(QObject::tr("h"));
    ui->Inputkey_I_button->setText(QObject::tr("i"));
    ui->Inputkey_J_button->setText(QObject::tr("j"));
    ui->Inputkey_K_button->setText(QObject::tr("k"));
    ui->Inputkey_L_button->setText(QObject::tr("l"));
    ui->Inputkey_M_button->setText(QObject::tr("m"));
    ui->Inputkey_N_button->setText(QObject::tr("n"));
    ui->Inputkey_O_button->setText(QObject::tr("o"));
    ui->Inputkey_P_button->setText(QObject::tr("p"));
    ui->Inputkey_Q_button->setText(QObject::tr("q"));
    ui->Inputkey_R_button->setText(QObject::tr("r"));
    ui->Inputkey_S_button->setText(QObject::tr("s"));
    ui->Inputkey_T_button->setText(QObject::tr("t"));
    ui->Inputkey_U_button->setText(QObject::tr("u"));
    ui->Inputkey_V_button->setText(QObject::tr("v"));
    ui->Inputkey_W_button->setText(QObject::tr("w"));
    ui->Inputkey_X_button->setText(QObject::tr("x"));
    ui->Inputkey_Y_button->setText(QObject::tr("y"));
    ui->Inputkey_Z_button->setText(QObject::tr("z"));
}
//删除按钮
void Keyboard::del()
{
	string = ui->inputKey_lineEdit->text();
	string = string.left(string.length()-1);
	//string=string.remove(currentCursorPosition-1,1);
	ui->inputKey_lineEdit->setText(string);
	//has_delete=true;
}

//大写按钮
void Keyboard::caps()
{
    QString	captial_title =QObject::tr("CAP");//QApplication::translate("Caps(大)", "Caps(大)",  0,  QApplication::UnicodeUTF8);
    QString	lowercase_title = QObject::tr("cap");//QApplication::translate("Caps(小)", "Caps(小)",  0,  QApplication::UnicodeUTF8);

	if(cap_type == CAPITAL)
	{
		cap_type = LOWERCASE;
        ui->Inputkey_Cap_button->setText(lowercase_title);
        setlowerletter();
	}
	else
	{
		cap_type = CAPITAL;
        ui->Inputkey_Cap_button->setText(captial_title);
        setcapitalletter();
	}
}

#if 0
//关闭按钮
void Keyboard::close_key()
{
	done(-1);
	close();
}
#endif

//插入字母
void Keyboard::insert(const QString& newstring)
{
	string = ui->inputKey_lineEdit->text();

	if(cap_type == CAPITAL)
		string += newstring;
	//string=string.insert(currentCursorPosition,newstring);
	else
		string += newstring.toLower();
	//string=string.insert(currentCursorPosition,newstring.toLower());

	ui->inputKey_lineEdit->setText(string);
}


//插入数字
void Keyboard::insert_num(const QString& newstring)
{
	string = ui->inputKey_lineEdit->text();
	string += newstring;
	//string=string.insert(currentCursorPosition,newstring);
	ui->inputKey_lineEdit->setText(string);
}

//插入特殊字符
void Keyboard::insert_chr(const QString& newstring)
{
	string = ui->inputKey_lineEdit->text();
	string += newstring;
	//string=string.insert(currentCursorPosition,newstring);
	ui->inputKey_lineEdit->setText(string);
}
//获取最后的输入字符
void Keyboard::get_string()
{
    //if(type == 0)
     //   ((QLineEdit *)p_back_widget)->setText(ui->inputKey_lineEdit->text());
    this->endString = ui->inputKey_lineEdit->text();
    emit enterStringEnd(this->endString);
}
QString Keyboard::queryInputString()
{
    return endString;
}
//光标位置改变
//void Keyboard::cursorPositionChanged(int old_position, int new_position)
//{
//	if(has_delete)
//	{
//		currentCursorPosition=old_position-1;
//		has_delete=false;
//	}
//	else
//	{
//		currentCursorPosition=new_position;
//		qDebug("current position=%d",new_position);
//	}
//}

void Keyboard::on_inputkey_ok_button_clicked()
{
    done(1);
    close();
}
void Keyboard::timeoutchange()
{
    done(1);
    close();
}
