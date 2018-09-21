/****************************************************************************
** Meta object code from reading C++ file 'Update.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Update/Update.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Update.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Update[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      34,    8,    7,    7, 0x05,
      65,    7,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
      87,    7,    7,    7, 0x08,
     112,   97,    7,    7, 0x0a,
     146,    7,    7,    7, 0x0a,
     169,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Update[] = {
    "Update\0\0TelecontrolMap,enAddrType\0"
    "sigToBus(InfoMap,InfoAddrType)\0"
    "sigRunUpdate(QString)\0timeOut()\0"
    "Map,enAddrType\0slotFromBus(InfoMap,InfoAddrType)\0"
    "procRunUpdate(QString)\0procStartWork()\0"
};

void Update::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Update *_t = static_cast<Update *>(_o);
        switch (_id) {
        case 0: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->sigRunUpdate((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->timeOut(); break;
        case 3: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 4: _t->procRunUpdate((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->procStartWork(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Update::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Update::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_Update,
      qt_meta_data_Update, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Update::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Update::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Update::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Update))
        return static_cast<void*>(const_cast< Update*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int Update::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CModuleIO::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void Update::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Update::sigRunUpdate(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
