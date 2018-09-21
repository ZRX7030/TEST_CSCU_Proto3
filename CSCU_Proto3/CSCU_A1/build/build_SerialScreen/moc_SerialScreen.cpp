/****************************************************************************
** Meta object code from reading C++ file 'SerialScreen.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SerialScreen/SerialScreen.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SerialScreen.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cSerialRead[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   13,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      71,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cSerialRead[] = {
    "cSerialRead\0\0pSerialData,iLength\0"
    "sigRecvSerialData(unsigned char*,int)\0"
    "ProcStartWork()\0"
};

void cSerialRead::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cSerialRead *_t = static_cast<cSerialRead *>(_o);
        switch (_id) {
        case 0: _t->sigRecvSerialData((*reinterpret_cast< unsigned char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->ProcStartWork(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cSerialRead::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cSerialRead::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cSerialRead,
      qt_meta_data_cSerialRead, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cSerialRead::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cSerialRead::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cSerialRead::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cSerialRead))
        return static_cast<void*>(const_cast< cSerialRead*>(this));
    return QObject::qt_metacast(_clname);
}

int cSerialRead::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void cSerialRead::sigRecvSerialData(unsigned char * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_cSerialWrite[] = {

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
      31,   14,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cSerialWrite[] = {
    "cSerialWrite\0\0pSerialData,iLen\0"
    "ProcSendSerialData(unsigned char*,int)\0"
};

void cSerialWrite::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cSerialWrite *_t = static_cast<cSerialWrite *>(_o);
        switch (_id) {
        case 0: _t->ProcSendSerialData((*reinterpret_cast< unsigned char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cSerialWrite::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cSerialWrite::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cSerialWrite,
      qt_meta_data_cSerialWrite, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cSerialWrite::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cSerialWrite::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cSerialWrite::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cSerialWrite))
        return static_cast<void*>(const_cast< cSerialWrite*>(this));
    return QObject::qt_metacast(_clname);
}

int cSerialWrite::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_cSerialScreen[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   15,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      62,   48,   14,   14, 0x0a,
     122,  101,   14,   14, 0x0a,
     156,   14,   14,   14, 0x0a,
     172,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_cSerialScreen[] = {
    "cSerialScreen\0\0,\0sigToBus(InfoMap,InfoAddrType)\0"
    "pData,iLength\0ProcRecvSerialData(unsigned char*,int)\0"
    "RecvCenterMap,enType\0"
    "slotFromBus(InfoMap,InfoAddrType)\0"
    "ProcStartWork()\0ProcOneSecTimeOut()\0"
};

void cSerialScreen::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cSerialScreen *_t = static_cast<cSerialScreen *>(_o);
        switch (_id) {
        case 0: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->ProcRecvSerialData((*reinterpret_cast< unsigned char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 3: _t->ProcStartWork(); break;
        case 4: _t->ProcOneSecTimeOut(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cSerialScreen::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cSerialScreen::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_cSerialScreen,
      qt_meta_data_cSerialScreen, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cSerialScreen::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cSerialScreen::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cSerialScreen::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cSerialScreen))
        return static_cast<void*>(const_cast< cSerialScreen*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int cSerialScreen::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CModuleIO::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void cSerialScreen::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
