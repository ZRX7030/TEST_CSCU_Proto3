/****************************************************************************
** Meta object code from reading C++ file 'CanBus.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ChargeEquipment/Can/CanBus.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CanBus.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cCanBusRecv[] = {

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
      13,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cCanBusRecv[] = {
    "cCanBusRecv\0\0ProcStart()\0"
};

void cCanBusRecv::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cCanBusRecv *_t = static_cast<cCanBusRecv *>(_o);
        switch (_id) {
        case 0: _t->ProcStart(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData cCanBusRecv::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cCanBusRecv::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cCanBusRecv,
      qt_meta_data_cCanBusRecv, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cCanBusRecv::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cCanBusRecv::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cCanBusRecv::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cCanBusRecv))
        return static_cast<void*>(const_cast< cCanBusRecv*>(this));
    return QObject::qt_metacast(_clname);
}

int cCanBusRecv::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_cCanBusSend[] = {

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
      13,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cCanBusSend[] = {
    "cCanBusSend\0\0ProcStart()\0"
};

void cCanBusSend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cCanBusSend *_t = static_cast<cCanBusSend *>(_o);
        switch (_id) {
        case 0: _t->ProcStart(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData cCanBusSend::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cCanBusSend::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cCanBusSend,
      qt_meta_data_cCanBusSend, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cCanBusSend::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cCanBusSend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cCanBusSend::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cCanBusSend))
        return static_cast<void*>(const_cast< cCanBusSend*>(this));
    return QObject::qt_metacast(_clname);
}

int cCanBusSend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_cCanBus[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      42,    9,    8,    8, 0x05,

 // slots: signature, parameters, type, tag, flags
      83,    8,    8,    8, 0x0a,
     132,   99,    8,    8, 0x0a,
     173,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_cCanBus[] = {
    "cCanBus\0\0pTerminalRecvList,pRecvListMutex\0"
    "sigParseData(QList<can_frame*>*,QMutex*)\0"
    "ProcStartWork()\0pTerminalSendList,pSendListMutex\0"
    "ProcSendData(QList<can_frame*>*,QMutex*)\0"
    "ProcMsecTimeOut()\0"
};

void cCanBus::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cCanBus *_t = static_cast<cCanBus *>(_o);
        switch (_id) {
        case 0: _t->sigParseData((*reinterpret_cast< QList<can_frame*>*(*)>(_a[1])),(*reinterpret_cast< QMutex*(*)>(_a[2]))); break;
        case 1: _t->ProcStartWork(); break;
        case 2: _t->ProcSendData((*reinterpret_cast< QList<can_frame*>*(*)>(_a[1])),(*reinterpret_cast< QMutex*(*)>(_a[2]))); break;
        case 3: _t->ProcMsecTimeOut(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cCanBus::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cCanBus::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cCanBus,
      qt_meta_data_cCanBus, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cCanBus::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cCanBus::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cCanBus::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cCanBus))
        return static_cast<void*>(const_cast< cCanBus*>(this));
    return QObject::qt_metacast(_clname);
}

int cCanBus::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void cCanBus::sigParseData(QList<can_frame*> * _t1, QMutex * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
