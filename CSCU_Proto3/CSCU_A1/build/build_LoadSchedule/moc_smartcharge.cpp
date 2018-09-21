/****************************************************************************
** Meta object code from reading C++ file 'smartcharge.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../LoadSchedule/smartcharge.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'smartcharge.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SmartCharge[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   13,   12,   12, 0x05,
      52,   13,   12,   12, 0x05,
      91,   12,   12,   12, 0x05,
     105,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
     131,   12,   12,   12, 0x0a,
     145,   12,   12,   12, 0x0a,
     159,   12,   12,   12, 0x0a,
     198,  193,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SmartCharge[] = {
    "SmartCharge\0\0,\0sig_stopCharge(InfoMap,InfoAddrType)\0"
    "sig_adjChargeCur(InfoMap,InfoAddrType)\0"
    "sig_timeout()\0sig_clearcurruntnum(uint)\0"
    "slotFromBus()\0ProcTimeOut()\0"
    "slot_paraChange(stAllTPFVConfig*)\0"
    "flag\0slot_smartChargeSwitch(bool)\0"
};

void SmartCharge::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SmartCharge *_t = static_cast<SmartCharge *>(_o);
        switch (_id) {
        case 0: _t->sig_stopCharge((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->sig_adjChargeCur((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 2: _t->sig_timeout(); break;
        case 3: _t->sig_clearcurruntnum((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 4: _t->slotFromBus(); break;
        case 5: _t->ProcTimeOut(); break;
        case 6: _t->slot_paraChange((*reinterpret_cast< stAllTPFVConfig*(*)>(_a[1]))); break;
        case 7: _t->slot_smartChargeSwitch((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SmartCharge::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SmartCharge::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SmartCharge,
      qt_meta_data_SmartCharge, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SmartCharge::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SmartCharge::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SmartCharge::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SmartCharge))
        return static_cast<void*>(const_cast< SmartCharge*>(this));
    return QObject::qt_metacast(_clname);
}

int SmartCharge::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void SmartCharge::sig_stopCharge(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SmartCharge::sig_adjChargeCur(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SmartCharge::sig_timeout()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void SmartCharge::sig_clearcurruntnum(unsigned int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
