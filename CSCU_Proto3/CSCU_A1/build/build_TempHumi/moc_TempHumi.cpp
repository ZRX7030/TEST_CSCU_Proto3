/****************************************************************************
** Meta object code from reading C++ file 'TempHumi.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../TempHumi/TempHumi.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TempHumi.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cTempHumi[] = {

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
      11,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      37,   10,   10,   10, 0x08,
      51,   10,   10,   10, 0x08,
      67,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_cTempHumi[] = {
    "cTempHumi\0\0sendTempHumiData(InfoMap)\0"
    "ProcTimeOut()\0ProcStartWork()\0"
    "slotParseTempHumiData(InfoMap)\0"
};

void cTempHumi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cTempHumi *_t = static_cast<cTempHumi *>(_o);
        switch (_id) {
        case 0: _t->sendTempHumiData((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 1: _t->ProcTimeOut(); break;
        case 2: _t->ProcStartWork(); break;
        case 3: _t->slotParseTempHumiData((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cTempHumi::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cTempHumi::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_cTempHumi,
      qt_meta_data_cTempHumi, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cTempHumi::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cTempHumi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cTempHumi::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cTempHumi))
        return static_cast<void*>(const_cast< cTempHumi*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int cTempHumi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CModuleIO::qt_metacall(_c, _id, _a);
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
void cTempHumi::sendTempHumiData(InfoMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
