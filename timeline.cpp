#include "timeline.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>


Timeline::Timeline(QWidget *parent): QSlider(Qt::Horizontal, parent)
{
    setMinimum(0);
    setMaximum(1000);
    setSingleStep(0);
    setPageStep(0);
}


void Timeline::setSynchroPoint(const SynchroPoints &synchroPoints)
{
    m_synchroPoints = synchroPoints;
    update();
}


const SynchroPoints& Timeline::getSynchroPoints() const
{
    return m_synchroPoints;
}


void Timeline::setDuration(qint64 durationMs)
{
    m_duration = durationMs;
    update();
}


void Timeline::paintEvent(QPaintEvent *ev)
{
    QSlider::paintEvent(ev);

    if (m_synchroPoints.isEmpty() || m_duration <= 0)
    {
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QStyleOptionSlider opt;
    initStyleOption(&opt);

    QRect groove = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);

    paintSynchroPoints(p, groove);
}


void Timeline::paintSynchroPoints(QPainter &p, QRect groove)
{
    int left = groove.left();
    int right = groove.right();
    int w = right - left;
    int y = groove.center().y();

    for (const SynchroPoint &m : m_synchroPoints)
    {
        double t = double(m.timestamp) / m_duration;
        if (t > 1.0)
        {
            t = 1.0;
        }

        int x = left + int(t * w);

        QRect r(x - 3, y - 10, 6, 20);

        if (m.type == StartPoint)
        {
            p.fillRect(r, Qt::green);
        }
        else
        {
            p.fillRect(r, Qt::red);
        }

        p.drawText(x + 2, y - 12, m.name);
    }
}
