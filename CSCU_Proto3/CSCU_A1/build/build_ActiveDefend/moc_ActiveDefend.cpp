/****************************************************************************
** Meta object code from reading C++ file 'ActiveDefend.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ActiveDefend/core/ActiveDefend.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ActiveDefend.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ActiveDefend[] = {

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
      27,   14,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      58,   14,   13,   13, 0x0a,
      92,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ActiveDefend[] = {
    "ActiveDefend\0\0mapInfo,type\0"
    "sigToBus(InfoMap,InfoAddrType)\0"
    "slotFromBus(InfoMap,InfoAddrType)\0"
    "slot_onThreadRun()\0"
};

void ActiveDefend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ActiveDefend *_t = static_cast<ActiveDefend *>(_o);
        switch (_id) {
        case 0: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 2: _t->slot_onThreadRun(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ActiveDefend::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ActiveDefend::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_ActiveDefend,
      qt_meta_data_ActiveDefend, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ActiveDefend::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ActiveDefend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ActiveDefend::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ActiveDefend))
        return static_cast<void*>(const_cast< ActiveDefend*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int ActiveDefend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CModuleIO::qt_metacall(_c, _id, _a);
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
void ActiveDefend::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
