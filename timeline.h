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
    qint64 timestamp = -1;
    QString name = "";
    SynchroPointType type = StopPoint;
    int id = -1;
};
Q_DECLARE_METATYPE(SynchroPoint)

inline bool operator==(const SynchroPoint &a, const SynchroPoint &b)
{
    return a.id == b.id;
}

using SynchroPoints = QVector<SynchroPoint>;

class Timeline : public QSlider
{
    Q_OBJECT

public:
    explicit Timeline(QWidget *parent = nullptr);

    void setSynchroPoints (const SynchroPoints& synchroPoints);
    void setDuration(qint64 durationMs);

    const SynchroPoints& getSynchroPoints() const;

protected:
    void paintEvent(QPaintEvent *ev) override;

private:
    SynchroPoints m_synchroPoints;
    qint64 m_duration = 0;

    void paintSynchroPoints(QPainter &p, QRect groove, QRect handle);
};

#endif // TIMELINE_H
