/****************************************************************************
** Meta object code from reading C++ file 'Can1Bus.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ParkingLock/Can1/Can1Bus.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Can1Bus.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cCan1BusRecv[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cCan1BusRecv[] = {
    "cCan1BusRecv\0\0ProcStart()\0"
};

void cCan1BusRecv::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cCan1BusRecv *_t = static_cast<cCan1BusRecv *>(_o);
        switch (_id) {
        case 0: _t->ProcStart(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData cCan1BusRecv::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cCan1BusRecv::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cCan1BusRecv,
      qt_meta_data_cCan1BusRecv, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cCan1BusRecv::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cCan1BusRecv::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cCan1BusRecv::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cCan1BusRecv))
        return static_cast<void*>(const_cast< cCan1BusRecv*>(this));
    return QObject::qt_metacast(_clname);
}

int cCan1BusRecv::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_cCan1BusSend[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_cCan1BusSend[] = {
    "cCan1BusSend\0"
};

void cCan1BusSend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData cCan1BusSend::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cCan1BusSend::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cCan1BusSend,
      qt_meta_data_cCan1BusSend, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cCan1BusSend::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cCan1BusSend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cCan1BusSend::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cCan1BusSend))
        return static_cast<void*>(const_cast< cCan1BusSend*>(this));
    return QObject::qt_metacast(_clname);
}

int cCan1BusSend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_cCan1Bus[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   10,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      71,   61,    9,    9, 0x0a,
      96,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_cCan1Bus[] = {
    "cCan1Bus\0\0pTerminalRecvList\0"
    "sigParseData(QList<can_frame*>*)\0"
    "pCanFrame\0ProcSendData(can_frame*)\0"
    "ProcMsecTimeOut()\0"
};

void cCan1Bus::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cCan1Bus *_t = static_cast<cCan1Bus *>(_o);
        switch (_id) {
        case 0: _t->sigParseData((*reinterpret_cast< QList<can_frame*>*(*)>(_a[1]))); break;
        case 1: _t->ProcSendData((*reinterpret_cast< can_frame*(*)>(_a[1]))); break;
        case 2: _t->ProcMsecTimeOut(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cCan1Bus::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cCan1Bus::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cCan1Bus,
      qt_meta_data_cCan1Bus, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cCan1Bus::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cCan1Bus::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cCan1Bus::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cCan1Bus))
        return static_cast<void*>(const_cast< cCan1Bus*>(this));
    return QObject::qt_metacast(_clname);
}

int cCan1Bus::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void cCan1Bus::sigParseData(QList<can_frame*> * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
