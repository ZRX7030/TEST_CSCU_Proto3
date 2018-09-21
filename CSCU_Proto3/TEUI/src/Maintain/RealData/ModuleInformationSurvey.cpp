#include "ModuleInformationSurvey.h"
#include "ui_ModuleInformationSurvey.h"

ModuleInformationSurvey::ModuleInformationSurvey(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModuleInformationSurvey)
{
    ui->setupUi(this);
}

ModuleInformationSurvey::~ModuleInformationSurvey()
{
    delete ui;
}
