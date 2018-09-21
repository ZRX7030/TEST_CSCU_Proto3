/****************************************************************************
** Meta object code from reading C++ file 'dlt645_07.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Ammeter/Protocol/dlt645_07.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dlt645_07.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_dlt645_07[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      43,   10,   10,   10, 0x05,
      65,   10,   10,   10, 0x05,
      89,   10,   10,   10, 0x05,
     113,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     164,  160,   10,   10, 0x0a,
     248,   10,   10,   10, 0x0a,
     299,   10,   10,   10, 0x0a,
     345,   10,   10,   10, 0x0a,
     365,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_dlt645_07[] = {
    "dlt645_07\0\0sendAmmeterData_645_07(InfoMap)\0"
    "sig_readFail_645_07()\0sig_readSucess_645_07()\0"
    "sigReadOver_645_07(int)\0"
    "sig_sendToBusRemoteAmmeterData_645_07(InfoMap)\0"
    ",,,\0"
    "slot_readRemoteAmmeter_dlt645_07(unsigned char*,int,unsigned char*,stA"
    "mmeterConfig)\0"
    "slot_readAmmeter_dlt645_07(QList<stAmmeterConfig>)\0"
    "slot_readPowerMonitorAmmeter(stAmmeterConfig)\0"
    "slot_stopRead(bool)\0slot_getBoardType(int)\0"
};

void dlt645_07::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        dlt645_07 *_t = static_cast<dlt645_07 *>(_o);
        switch (_id) {
        case 0: _t->sendAmmeterData_645_07((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 1: _t->sig_readFail_645_07(); break;
        case 2: _t->sig_readSucess_645_07(); break;
        case 3: _t->sigReadOver_645_07((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->sig_sendToBusRemoteAmmeterData_645_07((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 5: _t->slot_readRemoteAmmeter_dlt645_07((*reinterpret_cast< unsigned char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< unsigned char*(*)>(_a[3])),(*reinterpret_cast< stAmmeterConfig(*)>(_a[4]))); break;
        case 6: _t->slot_readAmmeter_dlt645_07((*reinterpret_cast< QList<stAmmeterConfig>(*)>(_a[1]))); break;
        case 7: _t->slot_readPowerMonitorAmmeter((*reinterpret_cast< stAmmeterConfig(*)>(_a[1]))); break;
        case 8: _t->slot_stopRead((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->slot_getBoardType((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData dlt645_07::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject dlt645_07::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_dlt645_07,
      qt_meta_data_dlt645_07, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &dlt645_07::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *dlt645_07::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *dlt645_07::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_dlt645_07))
        return static_cast<void*>(const_cast< dlt645_07*>(this));
    return QObject::qt_metacast(_clname);
}

int dlt645_07::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void dlt645_07::sendAmmeterData_645_07(InfoMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void dlt645_07::sig_readFail_645_07()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void dlt645_07::sig_readSucess_645_07()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void dlt645_07::sigReadOver_645_07(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void dlt645_07::sig_sendToBusRemoteAmmeterData_645_07(InfoMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
