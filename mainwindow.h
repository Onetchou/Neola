#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "timeline.h"

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QListWidget>
#include <QSettings>


QT_BEGIN_NAMESPACE
namespace Ui 
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private slots:
        void handleLoadAudioButton();
        void handlePlayButton();
        void handleInsertStopPointButton();
        void handleInsertStartPointButton();
        void handleSyncButton();
        void handleSaveButton();
        void handleOpenButton();
        void handlePositionSliderPressed();
        void handlePositionSliderReleased();
        void handlePlayerPositionChanged(const qint64 pos);
        void handlePlayerDurationChanged(const qint64 dur);
        void handleSynchroPointListItemDoubleClicked(QListWidgetItem* item);
        void handleWaitCheckbox(const bool checked);
        void handlePreferences();

    private:
        Ui::MainWindow *ui;
        QMediaPlayer *m_player = nullptr;
        QAudioOutput *m_audioOutput = nullptr;
        Timeline *m_timeline = nullptr;

        QSettings m_settings;
        SynchroPoints m_synchroPoints;
        QString m_audioPath;
        bool m_sliderPressed = false;
        bool m_waitAtSynchroPoint;
        qint64 m_nextSynchroPoint;

        void updateSynchroPointList();
        void sortSynchroPoints(SynchroPoints &points);
        void addSynchroPoint(const SynchroPoint &point);
        void setSynchroPoints(const SynchroPoints &points);
        void changePlayerPosition(const qint64 pos);
        void playPlayer();
        void pausePlayer();

        qint64 findNearestSynchroPoint(const qint64 posMs);
        void findNextSynchroPoint(const qint64 posMs);
};
#endif // MAINWINDOW_H
