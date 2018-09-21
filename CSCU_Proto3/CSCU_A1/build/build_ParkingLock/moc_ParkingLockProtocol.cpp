/****************************************************************************
** Meta object code from reading C++ file 'ParkingLockProtocol.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ParkingLock/ParkingLockProtocol.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ParkingLockProtocol.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cParkingLockProtocol[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      49,   22,   21,   21, 0x05,
      89,   79,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
     134,  116,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cParkingLockProtocol[] = {
    "cParkingLockProtocol\0\0uiInfoAddr,TerminalDataMap\0"
    "sigSendToCenter(uint,InfoMap)\0pCanFrame\0"
    "sigSendCanData(can_frame*)\0pTerminalRecvList\0"
    "ProcParseData(QList<can_frame*>*)\0"
};

void cParkingLockProtocol::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cParkingLockProtocol *_t = static_cast<cParkingLockProtocol *>(_o);
        switch (_id) {
        case 0: _t->sigSendToCenter((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< InfoMap(*)>(_a[2]))); break;
        case 1: _t->sigSendCanData((*reinterpret_cast< can_frame*(*)>(_a[1]))); break;
        case 2: _t->ProcParseData((*reinterpret_cast< QList<can_frame*>*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cParkingLockProtocol::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cParkingLockProtocol::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cParkingLockProtocol,
      qt_meta_data_cParkingLockProtocol, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cParkingLockProtocol::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cParkingLockProtocol::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cParkingLockProtocol::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cParkingLockProtocol))
        return static_cast<void*>(const_cast< cParkingLockProtocol*>(this));
    return QObject::qt_metacast(_clname);
}

int cParkingLockProtocol::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void cParkingLockProtocol::sigSendToCenter(unsigned int _t1, InfoMap _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void cParkingLockProtocol::sigSendCanData(can_frame * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
