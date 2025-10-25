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
#include <QStyle>
#include <QStyleOptionSlider>
#include <QDebug>
#include <iostream>


// MarkerSlider implementation
MarkerSlider::MarkerSlider(QWidget *parent): QSlider(Qt::Horizontal, parent) 
{ 
    setMinimum(0);
    setMaximum(1000); 
}


void MarkerSlider::setMarkers(const QVector<Marker> &markers) 
{ 
    m_markers = markers; update(); 
}


void MarkerSlider::setDuration(qint64 durationMs) 
{ 
    m_duration = durationMs; update(); 
}


void MarkerSlider::paintEvent(QPaintEvent *ev)
{
    QSlider::paintEvent(ev);
    if (m_markers.isEmpty() || m_duration <= 0) 
    {
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);


    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect groove = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    int left = groove.left();
    int right = groove.right();
    int w = right - left;


    for (const Marker &m : m_markers) 
    {
        double t = double(m.positionMs) / m_duration;
        if (t > 1.0) t = 1.0;
        int x = left + int(t * w);
        int y = groove.center().y();
        QRect r(x - 3, y - 10, 6, 20);
        p.fillRect(r, Qt::red);
        p.drawText(x + 2, y - 12, QString::number(m.measureNumber));
    }
}


void MainWindow::on_loadButtonClicked()
{
     QString file = QFileDialog::getOpenFileName(
        this,
        tr("Ouvrir piste audio"),
        QString(),
        tr("Fichiers audio (*.mp3 *.wav *.flac *.ogg);;Tous les fichiers (*)")
        );

    qDebug() << "Fichier choisi :" << file;

    if (file.isEmpty())
    {
         return;
    }

    m_audioPath = file;  // Sauvegarde le chemin
    m_player->setSource(QUrl::fromLocalFile(file));  // Charge dans le player
    m_player->stop();  // Assure que la lecture commence depuis le début

    ui->playButton->setText(tr("Play"));  // Réinitialise le bouton Play
    m_nextMeasureNumber = 1;  // Réinitialise la numérotation des marqueurs
}


void MainWindow::on_saveMarkersButtonClicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Sauvegarder marqueurs"), QString(), tr("JSON (*.json)"));
    if (file.isEmpty()) 
    {
        return;
    }
    QJsonObject root;
    root["audioPath"] = m_audioPath;
    QJsonArray arr;
    for (const Marker &m : m_markers) 
    {
        QJsonObject obj;
        obj["pos"] = QJsonValue::fromVariant(QVariant::fromValue(m.positionMs));
        obj["measure"] = m.measureNumber;
        arr.append(obj);
    }
    root["markers"] = arr;
    QJsonDocument doc(root);
    QFile f(file);
    if (!f.open(QIODevice::WriteOnly)) 
    { 
        qWarning() << "Impossible d'ouvrir" << file; return; 
    }
    f.write(doc.toJson());
    f.close();
}


void MainWindow::on_loadMarkersButtonClicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Charger marqueurs"), QString(), tr("JSON (*.json)"));
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
    QJsonArray arr = root.value("markers").toArray();
    m_markers.clear();
    m_nextMeasureNumber = 1;
    for (auto v : arr) 
    {
        QJsonObject obj = v.toObject();
        Marker m{ obj["pos"].toVariant().toLongLong(), obj["measure"].toInt() };
        m_markers.append(m);
        if (m.measureNumber >= m_nextMeasureNumber) 
        {
            m_nextMeasureNumber = m.measureNumber + 1;
        }
    }
    std::sort(m_markers.begin(), m_markers.end(), [](const Marker &a, const Marker &b){ return a.positionMs < b.positionMs; });
    m_markerSlider->setMarkers(m_markers);
    updateMarkerList();
    if (!m_audioPath.isEmpty()) 
    {
        m_player->setSource(QUrl::fromLocalFile(m_audioPath));
    }
}


void MainWindow::on_positionSliderPressed()
{ 
    m_sliderPressed = true; 
}


void MainWindow::on_positionSliderReleased()
{ 
    m_sliderPressed = false; qint64 pos = (m_player->duration() * qint64(m_markerSlider->value())) / 1000; m_player->setPosition(pos); 
}


void MainWindow::on_playerPositionChanged(qint64 pos)
{
    if (!m_sliderPressed) 
    {
        int val = (m_player->duration()>0) ? int((pos*1000)/m_player->duration()) : 0;
        m_markerSlider->blockSignals(true);
        m_markerSlider->setValue(val);
        m_markerSlider->blockSignals(false);
        ui->timeLabel->setText(QString("%1 / %2").arg(QString::number(pos/1000.0,'f',2)).arg(QString::number(m_player->duration()/1000.0,'f',2)));
    }
}


void MainWindow::on_playerDurationChanged(qint64 dur)
{ 
    m_markerSlider->setDuration(dur); 
}


void MainWindow::updateMarkerList()
{
    ui->markerList->clear();
    for (const Marker &m : m_markers)
    {
         ui->markerList->addItem("Mesure " + QString::number(m.measureNumber) + 
                            " : " + QString::number(m.positionMs/1000.0, 'f', 3) + " s");
    }
}


qint64 MainWindow::findNearestMarker(qint64 posMs)
{
    if (m_markers.isEmpty())
    {
        return posMs;
    }
    qint64 best = m_markers.first().positionMs;
    qint64 bestDist = llabs(best-posMs);
    for (const Marker &m : m_markers)
    {
        qint64 d = llabs(m.positionMs-posMs);
        if (d < bestDist) 
        { 
            bestDist=d; 
            best=m.positionMs; 
        }
    }
    return best;
}


void MainWindow::on_playButtonClicked()
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


void MainWindow::on_insertMarkerButtonClicked()
{
    qint64 pos = m_player->position();
    Marker m{ pos, m_nextMeasureNumber++ };
    m_markers.append(m);

    // Tri des marqueurs
    std::sort(m_markers.begin(), m_markers.end(), [](const Marker &a, const Marker &b) {
        return a.positionMs < b.positionMs;
    });

    m_markerSlider->setMarkers(m_markers);
    updateMarkerList();
}

void MainWindow::on_jumpMarkerButtonClicked()
{
    if (m_markers.isEmpty()) 
    {
        return;
    }
    qint64 pos = m_player->position();
    qint64 target = findNearestMarker(pos);
    m_player->setPosition(target);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "TESTESTESTESTESTEST";
    std::cout << "PIPOPIPOPIPO" << std::endl;
    ui->setupUi(this);

    QWidget *place = ui->positionWidget;
    m_markerSlider = new MarkerSlider(this);
    QHBoxLayout *lay = new QHBoxLayout(place);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(m_markerSlider);


    connect(ui->loadButton, &QPushButton::clicked, this, &MainWindow::on_loadButtonClicked);
    connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::on_playButtonClicked);
    connect(ui->insertMarkerButton, &QPushButton::clicked, this, &MainWindow::on_insertMarkerButtonClicked);
    connect(ui->jumpMarkerButton, &QPushButton::clicked, this, &MainWindow::on_jumpMarkerButtonClicked);
    connect(ui->saveMarkersButton, &QPushButton::clicked, this, &MainWindow::on_saveMarkersButtonClicked);
    connect(ui->loadMarkersButton, &QPushButton::clicked, this, &MainWindow::on_loadMarkersButtonClicked);


    connect(m_markerSlider, &QSlider::sliderPressed, this, &MainWindow::on_positionSliderPressed);
    connect(m_markerSlider, &QSlider::sliderReleased, this, &MainWindow::on_positionSliderReleased);


    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);


    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::on_playerPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MainWindow::on_playerDurationChanged);


}

MainWindow::~MainWindow()
{
    delete ui;
}
