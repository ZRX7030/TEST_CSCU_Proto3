#ifndef TERMINALINFORMATIONSURVEY_H
#define TERMINALINFORMATIONSURVEY_H

#include <QWidget>

namespace Ui {
class TerminalInformationSurvey;
}

class TerminalInformationSurvey : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalInformationSurvey(QWidget *parent = 0);
    ~TerminalInformationSurvey();

private:
    Ui::TerminalInformationSurvey *ui;
};

#endif // TERMINALINFORMATIONSURVEY_H
