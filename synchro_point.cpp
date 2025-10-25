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
    m_nextSynchroPoint.id        = -1;

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
        m_nextSynchroPoint.id        = it->id;
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


SynchroPoint MainWindow::findPreviousSynchroPoint(const qint64 posMs)
{
    SynchroPoint previousSynchroPoint;
    previousSynchroPoint.timestamp = -1;
    previousSynchroPoint.name      = "";
    previousSynchroPoint.type      = StopPoint;
    previousSynchroPoint.id        = -1;

    if (m_synchroPoints.isEmpty())
    {
        return previousSynchroPoint;
    }

    auto it = std::find_if(m_synchroPoints.rbegin(), m_synchroPoints.rend(), [posMs](const SynchroPoint &sp) { return sp.timestamp < posMs; });

    if (it != m_synchroPoints.rend())
    {
        previousSynchroPoint.timestamp = it->timestamp;
        previousSynchroPoint.name      = it->name;
        previousSynchroPoint.type      = it->type;
        previousSynchroPoint.id        = it->id;
    }

    return previousSynchroPoint;
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

    m_timeline->setSynchroPoints(m_synchroPoints);
    updateSynchroPointList();
}


// points must be sorted
void MainWindow::setSynchroPoints(const SynchroPoints &points)
{
    m_synchroPoints = points;

    m_timeline->setSynchroPoints(m_synchroPoints);
    updateSynchroPointList();
}


void MainWindow::updateSynchroPoint(SynchroPoints& synchroPoints, const SynchroPoint& point)
{
    for (SynchroPoint& sp : synchroPoints)
    {
        if (sp.id == point.id)
        {
            sp.name = point.name;
            sp.type = point.type;
            sp.timestamp = point.timestamp;
        }
    }
}


int MainWindow::getNewId()
{
    // Find the biggest id
    int bestId = 0;
    for (const SynchroPoint &sp : m_synchroPoints)
    {
        if (sp.id > bestId)
        {
            bestId = sp.id;
        }
    }

    return bestId + 1;
}
