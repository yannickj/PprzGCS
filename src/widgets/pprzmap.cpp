#include "pprzmap.h"
#include "ui_pprzmap.h"
#include "pprz_dispatcher.h"
#include "dispatcher_ui.h"
#include <iostream>
#include <QStandardItemModel>
#include <QCheckBox>
#include <QMenu>
#include <QAction>
#include "mapscene.h"
#include <QGraphicsSceneMouseEvent>
#include "waypointitem.h"
#include "circleitem.h"
#include "maputils.h"
#include "path.h"
#include "smwaypointitem.h"
#include "smcircleitem.h"
#include "smpathitem.h"
#include <QCursor>

PprzMap::PprzMap(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PprzMap),
    drawState(false),
    interaction_state(PMIS_OTHER), fp_edit_sm(nullptr)
{
    ui->setupUi(this);
    MapScene* scene = static_cast<MapScene*>(ui->map->scene());

    connect(ui->map, &MapWidget::itemRemoved,
        [=](MapItem* item) {
            //remove this item from everywhere, including dependencies
            //if nobody know it recursively, delete it !
            delete item;
        });

    connect(ui->map, &MapWidget::itemAdded,
        [=](MapItem* map_item) {
            if(map_item->getType() == ITEM_WAYPOINT) {
                registerWaypoint(dynamic_cast<WaypointItem*>(map_item));
            }
        });

    connect(scene, &MapScene::eventScene,
        [=](FPEditEvent eventType, QGraphicsSceneMouseEvent *mouseEvent) {
            if(interaction_state == PMIS_FLIGHT_PLAN_EDIT && fp_edit_sm != nullptr) {
                MapItem* item = fp_edit_sm->update(eventType, mouseEvent, nullptr, Qt::green);
                (void)item; //put item in a list relative to the drone (in a drone FP, in a block)
            }
        });

}

void PprzMap::registerWaypoint(WaypointItem* waypoint) {
    connect(waypoint, &WaypointItem::itemClicked,
        [=](QPointF pos) {
            (void)pos;
            if(interaction_state == PMIS_FLIGHT_PLAN_EDIT && fp_edit_sm != nullptr) {
                MapItem* item = fp_edit_sm->update(FPEE_WP_CLICKED, nullptr, waypoint, Qt::green);
                (void)item; //put item in a list relative to the drone (in a drone FP, in a block)
            }
        });
}

PprzMap::~PprzMap()
{
    delete ui;
}


void PprzMap::keyPressEvent(QKeyEvent *event) {
    (void)event;
}

void PprzMap::keyReleaseEvent(QKeyEvent *event) {
    (void)event;
    if(event->key() == Qt::Key_Space) {
        interaction_state = PMIS_FLIGHT_PLAN_EDIT;
        switch (drawState) {
        case 0:
            fp_edit_sm = new SmWaypointItem(ui->map);
            ui->map->setCursor(QCursor(QPixmap(":/pictures/cursor_waypoint_black.svg"),0, 0));
            break;
        case 1:
            fp_edit_sm = new SmCircleItem(ui->map);
            ui->map->setCursor(QCursor(QPixmap(":/pictures/cursor_circle_black.svg"),0, 0));
            break;
        case 2:
            fp_edit_sm = new SmPathItem(ui->map);
            ui->map->setCursor(QCursor(QPixmap(":/pictures/cursor_path_black.svg"),0, 0));
            break;
        default:
            break;
        }
        drawState = (drawState + 1) % 3;
    }
    else if(event->key() == Qt::Key_Escape) {
        if(interaction_state == PMIS_FLIGHT_PLAN_EDIT && fp_edit_sm != nullptr) {
            MapItem* item = fp_edit_sm->update(FPEE_CANCEL, nullptr, nullptr, Qt::green);
            (void)item; //put item in a list relative to the drone (in a drone FP, in a block)
        }

        ui->map->setMouseTracking(false);
        ui->map->scene()->setShortcutItems(false);
        interaction_state = PMIS_OTHER;
        drawState = 0;
        ui->map->setCursor(Qt::ArrowCursor);
    }
    else if (event->key() == Qt::Key_H) {
        for(auto mp: items) {
            mp->setHighlighted(false);
        }
    }
}
