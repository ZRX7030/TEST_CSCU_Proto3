/****************************************************************************
** Meta object code from reading C++ file 'ScanCode.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ScanCode/ScanCode.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScanCode.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cScanCode[] = {

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
      11,   10,   10,   10, 0x05,
      50,   33,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      81,   33,   10,   10, 0x0a,
     115,   10,   10,   10, 0x08,
     135,   10,  131,   10, 0x08,
     150,   33,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_cScanCode[] = {
    "cScanCode\0\0sendScanCode(InfoMap)\0"
    "CardMap,CardType\0sigToBus(InfoMap,InfoAddrType)\0"
    "slotFromBus(InfoMap,InfoAddrType)\0"
    "ProcStartWork()\0int\0readScanCode()\0"
    "slotFormDealScanCode(InfoMap,InfoAddrType)\0"
};

void cScanCode::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cScanCode *_t = static_cast<cScanCode *>(_o);
        switch (_id) {
        case 0: _t->sendScanCode((*reinterpret_cast< InfoMap(*)>(_a[1]))); break;
        case 1: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 2: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 3: _t->ProcStartWork(); break;
        case 4: { int _r = _t->readScanCode();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: _t->slotFormDealScanCode((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cScanCode::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cScanCode::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_cScanCode,
      qt_meta_data_cScanCode, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cScanCode::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cScanCode::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cScanCode::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cScanCode))
        return static_cast<void*>(const_cast< cScanCode*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int cScanCode::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void cScanCode::sendScanCode(InfoMap _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void cScanCode::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
