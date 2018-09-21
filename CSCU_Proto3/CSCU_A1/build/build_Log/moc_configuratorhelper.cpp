/****************************************************************************
** Meta object code from reading C++ file 'configuratorhelper.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Log/log4qt/helpers/configuratorhelper.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'configuratorhelper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Log4Qt__ConfiguratorHelper[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      44,   28,   27,   27, 0x05,

 // slots: signature, parameters, type, tag, flags
      93,   83,   27,   27, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Log4Qt__ConfiguratorHelper[] = {
    "Log4Qt::ConfiguratorHelper\0\0rFileName,error\0"
    "configurationFileChanged(QString,bool)\0"
    "rFileName\0doConfigurationFileChanged(QString)\0"
};

void Log4Qt::ConfiguratorHelper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ConfiguratorHelper *_t = static_cast<ConfiguratorHelper *>(_o);
        switch (_id) {
        case 0: _t->configurationFileChanged((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->doConfigurationFileChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Log4Qt::ConfiguratorHelper::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Log4Qt::ConfiguratorHelper::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Log4Qt__ConfiguratorHelper,
      qt_meta_data_Log4Qt__ConfiguratorHelper, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Log4Qt::ConfiguratorHelper::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Log4Qt::ConfiguratorHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Log4Qt::ConfiguratorHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Log4Qt__ConfiguratorHelper))
        return static_cast<void*>(const_cast< ConfiguratorHelper*>(this));
    return QObject::qt_metacast(_clname);
}

int Log4Qt::ConfiguratorHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Log4Qt::ConfiguratorHelper::configurationFileChanged(const QString & _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
