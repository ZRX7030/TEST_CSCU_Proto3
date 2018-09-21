/****************************************************************************
** Meta object code from reading C++ file 'modbus.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Ammeter/Protocol/modbus.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'modbus.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_modbus[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x05,
      40,    7,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
      73,   64,    7,    7, 0x0a,
     121,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_modbus[] = {
    "modbus\0\0sendAmmeterData_modbus(InfoMap)\0"
    "sigReadOver_modbus(int)\0infoList\0"
    "slot_readAmmeter_modbus(QList<stAmmeterConfig>)\0"
    "slot_getBoardType(int)\0"
};

void modbus::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        modbus *_t = static_cast<modbus *>(_o);
        switch (_id) {
        case 0: _t->sendAmmeterData_modbus((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 1: _t->sigReadOver_modbus((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slot_readAmmeter_modbus((*reinterpret_cast< QList<stAmmeterConfig>(*)>(_a[1]))); break;
        case 3: _t->slot_getBoardType((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData modbus::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject modbus::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_modbus,
      qt_meta_data_modbus, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &modbus::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *modbus::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *modbus::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_modbus))
        return static_cast<void*>(const_cast< modbus*>(this));
    return QObject::qt_metacast(_clname);
}

int modbus::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void modbus::sendAmmeterData_modbus(InfoMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void modbus::sigReadOver_modbus(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
