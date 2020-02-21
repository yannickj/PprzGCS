#include "mapitem.h"
#include "math.h"
#include <QApplication>
#include <QDebug>

MapItem::MapItem(double zoom, int tile_size, qreal z_value, MapWidget* map, double neutral_scale_zoom, QObject *parent) :
    QObject(parent),
    zoom_factor(1), neutral_scale_zoom(neutral_scale_zoom),
    tile_size(tile_size), _zoom(zoom), _view_scale(1), z_value(z_value),
    map(map)
{
}


QList<QColor> MapItem::makeColorVariants(QColor color) {
    QList<QColor> list;

    int h, s, v, a;
    color.getHsv(&h, &s, &v, &a);

    int v1 = qMin(static_cast<int>(v/1.2), 255);
    QColor c1 = QColor(color);
    c1.setHsv(h, s, v1, a);
    list.append(c1);

    int a2 = qMin(static_cast<int>(a/2), 255);
    QColor c2 = QColor(color);
    c2.setHsv(h, s, v, a2);
    list.append(c2);


    int s3 = static_cast<int>(s/2);
    QColor c3 = QColor(color);
    //c3.setHsv(h, s3, v, a);
    c3.setHsv(h, s3, v, a2);
    list.append(c3);

    return list;
}

void MapItem::scaleToZoom(double zoom, double viewScale) {
    _zoom = zoom;
    _view_scale = viewScale;
    updateGraphics();
}

double MapItem::getScale() {
    return pow(zoom_factor, _zoom - neutral_scale_zoom)/_view_scale;
}
