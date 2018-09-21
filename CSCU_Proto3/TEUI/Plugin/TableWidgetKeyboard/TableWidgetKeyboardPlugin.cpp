#include "TableWidgetKeyboard.h"
#include "TableWidgetKeyboardPlugin.h"

#include <QtPlugin>

TableWidgetKeyboardPlugin::TableWidgetKeyboardPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void TableWidgetKeyboardPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;
    
    // Add extension registrations, etc. here
    
    m_initialized = true;
}

bool TableWidgetKeyboardPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *TableWidgetKeyboardPlugin::createWidget(QWidget *parent)
{
    return new TableWidgetKeyboard(parent);
}

QString TableWidgetKeyboardPlugin::name() const
{
    return QLatin1String("TableWidgetKeyboard");
}

QString TableWidgetKeyboardPlugin::group() const
{
    return QLatin1String("");
}

QIcon TableWidgetKeyboardPlugin::icon() const
{
    return QIcon();
}

QString TableWidgetKeyboardPlugin::toolTip() const
{
    return QLatin1String("");
}

QString TableWidgetKeyboardPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool TableWidgetKeyboardPlugin::isContainer() const
{
    return false;
}

QString TableWidgetKeyboardPlugin::domXml() const
{
    return QLatin1String("<widget class=\"TableWidgetKeyboard\" name=\"tableWidgetKeyboard\">\n</widget>\n");
}

QString TableWidgetKeyboardPlugin::includeFile() const
{
    return QLatin1String("TableWidgetKeyboard.h");
}

Q_EXPORT_PLUGIN2(tablewidgetkeyboardplugin, TableWidgetKeyboardPlugin)
