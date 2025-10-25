#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include <algorithm>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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

    connect(ui->loadButton, &QPushButton::clicked, this, &MainWindow::handleLoadButton);
    connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::handlePlayButton);
    connect(ui->insertSynchroPointButton, &QPushButton::clicked, this, &MainWindow::handleInsertSynchroPointButton);
    connect(ui->syncButton, &QPushButton::clicked, this, &MainWindow::handleJumpToNearestSynchroPointButton);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::handleSaveButton);
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::handleOpenButton);

    connect(m_timeline, &QSlider::sliderPressed, this, &MainWindow::handlePositionSliderPressed);
    connect(m_timeline, &QSlider::sliderReleased, this, &MainWindow::handlePositionSliderReleased);

    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::handlePlayerPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MainWindow::handlePlayerDurationChanged);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handleLoadButton()
{
     QString file = QFileDialog::getOpenFileName(
        this,
        tr("Open Audio File"),
        QString(),
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

    ui->playButton->setText(tr("Play"));
}


void MainWindow::handleSaveButton()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save project"), QString(), tr("NEOLA (*.neola)"));
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
    QString file = QFileDialog::getOpenFileName(this, tr("Load project"), QString(), tr("NEOLA (*.neola)"));
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
    m_synchroPoints.clear();
    for (QJsonValueRef v : arr)
    {
        QJsonObject obj = v.toObject();
        SynchroPoint m{obj["timestamp"].toInteger(), obj["name"].toString()};
        m_synchroPoints.append(m);
    }
    sortSynchroPoints();

    m_timeline->setSynchroPoint(m_synchroPoints);
    updateSynchroPointList();

    if (!m_audioPath.isEmpty()) 
    {
        m_player->setSource(QUrl::fromLocalFile(m_audioPath));
        m_player->stop(); // Start from the beginning of the file
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
    m_player->setPosition(pos);
}


void MainWindow::handlePlayerPositionChanged(qint64 pos)
{
    if (!m_sliderPressed) 
    {
        int val = (m_player->duration()>0) ? int((pos*1000)/m_player->duration()) : 0;
        m_timeline->blockSignals(true);
        m_timeline->setValue(val);
        m_timeline->blockSignals(false);
        ui->timeLabel->setText(QString("%1 / %2").arg(QString::number(pos/1000.0,'f',2)).arg(QString::number(m_player->duration()/1000.0,'f',2)));
    }
}


void MainWindow::handlePlayerDurationChanged(qint64 dur)
{ 
    m_timeline->setDuration(dur);
}


void MainWindow::updateSynchroPointList()
{
    ui->synchroPointList->clear();
    for (const SynchroPoint &m : m_synchroPoints)
    {
         ui->synchroPointList->addItem("Name " + m.name + " : " + QString::number(m.timestamp/1000.0, 'f', 3) + " s");
    }
}


qint64 MainWindow::findNearestSynchroPoint(qint64 posMs)
{
    if (m_synchroPoints.isEmpty())
    {
        return posMs;
    }

    qint64 best = m_synchroPoints.first().timestamp;
    qint64 bestDist = llabs(best-posMs);
    for (const SynchroPoint &m : m_synchroPoints)
    {
        qint64 d = llabs(m.timestamp-posMs);
        if (d < bestDist) 
        { 
            bestDist=d; 
            best=m.timestamp;
        }
    }
    return best;
}


void MainWindow::handlePlayButton()
{
    if (m_player->playbackState() != QMediaPlayer::PlayingState) 
    {
        m_player->play();
        ui->playButton->setText(tr("Pause"));
    } 
    else 
    {
        m_player->pause();
        ui->playButton->setText(tr("Play"));
    }
}

void MainWindow::sortSynchroPoints()
{
    auto compareSynchroPoints = [](const SynchroPoint &a, const SynchroPoint &b)
    {
        return a.timestamp < b.timestamp;
    };

    std::sort(m_synchroPoints.begin(), m_synchroPoints.end(), compareSynchroPoints);
}


void MainWindow::handleInsertSynchroPointButton()
{
    qint64 pos = m_player->position();
    SynchroPoint m{ pos, "" };
    m_synchroPoints.append(m);
    sortSynchroPoints();

    m_timeline->setSynchroPoint(m_synchroPoints);
    updateSynchroPointList();
}

void MainWindow::handleJumpToNearestSynchroPointButton()
{
    if (m_synchroPoints.isEmpty())
    {
        return;
    }

    qint64 pos = m_player->position();
    qint64 target = findNearestSynchroPoint(pos);
    m_player->setPosition(target);
}

