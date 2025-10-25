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
    m_nextSynchroPoint = -1;
    if (m_synchroPoints.isEmpty())
    {
        return;
    }

    auto compareSynchroPointAndTimestamp = [](const SynchroPoint &sp, qint64 ts)
    {
        return sp.timestamp < ts;
    };
    auto it = std::lower_bound(m_synchroPoints.begin(), m_synchroPoints.end(), posMs + 1, compareSynchroPointAndTimestamp); // +1 to avoid returning the current point

    if (it != m_synchroPoints.end())
    {
        m_nextSynchroPoint =  it->timestamp;
    }

    m_timeline->setNextSynchroPoint(m_nextSynchroPoint);
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
