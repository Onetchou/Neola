#include "mainwindow.h"

#include <algorithm>

qint64 MainWindow::findNearestSynchroPoint(const qint64 posMs)
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


void MainWindow::findNextSynchroPoint(const qint64 posMs)
{
    m_nextSynchroPoint.timestamp = -1;
    m_nextSynchroPoint.name      = "";
    m_nextSynchroPoint.type      = StopPoint;

    if (m_synchroPoints.isEmpty())
    {
        return;
    }

    auto compareSynchroPointAndTimestamp = [](const SynchroPoint &sp, qint64 ts){ return sp.timestamp < ts; };
    auto it = std::lower_bound(m_synchroPoints.begin(), m_synchroPoints.end(), posMs + 1, compareSynchroPointAndTimestamp); // +1 to avoid returning the current point

    if (it != m_synchroPoints.end())
    {
        m_nextSynchroPoint.timestamp =  it->timestamp;
        m_nextSynchroPoint.name      = it->name;
        m_nextSynchroPoint.type      = it->type;
    }
}


void MainWindow::findNextStopPoint(const qint64 posMs)
{
    qint64 pos = posMs;
    while (m_nextSynchroPoint.timestamp != -1 && m_nextSynchroPoint.type != StopPoint)
    {
        findNextSynchroPoint(posMs);
        pos = m_nextSynchroPoint.timestamp;
    }
}


void MainWindow::findNextStartPoint(const qint64 posMs)
{
    qint64 pos = posMs;
    while (m_nextSynchroPoint.timestamp != -1 && m_nextSynchroPoint.type != StartPoint)
    {
        findNextSynchroPoint(pos);
        pos = m_nextSynchroPoint.timestamp;
    }
}


void MainWindow::sortSynchroPoints(SynchroPoints &points)
{
    auto compareSynchroPoints = [](const SynchroPoint &a, const SynchroPoint &b)
    {
        return a.timestamp < b.timestamp;
    };

    std::sort(points.begin(), points.end(), compareSynchroPoints);
}


void MainWindow::addSynchroPoint(const SynchroPoint &point)
{
    m_synchroPoints.append(point);
    sortSynchroPoints(m_synchroPoints);

    m_timeline->setSynchroPoint(m_synchroPoints);
    updateSynchroPointList();
}


// points must be sorted
void MainWindow::setSynchroPoints(const SynchroPoints &points)
{
    m_synchroPoints = points;

    m_timeline->setSynchroPoint(m_synchroPoints);
    updateSynchroPointList();
}


void MainWindow::updateSynchroPoint(SynchroPoints& synchroPoints, const SynchroPoint& point)
{
    for (SynchroPoint& sp : synchroPoints)
    {
        if (sp.timestamp == point.timestamp)
        {
            sp.name = point.name;
            sp.type = point.type;
        }
    }
}
