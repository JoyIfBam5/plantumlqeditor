#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::PreferencesDialog)
{
    m_ui->setupUi(this);
}

PreferencesDialog::~PreferencesDialog()
{
    delete m_ui;
}

void PreferencesDialog::setJavaPath(const QString &path)
{
    m_ui->javaPathEdit->setText(path);
}

QString PreferencesDialog::javaPath() const
{
    return m_ui->javaPathEdit->text();
}

void PreferencesDialog::setPlantUmlPath(const QString &path)
{
    m_ui->plantUmlEdit->setText(path);
}

QString PreferencesDialog::plantUmlPath() const
{
    return m_ui->plantUmlEdit->text();
}

void PreferencesDialog::setAutoRefreshTimeout(int timeout)
{
    m_ui->autoRefreshSpin->setValue(timeout);
}

int PreferencesDialog::autoRefreshTimeout() const
{
    return m_ui->autoRefreshSpin->value();
}

void PreferencesDialog::setAssistantXml(const QString &path)
{
    m_ui->assistantXmlEdit->setText(path);
}

QString PreferencesDialog::assistantXml() const
{
    return m_ui->assistantXmlEdit->text();
}

void PreferencesDialog::on_javaPathButton_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,
                                                    tr("Select Java executable"),
                                                    m_ui->javaPathEdit->text());
    if (!file_name.isEmpty())
        m_ui->javaPathEdit->setText(file_name);
}

void PreferencesDialog::on_plantUmlButton_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,
                                                    tr("Select PlantUML jar"),
                                                    m_ui->plantUmlEdit->text());
    if (!file_name.isEmpty())
        m_ui->plantUmlEdit->setText(file_name);
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
