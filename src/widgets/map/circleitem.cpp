#include "circleitem.h"
#include <QPen>
#include "math.h"
#include <QApplication>
#include <QDebug>
#include "maputils.h"
#include <QGraphicsScene>
#include "mapwidget.h"

CircleItem::CircleItem(Point2DLatLon pt, double radius, QColor color, int tile_size, double zoom, qreal z_value, MapWidget* map, double neutral_scale_zoom, QObject *parent) :
    MapItem(zoom, tile_size, z_value, map, neutral_scale_zoom, parent),
    _radius(radius), stroke(5)
{
    center = new WaypointItem(pt, 20, color, tile_size, zoom, z_value, map, neutral_scale_zoom, parent);
    center->setZoomFactor(1.1);
    init(center, radius, color, tile_size, map);
}

CircleItem::CircleItem(WaypointItem* center, double radius, QColor color, int tile_size, double zoom, qreal z_value, MapWidget* map, double neutral_scale_zoom):
  MapItem(zoom, tile_size, z_value, map, neutral_scale_zoom),
  center(center), _radius(radius), stroke(5)
{
    init(center, radius, color, tile_size, map);
}

void CircleItem::init(WaypointItem* center, double radius, QColor color, int tile_size, MapWidget* map) {
    double pixelRadius = distMeters2Tile(radius, center->position().lat(), zoomLevel(_zoom)) * tile_size;
    circle = new GraphicsCircle(pixelRadius, QPen(QBrush(color), stroke), this);
    circle->setPos(center->scenePos());
    circle->setZValue(center->zValue() + 0.5);

    QList<QColor> color_variants = makeColorVariants(color);
    circle->setColors(color_variants[0], color_variants[1], color_variants[2]);

    map->scene()->addItem(circle);

    connect(
        center, &WaypointItem::waypointMoved,
        [=](Point2DLatLon latlon) {
            QPointF p = scenePoint(latlon, zoomLevel(_zoom), tile_size);
            circle->setPos(p);
            emit(circleMoved(latlon));
        }
    );

    connect(
        circle, &GraphicsCircle::circleScaled,
        [=](qreal size) {
            _radius = distTile2Meters(circle->pos().y()/tile_size, size/tile_size, zoomLevel(_zoom));
            emit(circleScaled(_radius));
        }
    );

    connect(
        circle, &GraphicsCircle::objectClicked,
        [=](QPointF scene_pos) {
            qDebug() << "circle clicked at " << scene_pos;
        }
    );

    connect(
        circle, &GraphicsCircle::objectGainedHighlight,
        [=]() {
            setHighlighted(true);
            emit(itemGainedHighlight());
        }
    );

    connect(
        center, &MapItem::itemGainedHighlight,
        [=]() {
            setHighlighted(true);
            emit(itemGainedHighlight());
        }
    );

    map->addItem(this);
}




void CircleItem::setHighlighted(bool h) {
    highlighted = h;
    center->setHighlighted(h);
    circle->setHighlighted(h);
}

void CircleItem::setZValue(qreal z) {
    z_value = z;
    //the circle is above the waypoint
    center->setZValue(z-0.5);
    circle->setZValue(z);
}

void CircleItem::updateGraphics() {
    double pixelRadius = distMeters2Tile(_radius, center->position().lat(), zoomLevel(_zoom))*tile_size;

    QPointF scene_pos = scenePoint(center->position(), zoomLevel(_zoom), tile_size);
    circle->setPos(scene_pos);
    circle->setRadius(pixelRadius);

    double s = getScale();

    QPen p = circle->pen();
    p.setWidth(static_cast<int>(stroke * s));
    circle->setPen(p);
}
