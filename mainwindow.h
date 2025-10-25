#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVector>
#include <QString>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui 
{
    class MainWindow;
}
QT_END_NAMESPACE

struct Marker 
{
    qint64 positionMs;
    int measureNumber;
};


class MarkerSlider : public QSlider 
{
    Q_OBJECT
    public:
        explicit MarkerSlider(QWidget *parent = nullptr);
        void setMarkers(const QVector<Marker>& markers);
        const QVector<Marker>& markers() const { return m_markers; }
        void setDuration(qint64 durationMs);
        void setZoomFactor(double factor); // 1.0 = 100%
        protected:
        void paintEvent(QPaintEvent *ev) override;
        private:
        QVector<Marker> m_markers;
        qint64 m_duration = 0;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private slots:
        void on_loadButtonClicked();
        void on_playButtonClicked();
        void on_insertMarkerButtonClicked();
        void on_jumpMarkerButtonClicked();
        void on_saveMarkersButtonClicked();
        void on_loadMarkersButtonClicked();
        void on_positionSliderPressed();
        void on_positionSliderReleased();
        void on_playerPositionChanged(qint64 pos);
        void on_playerDurationChanged(qint64 dur);

    private:
        Ui::MainWindow *ui;
        QMediaPlayer *m_player = nullptr;
        QAudioOutput *m_audioOutput = nullptr;
        MarkerSlider *m_markerSlider = nullptr;
        QVector<Marker> m_markers;
        QString m_audioPath;
        bool m_sliderPressed = false;
        int m_nextMeasureNumber = 1;
        void updateMarkerList();
        qint64 findNearestMarker(qint64 posMs);
};
#endif // MAINWINDOW_H
