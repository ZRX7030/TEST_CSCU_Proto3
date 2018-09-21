/****************************************************************************
** Meta object code from reading C++ file 'SerialScreenProtocol.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SerialScreen/SerialScreenProtocol.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SerialScreenProtocol.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cSerialScreenProtocol[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   23,   22,   22, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_cSerialScreenProtocol[] = {
    "cSerialScreenProtocol\0\0pData,iLength\0"
    "sigSendSerialData(unsigned char*,int)\0"
};

void cSerialScreenProtocol::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cSerialScreenProtocol *_t = static_cast<cSerialScreenProtocol *>(_o);
        switch (_id) {
        case 0: _t->sigSendSerialData((*reinterpret_cast< unsigned char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cSerialScreenProtocol::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cSerialScreenProtocol::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cSerialScreenProtocol,
      qt_meta_data_cSerialScreenProtocol, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cSerialScreenProtocol::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cSerialScreenProtocol::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cSerialScreenProtocol::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cSerialScreenProtocol))
        return static_cast<void*>(const_cast< cSerialScreenProtocol*>(this));
    return QObject::qt_metacast(_clname);
}

int cSerialScreenProtocol::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void cSerialScreenProtocol::sigSendSerialData(unsigned char * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
