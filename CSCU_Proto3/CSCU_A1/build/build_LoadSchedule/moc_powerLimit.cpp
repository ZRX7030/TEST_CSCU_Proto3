/****************************************************************************
** Meta object code from reading C++ file 'powerLimit.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../LoadSchedule/powerLimit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'powerLimit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PowerLimit[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   12,   11,   11, 0x05,
      54,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      91,   11,   11,   11, 0x0a,
     107,   11,   11,   11, 0x0a,
     121,   11,   11,   11, 0x0a,
     157,   11,   11,   11, 0x0a,
     180,   11,   11,   11, 0x0a,
     206,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PowerLimit[] = {
    "PowerLimit\0\0,\0sig_setLimitPower(InfoMap,InfoAddrType)\0"
    "sig_stopCharge(InfoMap,InfoAddrType)\0"
    "ProcStartWork()\0ProcTimeOut()\0"
    "slot_paraChange(stPowerLimitConfig)\0"
    "slot_readAmmeterFail()\0slot_readAmmeterSuccess()\0"
    "slot_setCCUResult(InfoMap)\0"
};

void PowerLimit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PowerLimit *_t = static_cast<PowerLimit *>(_o);
        switch (_id) {
        case 0: _t->sig_setLimitPower((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->sig_stopCharge((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 2: _t->ProcStartWork(); break;
        case 3: _t->ProcTimeOut(); break;
        case 4: _t->slot_paraChange((*reinterpret_cast< stPowerLimitConfig(*)>(_a[1]))); break;
        case 5: _t->slot_readAmmeterFail(); break;
        case 6: _t->slot_readAmmeterSuccess(); break;
        case 7: _t->slot_setCCUResult((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PowerLimit::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PowerLimit::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PowerLimit,
      qt_meta_data_PowerLimit, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PowerLimit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PowerLimit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PowerLimit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PowerLimit))
        return static_cast<void*>(const_cast< PowerLimit*>(this));
    return QObject::qt_metacast(_clname);
}

int PowerLimit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void PowerLimit::sig_setLimitPower(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PowerLimit::sig_stopCharge(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
