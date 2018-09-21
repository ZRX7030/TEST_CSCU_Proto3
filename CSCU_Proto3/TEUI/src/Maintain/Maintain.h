#ifndef MAINTAIN_H
#define MAINTAIN_H

#include <QWidget>
#include <QMouseEvent>

#include "Bus.h"
#include "ProtocolBase.h"
#include "Common.h"

namespace Ui {
class Maintain;
}

class Maintain : public QWidget
{
    Q_OBJECT

public:
    explicit Maintain(QWidget *parent = 0, CBus *bus = 0, ProtocolBase *protocol = 0, void *param = 0);
    QRect getFillSize();
    ~Maintain();
	void switchToMaintain(void);
signals:
	void sigShowDefault();
private slots:
    void on_cscuSetButton_clicked();
    void on_chargeRecordButton_clicked();
    void on_BMSInformationButton_clicked();
    void on_UserLineSideInformationButton_clicked();
    void on_ActriphaseSetButton_clicked();
    void on_SpecialFeatureButton_clicked();
    void on_LoadConstraintSetButton_clicked();
    void on_TerminalInformationButton_clicked();
    void on_PeakChargingFeatureSetButton_clicked();
    void on_TimeSetButton_clicked();
    //void on_realInofButton_clicked();
    void on_FailureInformationButton_clicked();
    void on_PasswordSetButton_clicked();
    void on_versionButton_clicked();
    void on_SubStationEnInformationButton_clicked();
    void on_ModuleInformationButton_clicked();
    void on_operateRecordButton_clicked();
    void on_QRcodeCreate_clicked();
    void on_DCChargerDataButton_clicked();
    void on_DCChargerSetButton_clicked();

    void on_StartTypeSelect_clicked();

    void on_toolBox_currentChanged(int index);

    void on_ChargePassword_clicked();

    void on_LogoSetButton_clicked();

    void on_RealTimeFaultBtn_clicked();

    void on_CoupleGunSetButton_clicked();

    void on_languageSelectButton_clicked();

protected:
	void mousePressEvent(QMouseEvent *event);
private:
    Ui::Maintain *ui;
    QWidget *funcWidget;
	CBus *bus;
	ProtocolBase *protocol;

	stTeuiParam *teuiParam;
};

#endif // MAINTAIN_H
