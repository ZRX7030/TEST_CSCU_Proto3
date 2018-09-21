#ifndef MODULEINFORMATIONSURVEY_H
#define MODULEINFORMATIONSURVEY_H

#include <QDialog>

namespace Ui {
class ModuleInformationSurvey;
}

class ModuleInformationSurvey : public QDialog
{
    Q_OBJECT

public:
    explicit ModuleInformationSurvey(QWidget *parent = 0);
    ~ModuleInformationSurvey();

private:
    Ui::ModuleInformationSurvey *ui;
};

#endif // MODULEINFORMATIONSURVEY_H
