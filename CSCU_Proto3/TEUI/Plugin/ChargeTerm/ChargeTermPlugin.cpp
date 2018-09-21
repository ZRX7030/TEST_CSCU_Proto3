#include "ChargeTerm.h"
#include "ChargeTermPlugin.h"

#include <QtPlugin>

ChargeTermPlugin::ChargeTermPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void ChargeTermPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool ChargeTermPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *ChargeTermPlugin::createWidget(QWidget *parent)
{
    return new ChargeTerm(parent);
}

QString ChargeTermPlugin::name() const
{
    return QLatin1String("ChargeTerm");
}

QString ChargeTermPlugin::group() const
{
    return QLatin1String("");
}

QIcon ChargeTermPlugin::icon() const
{
    return QIcon();
}

QString ChargeTermPlugin::toolTip() const
{
    return QLatin1String("");
}

QString ChargeTermPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool ChargeTermPlugin::isContainer() const
{
    return false;
}

QString ChargeTermPlugin::domXml() const
{
    return QLatin1String("<widget class=\"ChargeTerm\" name=\"chargeTerm\">\n</widget>\n");
}

QString ChargeTermPlugin::includeFile() const
{
    return QLatin1String("ChargeTerm.h");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(chargetermplugin, ChargeTermPlugin)
#endif // QT_VERSION < 0x050000
