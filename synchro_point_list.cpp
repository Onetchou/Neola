#include "mainwindow.h"
#include "./ui_mainwindow.h"


void MainWindow::updateSynchroPointList()
{
    ui->synchroPointList->clear();
    for (const SynchroPoint &point : m_synchroPoints)
    {
        QString timestampString = QString::number(point.timestamp/1000.0, 'f', 3) + " s";
        QString nameString = point.name.isEmpty() ? "" : point.name + " : ";
        QString typeString = (point.type == StartPoint) ? " [Start]" : " [Stop]";

        QListWidgetItem* item = new QListWidgetItem(nameString + timestampString + typeString);
        item->setData(Qt::UserRole, QVariant::fromValue(point));

        ui->synchroPointList->addItem(item);
    }
}



void MainWindow::handleSynchroPointListItemDoubleClicked(QListWidgetItem* item)
{
    SynchroPoint point = item->data(Qt::UserRole).value<SynchroPoint>();
    changePlayerPosition(point.timestamp);
}


void MainWindow::handleSynchroPointListItemSelection()
{
    QList<QListWidgetItem*> items = ui->synchroPointList->selectedItems();
    if (items.size() == 1)
    {
        SynchroPoint point = items.first()->data(Qt::UserRole).value<SynchroPoint>();
        ui->nameEdit->setText(point.name);
    }
}
