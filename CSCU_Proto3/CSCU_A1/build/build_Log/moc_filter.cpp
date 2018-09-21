/****************************************************************************
** Meta object code from reading C++ file 'filter.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Log/log4qt/spi/filter.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'filter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Log4Qt__Filter[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       1,   14, // properties
       1,   17, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      23,   15, 0x0009510b,

 // enums: name, flags, count, data
      28, 0x0,    3,   21,

 // enum data: key, value
      37, uint(Log4Qt::Filter::ACCEPT),
      44, uint(Log4Qt::Filter::DENY),
      49, uint(Log4Qt::Filter::NEUTRAL),

       0        // eod
};

static const char qt_meta_stringdata_Log4Qt__Filter[] = {
    "Log4Qt::Filter\0Filter*\0next\0Decision\0"
    "ACCEPT\0DENY\0NEUTRAL\0"
};

void Log4Qt::Filter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Log4Qt::Filter::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Log4Qt::Filter::staticMetaObject = {
    { &LogObject::staticMetaObject, qt_meta_stringdata_Log4Qt__Filter,
      qt_meta_data_Log4Qt__Filter, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Log4Qt::Filter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Log4Qt::Filter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Log4Qt::Filter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Log4Qt__Filter))
        return static_cast<void*>(const_cast< Filter*>(this));
    return LogObject::qt_metacast(_clname);
}

int Log4Qt::Filter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = LogObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< Filter**>(_v) = next(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setNext(*reinterpret_cast< Filter**>(_v)); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_END_MOC_NAMESPACE
