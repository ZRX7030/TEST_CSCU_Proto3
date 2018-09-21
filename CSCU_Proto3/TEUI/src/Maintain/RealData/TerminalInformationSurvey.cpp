#include "TerminalInformationSurvey.h"
#include "ui_TerminalInformationSurvey.h"

TerminalInformationSurvey::TerminalInformationSurvey(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TerminalInformationSurvey)
{
    ui->setupUi(this);
}

TerminalInformationSurvey::~TerminalInformationSurvey()
{
    delete ui;
}
