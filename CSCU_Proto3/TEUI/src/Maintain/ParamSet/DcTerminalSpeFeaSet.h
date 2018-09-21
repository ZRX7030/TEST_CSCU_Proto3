#ifndef DCTERMINALSPEFEASET_H
#define DCTERMINALSPEFEASET_H

#include <QWidget>

namespace Ui {
class DcTerminalSpeFeaSet;
}

class DcTerminalSpeFeaSet : public QWidget
{
    Q_OBJECT

public:
    explicit DcTerminalSpeFeaSet(QWidget *parent = 0);
    ~DcTerminalSpeFeaSet();

private:
    Ui::DcTerminalSpeFeaSet *ui;
};

#endif // DCTERMINALSPEFEASET_H
