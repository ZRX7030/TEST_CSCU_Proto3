#ifndef CHARGINGENERGYSET_H
#define CHARGINGENERGYSET_H

#include <QWidget>

namespace Ui {
class ChargingEnergySet;
}

class ChargingEnergySet : public QWidget
{
    Q_OBJECT

public:
    explicit ChargingEnergySet(QWidget *parent = 0);
    ~ChargingEnergySet();

private:
    Ui::ChargingEnergySet *ui;
	unsigned char canAddr;
};

#endif // CHARGINGENERGYSET_H
