#ifndef LANGUAGESELECT_H
#define LANGUAGESELECT_H

#include <QWidget>
#include "Common.h"
#include "InfoData.h"
#include "SwapBase.h"
#include "ProtocolBase.h"
#include "Bus.h"

namespace Ui {
class LanguageSelect;
}

class LanguageSelect : public QWidget, public SwapBase
{
    Q_OBJECT
    
public:
    explicit LanguageSelect(QWidget *parent, CBus * bus, ProtocolBase *protocol,void *param = 0);
    ~LanguageSelect();
    void receiveFromBus(InfoMap Map, InfoAddrType Type);

signals:
   void sigFromBus(InfoMap, InfoAddrType);

private slots:
    void slotBusToOwn(InfoMap Map, InfoAddrType Type);

    
    void on_buttonSave_clicked();

private:
    Ui::LanguageSelect *ui;
    CBus *bus;
    ProtocolBase *protocol;
    stSpecialFunc special_set;  //add by songqb 2018-1-15
    stTeuiParam *teuiParam;
};

#endif // LANGUAGESELECT_H
