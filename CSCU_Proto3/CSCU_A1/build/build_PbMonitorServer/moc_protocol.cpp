/****************************************************************************
** Meta object code from reading C++ file 'protocol.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ProtobufServer/protocol/protocol.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'protocol.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Protocol[] = {

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
      17,   10,    9,    9, 0x05,
      29,    9,    9,    9, 0x05,
      45,   36,    9,    9, 0x05,
      86,   74,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
     107,    9,    9,    9, 0x08,
     124,    9,    9,    9, 0x08,
     148,  144,    9,    9, 0x08,
     164,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Protocol[] = {
    "Protocol\0\0logged\0active(int)\0live()\0"
    "map,type\0parsed(InfoMap,InfoAddrType)\0"
    "qstrMessage\0sendMessage(QString)\0"
    "onNetConnected()\0onNetDisconnected()\0"
    "err\0onNetError(int)\0onNetReceive()\0"
};

void Protocol::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Protocol *_t = static_cast<Protocol *>(_o);
        switch (_id) {
        case 0: _t->active((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->live(); break;
        case 2: _t->parsed((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 3: _t->sendMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->onNetConnected(); break;
        case 5: _t->onNetDisconnected(); break;
        case 6: _t->onNetError((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->onNetReceive(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Protocol::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Protocol::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Protocol,
      qt_meta_data_Protocol, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Protocol::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Protocol::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Protocol::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Protocol))
        return static_cast<void*>(const_cast< Protocol*>(this));
    return QObject::qt_metacast(_clname);
}

int Protocol::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Protocol::active(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Protocol::live()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void Protocol::parsed(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Protocol::sendMessage(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
