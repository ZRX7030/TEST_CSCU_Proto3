#include "TimeLimit.h"
#include "TimeLimitPlugin.h"

#include <QtPlugin>

TimeLimitPlugin::TimeLimitPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void TimeLimitPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool TimeLimitPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *TimeLimitPlugin::createWidget(QWidget *parent)
{
    return new TimeLimit(parent);
}

QString TimeLimitPlugin::name() const
{
    return QLatin1String("TimeLimit");
}

QString TimeLimitPlugin::group() const
{
    return QLatin1String("");
}

QIcon TimeLimitPlugin::icon() const
{
    return QIcon();
}

QString TimeLimitPlugin::toolTip() const
{
    return QLatin1String("");
}

QString TimeLimitPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool TimeLimitPlugin::isContainer() const
{
    return false;
}

QString TimeLimitPlugin::domXml() const
{
    return QLatin1String("<widget class=\"TimeLimit\" name=\"timeLimit\">\n</widget>\n");
}

QString TimeLimitPlugin::includeFile() const
{
    return QLatin1String("TimeLimit.h");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(timelimitplugin, TimeLimitPlugin)
#endif // QT_VERSION < 0x050000
