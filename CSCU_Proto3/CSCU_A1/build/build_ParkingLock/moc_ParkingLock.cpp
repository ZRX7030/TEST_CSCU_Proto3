/****************************************************************************
** Meta object code from reading C++ file 'ParkingLock.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ParkingLock/ParkingLock.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ParkingLock.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ParkingLock[] = {

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
      26,   13,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      86,   57,   12,   12, 0x0a,
     120,   12,   12,   12, 0x08,
     136,   12,   12,   12, 0x08,
     156,   12,   12,   12, 0x08,
     204,  177,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ParkingLock[] = {
    "ParkingLock\0\0mapInfo,type\0"
    "sigToBus(InfoMap,InfoAddrType)\0"
    "RecvCenterDataMap,enAddrType\0"
    "slotFromBus(InfoMap,InfoAddrType)\0"
    "ProcStartWork()\0ProcOneSecTimeOut()\0"
    "ProcMinutesTimeOut()\0uiInfoAddr,TerminalDataMap\0"
    "ProcRecvProtocolData(uint,InfoMap)\0"
};

void ParkingLock::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ParkingLock *_t = static_cast<ParkingLock *>(_o);
        switch (_id) {
        case 0: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 2: _t->ProcStartWork(); break;
        case 3: _t->ProcOneSecTimeOut(); break;
        case 4: _t->ProcMinutesTimeOut(); break;
        case 5: _t->ProcRecvProtocolData((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< InfoMap(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ParkingLock::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ParkingLock::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_ParkingLock,
      qt_meta_data_ParkingLock, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ParkingLock::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ParkingLock::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ParkingLock::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ParkingLock))
        return static_cast<void*>(const_cast< ParkingLock*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int ParkingLock::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CModuleIO::qt_metacall(_c, _id, _a);
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
void ParkingLock::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
