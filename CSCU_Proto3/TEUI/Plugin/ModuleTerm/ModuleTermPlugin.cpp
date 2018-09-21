#include "ModuleTerm.h"
#include "ModuleTermPlugin.h"

#include <QtPlugin>

ModuleTermPlugin::ModuleTermPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void ModuleTermPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool ModuleTermPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *ModuleTermPlugin::createWidget(QWidget *parent)
{
    return new ModuleTerm(parent);
}

QString ModuleTermPlugin::name() const
{
    return QLatin1String("ModuleTerm");
}

QString ModuleTermPlugin::group() const
{
    return QLatin1String("");
}

QIcon ModuleTermPlugin::icon() const
{
    return QIcon();
}

QString ModuleTermPlugin::toolTip() const
{
    return QLatin1String("");
}

QString ModuleTermPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool ModuleTermPlugin::isContainer() const
{
    return false;
}

QString ModuleTermPlugin::domXml() const
{
    return QLatin1String("<widget class=\"ModuleTerm\" name=\"moduleTerm\">\n</widget>\n");
}

QString ModuleTermPlugin::includeFile() const
{
    return QLatin1String("ModuleTerm.h");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(moduletermplugin, ModuleTermPlugin)
#endif // QT_VERSION < 0x050000
