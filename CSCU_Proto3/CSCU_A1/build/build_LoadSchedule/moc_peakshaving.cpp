/****************************************************************************
** Meta object code from reading C++ file 'peakshaving.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../LoadSchedule/peakshaving.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'peakshaving.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PeakShaving[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   13,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      53,   12,   12,   12, 0x0a,
      67,   12,   12,   12, 0x0a,
     106,  101,   12,   12, 0x0a,
     141,  135,   12,   12, 0x0a,
     178,   13,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PeakShaving[] = {
    "PeakShaving\0\0,\0sig_ChargeApply(InfoMap,InfoAddrType)\0"
    "ProcTimeOut()\0slot_paraChange(stAllTPFVConfig*)\0"
    "flag\0slot_smartChargeSwitch(bool)\0"
    "canID\0slot_StartChargingCMD(unsigned char)\0"
    "slot_chargeFromEnergyPlan(unsigned char,unsigned char)\0"
};

void PeakShaving::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PeakShaving *_t = static_cast<PeakShaving *>(_o);
        switch (_id) {
        case 0: _t->sig_ChargeApply((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->ProcTimeOut(); break;
        case 2: _t->slot_paraChange((*reinterpret_cast< stAllTPFVConfig*(*)>(_a[1]))); break;
        case 3: _t->slot_smartChargeSwitch((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->slot_StartChargingCMD((*reinterpret_cast< unsigned char(*)>(_a[1]))); break;
        case 5: _t->slot_chargeFromEnergyPlan((*reinterpret_cast< unsigned char(*)>(_a[1])),(*reinterpret_cast< unsigned char(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PeakShaving::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PeakShaving::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PeakShaving,
      qt_meta_data_PeakShaving, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PeakShaving::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PeakShaving::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PeakShaving::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PeakShaving))
        return static_cast<void*>(const_cast< PeakShaving*>(this));
    return QObject::qt_metacast(_clname);
}

int PeakShaving::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void PeakShaving::sig_ChargeApply(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
