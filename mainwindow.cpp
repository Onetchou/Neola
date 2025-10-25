#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "preferences_dialog.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include <QCheckBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSpinBox>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

    m_selectedSynchroPoint = SynchroPoint{-1, "", StopPoint, 0};

    // Setup the timeline
    QWidget *place = ui->positionWidget;
    m_timeline = new Timeline(this);
    QHBoxLayout *lay = new QHBoxLayout(place);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(m_timeline);

    // Setup the media player
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);

    connect(ui->actionImport_Audio,       &QAction::triggered,   this, &MainWindow::handleLoadAudioButton);
    connect(ui->actionInsert_stop_point,  &QAction::triggered,   this, &MainWindow::handleInsertStopPointButton);
    connect(ui->actionInsert_start_point, &QAction::triggered,   this, &MainWindow::handleInsertStartPointButton);
    connect(ui->actionSync,               &QAction::triggered,   this, &MainWindow::handleSyncButton);
    connect(ui->actionSave_as,            &QAction::triggered,   this, &MainWindow::handleSaveButton);
    connect(ui->actionOpen,               &QAction::triggered,   this, &MainWindow::handleOpenButton);
    connect(ui->actionPreferences,        &QAction::triggered,   this, &MainWindow::handlePreferences);
    connect(ui->actionNew,                &QAction::triggered,   this, &MainWindow::handleNew);

    connect(ui->playButton,               &QPushButton::clicked, this, &MainWindow::handlePlayButton);
    connect(ui->insertStopPointButton,    &QPushButton::clicked, this, &MainWindow::handleInsertStopPointButton);
    connect(ui->insertStartPointButton,   &QPushButton::clicked, this, &MainWindow::handleInsertStartPointButton);
    connect(ui->syncButton,               &QPushButton::clicked, this, &MainWindow::handleSyncButton);

    connect(ui->synchroPointList, &QListWidget::itemDoubleClicked, this, &MainWindow::handleSynchroPointListItemDoubleClicked);
    connect(ui->synchroPointList, &QListWidget::itemSelectionChanged, this, &MainWindow::handleSynchroPointListItemSelection);

    connect(ui->nameEdit, &QLineEdit::returnPressed, this, &MainWindow::handleNameEdit);

    connect(ui->timestampSpinBox, &QSpinBox::valueChanged, this, &MainWindow::handleTimestampSpinbox);

    connect(m_timeline, &QSlider::sliderPressed, this, &MainWindow::handlePositionSliderPressed);
    connect(m_timeline, &QSlider::sliderReleased, this, &MainWindow::handlePositionSliderReleased);

    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::handlePlayerPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MainWindow::handlePlayerDurationChanged);

}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changePlayerPosition(const qint64 pos)
{
    m_player->setPosition(pos);
    findNextSynchroPoint(pos);
}


void MainWindow::pausePlayer()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
    {
        m_player->pause();
        ui->playButton->setText(tr("Play"));
    }
}


void MainWindow::playPlayer()
{
    if (m_player->playbackState() != QMediaPlayer::PlayingState)
    {
        m_player->play();
        ui->playButton->setText(tr("Pause"));
    }
}


void MainWindow::handlePlayButton()
{
    if (m_player->playbackState() != QMediaPlayer::PlayingState)
    {
        playPlayer();
    }
    else
    {
        pausePlayer();
    }
}


void MainWindow::handleInsertStopPointButton()
{
    qint64 pos = m_player->position();
    SynchroPoint point{ pos, "SyncStopPoint", StopPoint, getNewId()};
    addSynchroPoint(point);
}


void MainWindow::handleInsertStartPointButton()
{
    qint64 pos = m_player->position();
    SynchroPoint point{ pos, "SyncStartPoint", StartPoint, getNewId()};
    addSynchroPoint(point);
}


void MainWindow::handleSyncButton()
{
    qint64 pos = m_player->position();

    if (pos == 0)
    {
        // Clicking "Sync" at the beginning of the audio file starts it
        playPlayer();
        return;
    }

    if (m_nextSynchroPoint.timestamp == -1 || m_synchroPoints.isEmpty())
    {
        return;
    }

    if (m_nextSynchroPoint.type != StartPoint)
    {
        findNextStartPoint(pos);
    }
    changePlayerPosition(m_nextSynchroPoint.timestamp);
    playPlayer();
}


void MainWindow::handleLoadAudioButton()
{
    QString default_path = m_settings.value("paths/audio", "").toString();
    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Open Audio File"),
        default_path,
        tr("Audio Files (*.mp3 *.wav *.ogg);;All files (*)")
        );

    qDebug() << "Chosen file :" << file;

    if (file.isEmpty())
    {
        return;
    }

    m_audioPath = file;
    m_player->setSource(QUrl::fromLocalFile(file));
    m_player->stop(); // Start from the beginning of the file
    findNextSynchroPoint(0);

    ui->playButton->setText(tr("Play"));
}


void MainWindow::handleSaveButton()
{
    QString default_path = m_settings.value("paths/save", "").toString();
    QString file = QFileDialog::getSaveFileName(this, tr("Save project"), default_path, tr("NEOLA (*.neola)"));
    if (file.isEmpty())
    {
        return;
    }

    QJsonObject root;
    root["audioPath"] = m_audioPath;

    QJsonArray arr;
    for (const SynchroPoint &m : m_synchroPoints)
    {
        QJsonObject obj;
        obj["timestamp"] = m.timestamp;
        obj["name"] = m.name;
        obj["type"] = m.type == StartPoint ? "start" : "stop";
        obj["id"] = m.id;
        arr.append(obj);
    }
    root["synchroPoints"] = arr;

    QJsonDocument doc(root);
    QFile f(file);
    if (!f.open(QIODevice::WriteOnly))
    {
        qWarning() << "Unable to open " << file;
        return;
    }

    f.write(doc.toJson());
    f.close();
}


void MainWindow::handleOpenButton()
{
    QString default_path = m_settings.value("paths/open", "").toString();
    QString file = QFileDialog::getOpenFileName(this, tr("Load project"), default_path, tr("NEOLA (*.neola)"));
    if (file.isEmpty())
    {
        return;
    }

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly))
    {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject())
    {
        return;
    }

    QJsonObject root = doc.object();
    m_audioPath = root.value("audioPath").toString();

    QJsonArray arr = root.value("synchroPoints").toArray();

    SynchroPoints points;

    for (QJsonValueRef v : arr)
    {
        QJsonObject obj = v.toObject();
        SynchroPoint point{obj["timestamp"].toInteger(), obj["name"].toString(), obj["type"].toString() == "start" ? StartPoint : StopPoint, obj["id"].toInt()};
        points.append(point);
    }
    sortSynchroPoints(points);

    setSynchroPoints(points);

    if (!m_audioPath.isEmpty())
    {
        m_player->setSource(QUrl::fromLocalFile(m_audioPath));
        m_player->stop(); // Start from the beginning of the file
        findNextSynchroPoint(0);
    }
}


void MainWindow::handlePositionSliderPressed()
{
    m_sliderPressed = true;
}


void MainWindow::handlePositionSliderReleased()
{
    m_sliderPressed = false;
    qint64 pos = (m_player->duration() * qint64(m_timeline->value())) / 1000;
    changePlayerPosition(pos);
}


void MainWindow::handlePlayerPositionChanged(const qint64 pos)
{
    if (!m_sliderPressed)
    {
        // Move the timeline
        int val = (m_player->duration()>0) ? int((pos*1000)/m_player->duration()) : 0;
        m_timeline->blockSignals(true);
        m_timeline->setValue(val);
        m_timeline->blockSignals(false);

        // Update time label
        ui->timeLabel->setText(QString("%1 / %2").arg(QString::number(pos/1000.0,'f',2)).arg(QString::number(m_player->duration()/1000.0,'f',2)));

        // Check for synchro point
        if ( m_nextSynchroPoint.timestamp != -1 && pos > m_nextSynchroPoint.timestamp )
        {
            if (m_nextSynchroPoint.type == StopPoint)
            {
                pausePlayer();
                findNextStartPoint(pos);
            }
            else if (m_nextSynchroPoint.type == StartPoint)
            {
                findNextStopPoint(pos);
            }
        }
    }
}


void MainWindow::handlePlayerDurationChanged(const qint64 dur)
{
    m_timeline->setDuration(dur);
}


void MainWindow::handlePreferences()
{
    PreferencesDialog dialog(m_settings, this);
    dialog.exec();
}


void MainWindow::handleNew()
{
    m_audioPath.clear();
    m_player->stop();
    m_synchroPoints.clear();
    m_selectedSynchroPoint = SynchroPoint{ -1, "", StartPoint, -1 };
    updateSynchroPointList();
    m_timeline->setSynchroPoints(m_synchroPoints);
    m_timeline->setValue(0);
    ui->timeLabel->setText("0.00 / 0.00");
    ui->playButton->setText(tr("Play"));
    ui->timestampSpinBox->setValue(0);
    ui->nameEdit->setText("");
}


void MainWindow::handleNameEdit()
{
    // Change the name of the selected item
    if (m_selectedSynchroPoint.timestamp != -1)
    {
        m_selectedSynchroPoint.name = ui->nameEdit->text();
        updateSynchroPoint(m_synchroPoints, m_selectedSynchroPoint);
        updateSynchroPointList();
    }
}


void MainWindow::handleTimestampSpinbox()
{
    // Change the timestamp of the selected item
    if (m_selectedSynchroPoint.timestamp != -1)
    {
        m_selectedSynchroPoint.timestamp = ui->timestampSpinBox->value();
        updateSynchroPoint(m_synchroPoints, m_selectedSynchroPoint);
        updateSynchroPointList();
        m_timeline->setSynchroPoints(m_synchroPoints);
        findNextSynchroPoint(m_player->position());
    }
}


void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space)
    {
        handleSyncButton();
    }
}
