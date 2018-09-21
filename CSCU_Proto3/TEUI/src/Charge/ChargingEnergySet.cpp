#include "ChargingEnergySet.h"
#include "ui_ChargingEnergySet.h"

ChargingEnergySet::ChargingEnergySet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChargingEnergySet)
{
    ui->setupUi(this);
}

ChargingEnergySet::~ChargingEnergySet()
{
    delete ui;
}
