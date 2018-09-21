/****************************************************************************
** Meta object code from reading C++ file 'ammeterData.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Ammeter/ammeterData.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ammeterData.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AmmeterData[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      63,   12,   12,   12, 0x05,
     113,   12,   12,   12, 0x05,
     160,   12,   12,   12, 0x05,
     205,   12,   12,   12, 0x05,
     224,   12,   12,   12, 0x05,
     249,  247,   12,   12, 0x05,
     284,  280,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
     367,   12,   12,   12, 0x0a,
     418,  397,   12,   12, 0x0a,
     458,   12,   12,   12, 0x0a,
     483,   12,   12,   12, 0x0a,
     499,   12,   12,   12, 0x0a,
     513,  247,   12,   12, 0x0a,
     547,   12,   12,   12, 0x0a,
     562,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AmmeterData[] = {
    "AmmeterData\0\0"
    "sig_readAmmeter_dlt645_07(QList<stAmmeterConfig>)\0"
    "sig_readAmmeter_dlt645_97(QList<stAmmeterConfig>)\0"
    "sig_readAmmeter_modbus(QList<stAmmeterConfig>)\0"
    "sig_readPowerMonitorAmmeter(stAmmeterConfig)\0"
    "sig_stopRead(bool)\0sig_sendBoardType(int)\0"
    ",\0sigToBus(InfoMap,InfoAddrType)\0,,,\0"
    "sig_readRemoteAmmeter_dlt645_07(unsigned char*,int,unsigned char*,stAm"
    "meterConfig)\0"
    "slotParseAmmeterData(InfoMap)\0"
    "readRemoteAmmeterMap\0"
    "slotsendToBusRemoteAmmeterData(InfoMap)\0"
    "slotParseReadResult(int)\0ProcStartWork()\0"
    "ProcTimeOut()\0slotFromBus(InfoMap,InfoAddrType)\0"
    "slotReadFail()\0slotReadSuccess()\0"
};

void AmmeterData::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AmmeterData *_t = static_cast<AmmeterData *>(_o);
        switch (_id) {
        case 0: _t->sig_readAmmeter_dlt645_07((*reinterpret_cast< QList<stAmmeterConfig>(*)>(_a[1]))); break;
        case 1: _t->sig_readAmmeter_dlt645_97((*reinterpret_cast< QList<stAmmeterConfig>(*)>(_a[1]))); break;
        case 2: _t->sig_readAmmeter_modbus((*reinterpret_cast< QList<stAmmeterConfig>(*)>(_a[1]))); break;
        case 3: _t->sig_readPowerMonitorAmmeter((*reinterpret_cast< stAmmeterConfig(*)>(_a[1]))); break;
        case 4: _t->sig_stopRead((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->sig_sendBoardType((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 7: _t->sig_readRemoteAmmeter_dlt645_07((*reinterpret_cast< unsigned char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< unsigned char*(*)>(_a[3])),(*reinterpret_cast< stAmmeterConfig(*)>(_a[4]))); break;
        case 8: _t->slotParseAmmeterData((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 9: _t->slotsendToBusRemoteAmmeterData((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 10: _t->slotParseReadResult((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->ProcStartWork(); break;
        case 12: _t->ProcTimeOut(); break;
        case 13: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 14: _t->slotReadFail(); break;
        case 15: _t->slotReadSuccess(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData AmmeterData::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AmmeterData::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_AmmeterData,
      qt_meta_data_AmmeterData, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AmmeterData::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AmmeterData::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AmmeterData::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AmmeterData))
        return static_cast<void*>(const_cast< AmmeterData*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int AmmeterData::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CModuleIO::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void AmmeterData::sig_readAmmeter_dlt645_07(QList<stAmmeterConfig> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AmmeterData::sig_readAmmeter_dlt645_97(QList<stAmmeterConfig> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AmmeterData::sig_readAmmeter_modbus(QList<stAmmeterConfig> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void AmmeterData::sig_readPowerMonitorAmmeter(stAmmeterConfig _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void AmmeterData::sig_stopRead(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void AmmeterData::sig_sendBoardType(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void AmmeterData::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void AmmeterData::sig_readRemoteAmmeter_dlt645_07(unsigned char * _t1, int _t2, unsigned char * _t3, stAmmeterConfig _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_END_MOC_NAMESPACE
