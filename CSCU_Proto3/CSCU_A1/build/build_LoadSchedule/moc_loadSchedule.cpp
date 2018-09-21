/****************************************************************************
** Meta object code from reading C++ file 'loadSchedule.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../LoadSchedule/loadSchedule.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'loadSchedule.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LoadSchedule[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   14,   13,   13, 0x05,
      47,   13,   13,   13, 0x05,
      93,   13,   13,   13, 0x05,
     138,   13,   13,   13, 0x05,
     177,   13,   13,   13, 0x05,
     199,   13,   13,   13, 0x05,
     223,   13,   13,   13, 0x05,
     258,  249,   13,   13, 0x05,
     286,   13,   13,   13, 0x05,
     317,   14,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
     371,   14,   13,   13, 0x0a,
     405,   13,   13,   13, 0x0a,
     421,   14,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_LoadSchedule[] = {
    "LoadSchedule\0\0,\0sigToBus(InfoMap,InfoAddrType)\0"
    "sig_paraChange_powerLimit(stPowerLimitConfig)\0"
    "sig_paraChange_smartCharge(stAllTPFVConfig*)\0"
    "sig_paraChange_smartChargeSwitch(bool)\0"
    "sig_readAmmeterFail()\0sig_readAmmeterSucess()\0"
    "sig_setCCUResult(InfoMap)\0qInfoMap\0"
    "sig_sendTermSignal(InfoMap)\0"
    "sig_StartCharge(unsigned char)\0"
    "sig_chargeFromEnergyPlan(unsigned char,unsigned char)\0"
    "slotFromBus(InfoMap,InfoAddrType)\0"
    "ProcStartWork()\0"
    "slot_setPowerLimit(InfoMap,InfoAddrType)\0"
};

void LoadSchedule::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LoadSchedule *_t = static_cast<LoadSchedule *>(_o);
        switch (_id) {
        case 0: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->sig_paraChange_powerLimit((*reinterpret_cast< stPowerLimitConfig(*)>(_a[1]))); break;
        case 2: _t->sig_paraChange_smartCharge((*reinterpret_cast< stAllTPFVConfig*(*)>(_a[1]))); break;
        case 3: _t->sig_paraChange_smartChargeSwitch((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->sig_readAmmeterFail(); break;
        case 5: _t->sig_readAmmeterSucess(); break;
        case 6: _t->sig_setCCUResult((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 7: _t->sig_sendTermSignal((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 8: _t->sig_StartCharge((*reinterpret_cast< unsigned char(*)>(_a[1]))); break;
        case 9: _t->sig_chargeFromEnergyPlan((*reinterpret_cast< unsigned char(*)>(_a[1])),(*reinterpret_cast< unsigned char(*)>(_a[2]))); break;
        case 10: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 11: _t->ProcStartWork(); break;
        case 12: _t->slot_setPowerLimit((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData LoadSchedule::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LoadSchedule::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_LoadSchedule,
      qt_meta_data_LoadSchedule, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LoadSchedule::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LoadSchedule::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LoadSchedule::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LoadSchedule))
        return static_cast<void*>(const_cast< LoadSchedule*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int LoadSchedule::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CModuleIO::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void LoadSchedule::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LoadSchedule::sig_paraChange_powerLimit(stPowerLimitConfig _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LoadSchedule::sig_paraChange_smartCharge(stAllTPFVConfig * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LoadSchedule::sig_paraChange_smartChargeSwitch(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LoadSchedule::sig_readAmmeterFail()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void LoadSchedule::sig_readAmmeterSucess()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void LoadSchedule::sig_setCCUResult(InfoMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void LoadSchedule::sig_sendTermSignal(InfoMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void LoadSchedule::sig_StartCharge(unsigned char _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void LoadSchedule::sig_chargeFromEnergyPlan(unsigned char _t1, unsigned char _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}
QT_END_MOC_NAMESPACE
