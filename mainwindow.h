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
        void handleSaveAsButton();
        void handleSave();
        void handleOpenButton();
        void handlePositionSliderPressed();
        void handlePositionSliderReleased();
        void handlePlayerPositionChanged(const qint64 pos);
        void handlePlayerDurationChanged(const qint64 dur);
        void handleSynchroPointListItemDoubleClicked(QListWidgetItem* item);
        void handleSynchroPointListItemSelection();
        void handlePreferences();
        void handleNew();
        void handleNameEdit();
        void handleTimestampSpinbox();
        void handleTimingCorrectionSpinbox();
        void handlePreviousPointButton();
        void handleRestartButton();
        void handleDeletePoint();
        void keyPressEvent(QKeyEvent* event) override;

    private:
        Ui::MainWindow *ui;
        QMediaPlayer *m_player = nullptr;
        QAudioOutput *m_audioOutput = nullptr;
        Timeline *m_timeline = nullptr;

        QSettings m_settings;
        QString m_currentFile;
        SynchroPoints m_synchroPoints;
        QString m_audioPath;
        bool m_sliderPressed = false;
        SynchroPoint m_nextSynchroPoint;
        SynchroPoint m_selectedSynchroPoint;
        qint64 m_timingCorrection = 0;

        void updateSynchroPointList();
        void updateSynchroPoint(SynchroPoints& synchroPoints, const SynchroPoint& point);
        void deleteSynchroPoint(const SynchroPoint& point);
        void sortSynchroPoints(SynchroPoints &points);
        void addSynchroPoint(const SynchroPoint &point);
        void setSynchroPoints(const SynchroPoints &points);
        void changePlayerPosition(qint64 pos);
        void playPlayer();
        void pausePlayer();
        void setCurrentFile(QString file);
        QJsonDocument createJsonDocument();
        SynchroPoint getSelectedSynchroPoint();
        qint64 findNearestSynchroPoint(const qint64 posMs);
        SynchroPoint findPreviousSynchroPoint(const qint64 posMs);
        void findNextSynchroPoint(const qint64 posMs);
        void findNextStopPoint(const qint64 posMs);
        void findNextStartPoint(const qint64 posMs);
        void highlightSelectedSynchroPoint();
        void scrollToNextSynchroPoint();
        int getNewId();
        bool checkUniqueIds(const SynchroPoints &points);
};
#endif // MAINWINDOW_H
