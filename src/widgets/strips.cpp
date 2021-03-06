#include "strips.h"
#include "dispatcher_ui.h"
#include "AircraftManager.h"
#include "strip.h"

Strips::Strips(QWidget *parent) : QWidget(parent)
{
    auto scroll = new QScrollArea(this);
    auto scroll_content = new QWidget(scroll);
    scroll_layout = new QVBoxLayout(scroll_content);
    auto main_layout  = new QVBoxLayout(this);
    main_layout->addWidget(scroll);
    scroll->setWidget(scroll_content);
    scroll->setWidgetResizable(true);
    scroll->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    scroll_layout->setAlignment(Qt::AlignRight);
    connect(DispatcherUi::get(), &DispatcherUi::new_ac_config, this, &Strips::handleNewAC);
    connect(DispatcherUi::get(), &DispatcherUi::ac_deleted, this, &Strips::removeAC);
}

void Strips::handleNewAC(QString ac_id) {
    auto ac = AircraftManager::get()->getAircraft(ac_id);

    Strip* pageWidget = new Strip(ac_id, this);
    strips[ac_id] = pageWidget;

    scroll_layout->addWidget(pageWidget);
}

void Strips::removeAC(QString ac_id) {
    scroll_layout->removeWidget(strips[ac_id]);
    strips[ac_id]->deleteLater();
    strips.remove(ac_id);
}
