#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <QMainWindow>
#include <QObject>
#include <QLineEdit>
#include <QDialog>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class PreferencesDialog;
}
QT_END_NAMESPACE

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QSettings &settings, QWidget *parent = nullptr);

private slots:
    void chooseOpenPath();
    void chooseSavePath();
    void chooseAudioImportPath();
    void saveSettings();

private:
    Ui::PreferencesDialog *ui;
    QSettings &m_settings;
    QLineEdit* openPathEdit;
    QLineEdit* savePathEdit;
    QLineEdit* audioPathEdit;

};

#endif // PREFERENCES_DIALOG_H
