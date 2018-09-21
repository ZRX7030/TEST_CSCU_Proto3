#include "LineEditKeyboard.h"
#include "LineEditKeyboardplugin.h"

#include <QtPlugin>

LineEditKeyboardPlugin::LineEditKeyboardPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void LineEditKeyboardPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;
    
    // Add extension registrations, etc. here
    
    m_initialized = true;
}

bool LineEditKeyboardPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *LineEditKeyboardPlugin::createWidget(QWidget *parent)
{
    return new LineEditKeyboard(parent);
}

QString LineEditKeyboardPlugin::name() const
{
    return QLatin1String("LineEditKeyboard");
}

QString LineEditKeyboardPlugin::group() const
{
    return QLatin1String("");
}

QIcon LineEditKeyboardPlugin::icon() const
{
    return QIcon();
}

QString LineEditKeyboardPlugin::toolTip() const
{
    return QLatin1String("");
}

QString LineEditKeyboardPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool LineEditKeyboardPlugin::isContainer() const
{
    return false;
}

QString LineEditKeyboardPlugin::domXml() const
{
    return QLatin1String("<widget class=\"LineEditKeyboard\" name=\"lineEditKeyboard\">\n</widget>\n");
}

QString LineEditKeyboardPlugin::includeFile() const
{
    return QLatin1String("LineEditKeyboard.h");
}

Q_EXPORT_PLUGIN2(lineeditkeyboardplugin, LineEditKeyboardPlugin)
