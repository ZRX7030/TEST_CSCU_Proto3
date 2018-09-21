/****************************************************************************
** Meta object code from reading C++ file 'LCDScreen.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../LCDScreen/LCDScreen.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LCDScreen.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cLCDScreen[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      66,   45,   11,   11, 0x0a,
     100,   11,   11,   11, 0x0a,
     116,   11,   11,   11, 0x08,
     135,   11,   11,   11, 0x08,
     149,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_cLCDScreen[] = {
    "cLCDScreen\0\0,\0sigToBus(InfoMap,InfoAddrType)\0"
    "RecvCenterMap,enType\0"
    "slotFromBus(InfoMap,InfoAddrType)\0"
    "ProcStartWork()\0acceptConnection()\0"
    "recvMessage()\0ProcOneSecTimeOut()\0"
};

void cLCDScreen::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cLCDScreen *_t = static_cast<cLCDScreen *>(_o);
        switch (_id) {
        case 0: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 2: _t->ProcStartWork(); break;
        case 3: _t->acceptConnection(); break;
        case 4: _t->recvMessage(); break;
        case 5: _t->ProcOneSecTimeOut(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cLCDScreen::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cLCDScreen::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_cLCDScreen,
      qt_meta_data_cLCDScreen, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cLCDScreen::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cLCDScreen::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cLCDScreen::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cLCDScreen))
        return static_cast<void*>(const_cast< cLCDScreen*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int cLCDScreen::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void cLCDScreen::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
