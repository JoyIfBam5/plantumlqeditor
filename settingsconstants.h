#ifndef SETTINGSCONSTANTS_H
#define SETTINGSCONSTANTS_H

#include <QString>

const QString SETTINGS_MAIN_SECTION = "MainWindow";
const QString SETTINGS_GEOMETRY = "geometry";
const QString SETTINGS_WINDOW_STATE = "window_state";
const QString SETTINGS_SHOW_STATUSBAR = "show_statusbar";
const QString SETTINGS_AUTOREFRESH_ENABLED = "autorefresh_enabled";
const QString SETTINGS_AUTOREFRESH_TIMEOUT = "autorefresh_timeout";
const int     SETTINGS_AUTOREFRESH_TIMEOUT_DEFAULT = 5000; // in miliseconds
const QString SETTINGS_IMAGE_FORMAT = "image_format";
const QString SETTINGS_USE_CUSTOM_JAVA = "use_custom_java";
const bool    SETTINGS_USE_CUSTOM_JAVA_DEFAULT = true;
const QString SETTINGS_CUSTOM_JAVA_PATH = "custom_java";
const QString SETTINGS_CUSTOM_JAVA_PATH_DEFAULT = "/usr/bin/java";
const QString SETTINGS_USE_CUSTOM_PLANTUML = "use_custom_plantuml";
const bool    SETTINGS_USE_CUSTOM_PLANTUML_DEFAULT = true;
const QString SETTINGS_CUSTOM_PLANTUML_PATH = "custom_plantuml";
const QString SETTINGS_CUSTOM_PLANTUML_PATH_DEFAULT = "/usr/bin/plantuml";
const QString SETTINGS_ASSISTANT_XML_PATH = "assistant_xml";

const QString SETTINGS_RECENT_DOCUMENTS_SECTION = "recent_documents";
const QString SETTINGS_RECENT_DOCUMENTS_DOCUMENT = "document";

const QString SETTINGS_PREFERENCES_SECTION = "Preferences";

#endif // SETTINGSCONSTANTS_H
