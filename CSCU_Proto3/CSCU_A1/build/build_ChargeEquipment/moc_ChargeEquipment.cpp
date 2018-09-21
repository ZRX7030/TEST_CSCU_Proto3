/****************************************************************************
** Meta object code from reading C++ file 'ChargeEquipment.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ChargeEquipment/ChargeEquipment.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChargeEquipment.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cJ1939PreParse[] = {

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
      49,   16,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cJ1939PreParse[] = {
    "cJ1939PreParse\0\0pTerminalRecvList,pRecvListMutex\0"
    "ProcParseData(QList<can_frame*>*,QMutex*)\0"
};

void cJ1939PreParse::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cJ1939PreParse *_t = static_cast<cJ1939PreParse *>(_o);
        switch (_id) {
        case 0: _t->ProcParseData((*reinterpret_cast< QList<can_frame*>*(*)>(_a[1])),(*reinterpret_cast< QMutex*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cJ1939PreParse::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cJ1939PreParse::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cJ1939PreParse,
      qt_meta_data_cJ1939PreParse, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cJ1939PreParse::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cJ1939PreParse::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cJ1939PreParse::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cJ1939PreParse))
        return static_cast<void*>(const_cast< cJ1939PreParse*>(this));
    return QObject::qt_metacast(_clname);
}

int cJ1939PreParse::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_cChargeEquipment[] = {

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
      44,   18,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      75,   17,   17,   17, 0x0a,
     120,   91,   17,   17, 0x0a,
     154,   17,   17,   17, 0x08,
     201,  174,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_cChargeEquipment[] = {
    "cChargeEquipment\0\0TelecontrolMap,enAddrType\0"
    "sigToBus(InfoMap,InfoAddrType)\0"
    "ProcStartWork()\0RecvCenterDataMap,enAddrType\0"
    "slotFromBus(InfoMap,InfoAddrType)\0"
    "ProcOneSecTimeOut()\0uiInfoAddr,TerminalDataMap\0"
    "ProcRecvProtocolData(uint,InfoMap)\0"
};

void cChargeEquipment::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cChargeEquipment *_t = static_cast<cChargeEquipment *>(_o);
        switch (_id) {
        case 0: _t->sigToBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 1: _t->ProcStartWork(); break;
        case 2: _t->slotFromBus((*reinterpret_cast< InfoMap(*)>(_a[1])),(*reinterpret_cast< InfoAddrType(*)>(_a[2]))); break;
        case 3: _t->ProcOneSecTimeOut(); break;
        case 4: _t->ProcRecvProtocolData((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< InfoMap(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cChargeEquipment::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cChargeEquipment::staticMetaObject = {
    { &CModuleIO::staticMetaObject, qt_meta_stringdata_cChargeEquipment,
      qt_meta_data_cChargeEquipment, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cChargeEquipment::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cChargeEquipment::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cChargeEquipment::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cChargeEquipment))
        return static_cast<void*>(const_cast< cChargeEquipment*>(this));
    return CModuleIO::qt_metacast(_clname);
}

int cChargeEquipment::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void cChargeEquipment::sigToBus(InfoMap _t1, InfoAddrType _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
