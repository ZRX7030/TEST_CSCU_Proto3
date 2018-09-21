/****************************************************************************
** Meta object code from reading C++ file 'consoleappender.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Log/log4qt/consoleappender.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'consoleappender.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Log4Qt__ConsoleAppender[] = {

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
      32,   24, 0x0a095103,

 // enums: name, flags, count, data
      39, 0x0,    2,   21,

 // enum data: key, value
      46, uint(Log4Qt::ConsoleAppender::STDOUT_TARGET),
      60, uint(Log4Qt::ConsoleAppender::STDERR_TARGET),

       0        // eod
};

static const char qt_meta_stringdata_Log4Qt__ConsoleAppender[] = {
    "Log4Qt::ConsoleAppender\0QString\0target\0"
    "Target\0STDOUT_TARGET\0STDERR_TARGET\0"
};

void Log4Qt::ConsoleAppender::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Log4Qt::ConsoleAppender::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Log4Qt::ConsoleAppender::staticMetaObject = {
    { &WriterAppender::staticMetaObject, qt_meta_stringdata_Log4Qt__ConsoleAppender,
      qt_meta_data_Log4Qt__ConsoleAppender, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Log4Qt::ConsoleAppender::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Log4Qt::ConsoleAppender::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Log4Qt::ConsoleAppender::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Log4Qt__ConsoleAppender))
        return static_cast<void*>(const_cast< ConsoleAppender*>(this));
    return WriterAppender::qt_metacast(_clname);
}

int Log4Qt::ConsoleAppender::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = WriterAppender::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = target(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setTarget(*reinterpret_cast< QString*>(_v)); break;
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
