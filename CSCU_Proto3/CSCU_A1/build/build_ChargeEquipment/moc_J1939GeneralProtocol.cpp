/****************************************************************************
** Meta object code from reading C++ file 'J1939GeneralProtocol.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ChargeEquipment/TerminalProtocol/J1939Protocol/J1939GeneralProtocol.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'J1939GeneralProtocol.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cJ1939GeneralProtocol[] = {

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
      56,   23,   22,   22, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_cJ1939GeneralProtocol[] = {
    "cJ1939GeneralProtocol\0\0"
    "pTerminalSendList,pSendListMutex\0"
    "sigSendCanData(QList<can_frame*>*,QMutex*)\0"
};

void cJ1939GeneralProtocol::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cJ1939GeneralProtocol *_t = static_cast<cJ1939GeneralProtocol *>(_o);
        switch (_id) {
        case 0: _t->sigSendCanData((*reinterpret_cast< QList<can_frame*>*(*)>(_a[1])),(*reinterpret_cast< QMutex*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cJ1939GeneralProtocol::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cJ1939GeneralProtocol::staticMetaObject = {
    { &cTerminalProtocol::staticMetaObject, qt_meta_stringdata_cJ1939GeneralProtocol,
      qt_meta_data_cJ1939GeneralProtocol, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cJ1939GeneralProtocol::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cJ1939GeneralProtocol::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cJ1939GeneralProtocol::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cJ1939GeneralProtocol))
        return static_cast<void*>(const_cast< cJ1939GeneralProtocol*>(this));
    return cTerminalProtocol::qt_metacast(_clname);
}

int cJ1939GeneralProtocol::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = cTerminalProtocol::qt_metacall(_c, _id, _a);
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
void cJ1939GeneralProtocol::sigSendCanData(QList<can_frame*> * _t1, QMutex * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
