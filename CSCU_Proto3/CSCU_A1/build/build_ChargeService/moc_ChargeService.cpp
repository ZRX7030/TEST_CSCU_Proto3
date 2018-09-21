/****************************************************************************
** Meta object code from reading C++ file 'ChargeService.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ChargeService/ChargeService.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChargeService.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ChargeService[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   15,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      64,   15,   14,   14, 0x0a,
      98,   14,   14,   14, 0x0a,
     116,   14,   14,   14, 0x08,
     145,   14,   14,   14, 0x08,
     180,   14,   14,   14, 0x08,
     221,   15,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ChargeService[] = {
    "ChargeService\0\0qInfoMap,InfoType\0"
    "sigToBus(InfoMap,InfoAddrType)\0"
    "slotFromBus(InfoMap,InfoAddrType)\0"
    "slotThreadStart()\0slot_ProcChargeStepTimeOut()\0"
    "slot_ProcSaveChargingDataTimeOut()\0"
    "slot_ProcActiveUpateLogicStatusTimeOut()\0"
    "slot_RecvFromCardCharge(InfoMap,InfoAddrType)\0"
};

void ChargeService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ChargeService *_t = static_cast<ChargeService *>(_o);
        switch (_id) {
        case 0: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 2: _t->slotThreadStart(); break;
        case 3: _t->slot_ProcChargeStepTimeOut(); break;
        case 4: _t->slot_ProcSaveChargingDataTimeOut(); break;
        case 5: _t->slot_ProcActiveUpateLogicStatusTimeOut(); break;
        case 6: _t->slot_RecvFromCardCharge((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ChargeService::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ChargeService::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_ChargeService,
      qt_meta_data_ChargeService, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ChargeService::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ChargeService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ChargeService::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ChargeService))
        return static_cast<void*>(const_cast< ChargeService*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int ChargeService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CModuleIO::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void ChargeService::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
