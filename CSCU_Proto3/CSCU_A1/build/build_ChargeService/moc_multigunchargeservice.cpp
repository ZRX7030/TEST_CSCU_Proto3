/****************************************************************************
** Meta object code from reading C++ file 'multigunchargeservice.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ChargeService/multigunchargeservice.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'multigunchargeservice.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MultigunChargeService[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      41,   23,   22,   22, 0x05,
      83,   74,   22,   22, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_MultigunChargeService[] = {
    "MultigunChargeService\0\0qInfoMap,InfoType\0"
    "sigToInBus(InfoMap,InfoAddrType)\0"
    "qInfoMap\0sigInVinApplyStartCharge(InfoMap)\0"
};

void MultigunChargeService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MultigunChargeService *_t = static_cast<MultigunChargeService *>(_o);
        switch (_id) {
        case 0: _t->sigToInBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->sigInVinApplyStartCharge((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MultigunChargeService::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MultigunChargeService::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MultigunChargeService,
      qt_meta_data_MultigunChargeService, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MultigunChargeService::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MultigunChargeService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MultigunChargeService::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MultigunChargeService))
        return static_cast<void*>(const_cast< MultigunChargeService*>(this));
    return QObject::qt_metacast(_clname);
}

int MultigunChargeService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void MultigunChargeService::sigToInBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MultigunChargeService::sigInVinApplyStartCharge(InfoMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
