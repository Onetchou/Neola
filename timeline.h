#ifndef TIMELINE_H
#define TIMELINE_H

#include <QSlider>

enum SynchroPointType
{
    StartPoint,
    StopPoint
};

struct SynchroPoint
{
    qint64 timestamp;
    QString name;
    SynchroPointType type;
};
Q_DECLARE_METATYPE(SynchroPoint)

using SynchroPoints = QVector<SynchroPoint>;

class Timeline : public QSlider
{
    Q_OBJECT

public:
    explicit Timeline(QWidget *parent = nullptr);

    void setSynchroPoint (const SynchroPoints& synchroPoints);
    void setDuration(qint64 durationMs);
    void setNextSynchroPoint(qint64 timestamp);

    const SynchroPoints& getSynchroPoints() const;

protected:
    void paintEvent(QPaintEvent *ev) override;

private:
    SynchroPoints m_synchroPoints;
    qint64 m_duration = 0;
    qint64 m_nextSynchroPoint = -1;

    void paintSynchroPoints(QPainter &p, QRect groove);
};

#endif // TIMELINE_H
