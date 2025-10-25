#include "preferences_dialog.h"
#include "ui_preferences_dialog.h"

#include <QPushButton>
#include <QFormLayout>
#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QSettings &settings, QWidget *parent): QDialog(parent)
    , ui(new Ui::PreferencesDialog)
    , m_settings(settings)
{
    ui->setupUi(this);

    connect(ui->btnBrowseOpen, &QPushButton::clicked, this, &PreferencesDialog::chooseOpenPath);
    connect(ui->btnBrowseSave, &QPushButton::clicked, this, &PreferencesDialog::chooseSavePath);
    connect(ui->btnBrowseAudio, &QPushButton::clicked, this, &PreferencesDialog::chooseAudioImportPath);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::saveSettings);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::reject);

    // Charger les préférences existantes
    ui->lineOpenPath->setText(m_settings.value("paths/open", "").toString());
    ui->lineSavePath->setText(m_settings.value("paths/save", "").toString());
    ui->lineAudioPath->setText(m_settings.value("paths/audio", "").toString());
}


void PreferencesDialog::chooseOpenPath() {
    QString dir = QFileDialog::getExistingDirectory(this, "Choisir chemin d'ouverture");
    if (!dir.isEmpty()) ui->lineOpenPath->setText(dir);
}


void PreferencesDialog::chooseSavePath() {
    QString dir = QFileDialog::getExistingDirectory(this, "Choisir chemin de sauvegarde");
    if (!dir.isEmpty()) ui->lineSavePath->setText(dir);
}


void PreferencesDialog::chooseAudioImportPath() {
    QString dir = QFileDialog::getExistingDirectory(this, "Choisir chemin import audio");
    if (!dir.isEmpty()) ui->lineAudioPath->setText(dir);
}


void PreferencesDialog::saveSettings() {
    m_settings.setValue("paths/open", ui->lineOpenPath->text());
    m_settings.setValue("paths/save", ui->lineSavePath->text());
    m_settings.setValue("paths/audio", ui->lineAudioPath->text());
    accept();
}
