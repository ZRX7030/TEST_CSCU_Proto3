#include "AlertDialog.h"
#include "AlertDialogplugin.h"

#include <QtPlugin>

AlertDialogPlugin::AlertDialogPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void AlertDialogPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool AlertDialogPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *AlertDialogPlugin::createWidget(QWidget *parent)
{
    return new AlertDialog(parent);
}

QString AlertDialogPlugin::name() const
{
    return QLatin1String("AlertDialog");
}

QString AlertDialogPlugin::group() const
{
    return QLatin1String("");
}

QIcon AlertDialogPlugin::icon() const
{
    return QIcon();
}

QString AlertDialogPlugin::toolTip() const
{
    return QLatin1String("");
}

QString AlertDialogPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool AlertDialogPlugin::isContainer() const
{
    return false;
}

QString AlertDialogPlugin::domXml() const
{
    return QLatin1String("<widget class=\"AlertDialog\" name=\"alertDialog\">\n</widget>\n");
}

QString AlertDialogPlugin::includeFile() const
{
    return QLatin1String("AlertDialog.h");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(alertdialogplugin, AlertDialogPlugin)
#endif // QT_VERSION < 0x050000
