#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QFileDialog>

#include "settingsconstants.h"
#include <QSettings>

namespace {
const int TIMEOUT_SCALE = 1000;
}

PreferencesDialog::PreferencesDialog(FileCache* file_cache, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::PreferencesDialog)
    , m_fileCache(file_cache)
{
    m_ui->setupUi(this);

    connect(this, SIGNAL(rejected()), this, SLOT(onRejected()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete m_ui;
}

void PreferencesDialog::readSettings()
{
    QSettings settings;

    settings.beginGroup(SETTINGS_MAIN_SECTION);

    m_ui->customJavaPathEdit->setText(settings.value(SETTINGS_CUSTOM_JAVA_PATH).toString());
    m_ui->customPlantUmlEdit->setText(settings.value(SETTINGS_CUSTOM_PLANTUML_PATH).toString());
    m_ui->autoRefreshSpin->setValue(settings.value(SETTINGS_AUTOREFRESH_TIMEOUT).toInt() / TIMEOUT_SCALE);
    m_ui->assistantXmlEdit->setText(settings.value(SETTINGS_ASSISTANT_XML_PATH).toString());

    if (settings.value(SETTINGS_USE_CUSTOM_JAVA, SETTINGS_USE_CUSTOM_JAVA_DEFAULT).toBool())
        m_ui->customJavaRadio->setChecked(true);
    else
        m_ui->defaultJavaRadio->setChecked(true);

    if (settings.value(SETTINGS_USE_CUSTOM_PLANTUML, SETTINGS_USE_CUSTOM_PLANTUML_DEFAULT).toBool())
        m_ui->customPlantUmlRadio->setChecked(true);
    else
        m_ui->defaultPlatUmlRadio->setChecked(true);

    settings.endGroup();

    settings.beginGroup(SETTINGS_PREFERENCES_SECTION);
    restoreGeometry(settings.value(SETTINGS_GEOMETRY).toByteArray());
    settings.endGroup();
}

void PreferencesDialog::writeSettings()
{
    QSettings settings;

    settings.beginGroup(SETTINGS_MAIN_SECTION);

    settings.setValue(SETTINGS_CUSTOM_JAVA_PATH, m_ui->customJavaPathEdit->text());
    settings.setValue(SETTINGS_CUSTOM_PLANTUML_PATH, m_ui->customPlantUmlEdit->text());
    settings.setValue(SETTINGS_AUTOREFRESH_TIMEOUT, m_ui->autoRefreshSpin->value() * TIMEOUT_SCALE);
    settings.setValue(SETTINGS_ASSISTANT_XML_PATH, m_ui->assistantXmlEdit->text());
    settings.setValue(SETTINGS_USE_CUSTOM_JAVA, m_ui->customJavaRadio->isChecked());
    settings.setValue(SETTINGS_USE_CUSTOM_PLANTUML, m_ui->customPlantUmlRadio->isChecked());
    settings.endGroup();

    settings.beginGroup(SETTINGS_PREFERENCES_SECTION);
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
    settings.endGroup();
}

void PreferencesDialog::onRejected()
{
    QSettings settings;
    settings.beginGroup(SETTINGS_PREFERENCES_SECTION);
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
    settings.endGroup();
}

void PreferencesDialog::on_customJavaPathButton_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,
                                                    tr("Select Java executable"),
                                                    m_ui->customJavaPathEdit->text());
    if (!file_name.isEmpty())
        m_ui->customJavaPathEdit->setText(file_name);
}

void PreferencesDialog::on_customPlantUmlButton_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,
                                                    tr("Select PlantUML jar"),
                                                    m_ui->customPlantUmlEdit->text());
    if (!file_name.isEmpty())
        m_ui->customPlantUmlEdit->setText(file_name);
}

void PreferencesDialog::on_assistantXmlButton_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,
                                                     tr("Select Assistant XML file"),
                                                     m_ui->assistantXmlEdit->text(),
                                                     tr("XML (*.xml);;All Files (*.*)"));
    if (!file_name.isEmpty())
        m_ui->assistantXmlEdit->setText(file_name);
}
