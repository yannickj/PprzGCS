#ifndef CIRCLEITEM_H
#define CIRCLEITEM_H

#include "map_item.h"
#include "waypoint_item.h"
#include "graphics_circle.h"

class MapWidget;

class CircleItem : public MapItem
{
    Q_OBJECT
public:
    explicit CircleItem(WaypointItem* center, double radius, QString ac_id, qreal z_value, double neutral_scale_zoom = 15);
    WaypointItem* getCenter() { return center;}
    GraphicsCircle* getGraphicsCircle() {return circle;}
    virtual void addToMap(MapWidget* mw);
    virtual void setHighlighted(bool h);
    void setForbidHighlight(bool fh);
    virtual void setEditable(bool ed);
    virtual void setZValue(qreal z);
    virtual void updateGraphics(double zoom, double scale_factor, int tile_size);
    virtual void removeFromScene(MapWidget* map);
    virtual ItemType getType() {return ITEM_CIRCLE;}
    double radius() {return _radius;}
    ///
    /// \brief setRadius
    /// \param radius in meters
    ///
    void setRadius(double radius);

    void setStyle(GraphicsCircle::Style s);

signals:
    void circleMoved(Point2DLatLon latlon_pos);
    void circleScaled(double radius);

protected:

private:
    void init(WaypointItem* center);
    WaypointItem* center;
    GraphicsCircle* circle;
    double _radius;
    int altitude;
    int stroke;

    bool highlighted;
};

#endif // CIRCLEITEM_H
