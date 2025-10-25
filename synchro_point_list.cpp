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
    SynchroPoint point = getSelectedSynchroPoint();
    if (point.timestamp != -1)
    {
        m_selectedSynchroPoint = point;

        ui->nameEdit->blockSignals(true);
        ui->nameEdit->setText(point.name);
        ui->nameEdit->blockSignals(false);

        ui->timestampSpinBox->blockSignals(true);
        ui->timestampSpinBox->setValue(point.timestamp);
        ui->timestampSpinBox->blockSignals(false);
    }
}


SynchroPoint MainWindow::getSelectedSynchroPoint()
{
    SynchroPoint point;
    point.timestamp = -1;
    point.name      = "";
    point.type      = StopPoint;

    QList<QListWidgetItem*> selected_items = ui->synchroPointList->selectedItems();
    if (selected_items.size() == 1)
    {
        point = selected_items.first()->data(Qt::UserRole).value<SynchroPoint>();
    }

    return point;
}
