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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

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

    m_waitAtSynchroPoint = ui->waitCheckbox->isChecked();

    connect(ui->loadAudioButton,          &QPushButton::clicked, this, &MainWindow::handleLoadAudioButton);
    connect(ui->playButton,               &QPushButton::clicked, this, &MainWindow::handlePlayButton);
    connect(ui->insertSynchroPointButton, &QPushButton::clicked, this, &MainWindow::handleInsertSynchroPointButton);
    connect(ui->syncButton,               &QPushButton::clicked, this, &MainWindow::handleJumpToNearestSynchroPointButton);
    connect(ui->saveButton,               &QPushButton::clicked, this, &MainWindow::handleSaveButton);
    connect(ui->openButton,               &QPushButton::clicked, this, &MainWindow::handleOpenButton);

    connect(ui->synchroPointList, &QListWidget::itemDoubleClicked, this, &MainWindow::handleSynchroPointListItemDoubleClicked);

    connect(ui->waitCheckbox, &QCheckBox::toggled, this, &MainWindow::handleWaitCheckbox);

    connect(m_timeline, &QSlider::sliderPressed, this, &MainWindow::handlePositionSliderPressed);
    connect(m_timeline, &QSlider::sliderReleased, this, &MainWindow::handlePositionSliderReleased);

    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::handlePlayerPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MainWindow::handlePlayerDurationChanged);

    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::handlePreferences);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changePlayerPosition(const qint64 pos)
{
    m_player->setPosition(pos);
    m_nextSynchroPoint = findNextSynchroPoint(pos);
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


void MainWindow::handleInsertSynchroPointButton()
{
    qint64 pos = m_player->position();
    SynchroPoint point{ pos, "" };
    addSynchroPoint(point);
}


void MainWindow::handleJumpToNearestSynchroPointButton()
{
    if (m_synchroPoints.isEmpty())
    {
        return;
    }

    qint64 pos = m_player->position();
    qint64 target = findNearestSynchroPoint(pos);
    changePlayerPosition(target);
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
    m_nextSynchroPoint = findNextSynchroPoint(0);

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
        SynchroPoint point{obj["timestamp"].toInteger(), obj["name"].toString()};
        points.append(point);
    }
    sortSynchroPoints(points);

    setSynchroPoints(points);

    if (!m_audioPath.isEmpty())
    {
        m_player->setSource(QUrl::fromLocalFile(m_audioPath));
        m_player->stop(); // Start from the beginning of the file
        m_nextSynchroPoint = findNextSynchroPoint(0);
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
        // Display the current position
        int val = (m_player->duration()>0) ? int((pos*1000)/m_player->duration()) : 0;
        m_timeline->blockSignals(true);
        m_timeline->setValue(val);
        m_timeline->blockSignals(false);
        ui->timeLabel->setText(QString("%1 / %2").arg(QString::number(pos/1000.0,'f',2)).arg(QString::number(m_player->duration()/1000.0,'f',2)));

        // Check for synchro point
        if ( pos > m_nextSynchroPoint )
        {
            if (m_waitAtSynchroPoint)
            {
                pausePlayer();
            }
            m_nextSynchroPoint = findNextSynchroPoint(pos);
        }

    }
}


void MainWindow::handlePlayerDurationChanged(const qint64 dur)
{
    m_timeline->setDuration(dur);
}


void MainWindow::handleWaitCheckbox(const bool checked)
{
    m_waitAtSynchroPoint = checked;
}


void MainWindow::handlePreferences()
{
    PreferencesDialog dialog(m_settings, this);
    dialog.exec();
}
