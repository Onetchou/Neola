#include "mainwindow.h"
#include "./ui_mainwindow.h"


void MainWindow::updateSynchroPointList()
{
    ui->synchroPointList->clear();
    for (const SynchroPoint &m : m_synchroPoints)
    {
        QString timestamp = QString::number(m.timestamp/1000.0, 'f', 3) + " s";
        QString name = m.name.isEmpty() ? "" : m.name + " : ";

        ui->synchroPointList->addItem(name + timestamp);
    }
}
