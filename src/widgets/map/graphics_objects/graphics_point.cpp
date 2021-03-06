#include "graphics_point.h"
#include "math.h"
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>

GraphicsPoint::GraphicsPoint(int size, QColor color, QObject *parent) :
    GraphicsObject(parent),
    QGraphicsItem (),
    halfSize(size), move_state(PMS_IDLE), current_color(nullptr), ignore_events(false), style(DEFAULT)
{
    color_idle = color;
    current_color = &color_idle;
}

void GraphicsPoint::setColors(QColor colPressed, QColor colMoving, QColor colUnfocused) {
    color_pressed = colPressed;
    color_moved = colMoving;
    color_unfocused = colUnfocused;
}


QRectF GraphicsPoint::boundingRect() const {
    return QRectF(-halfSize-1, -halfSize-1, 2*halfSize+2, 2*halfSize+2);
}


void GraphicsPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    (void)option;
    (void)widget;


    if(style == DEFAULT) {
        double fx = 0.8;
        double fy = 1.0;
        if(!isHighlighted()) {
            current_color = &color_unfocused;
            fx /= qApp->property("SIZE_HIGHLIGHT_FACTOR").toDouble();
            fy /= qApp->property("SIZE_HIGHLIGHT_FACTOR").toDouble();
        }

        QPolygonF poly;
        poly.append(QPointF(0, halfSize*fy));
        poly.append(QPointF(halfSize*fx, 0));
        poly.append(QPointF(0, -halfSize*fy));
        poly.append(QPointF(-halfSize*fx, 0));
        poly.append(QPointF(0, halfSize*fy));

        QPainterPath path;
        path.addPolygon(poly);

        painter->setBrush(QBrush(*current_color));
        //painter->setPen(Qt::NoPen);
        painter->drawPath(path);

    } else if (style == CARROT) {
        QPolygonF poly;
        poly.append(QPointF(halfSize*cos(M_PI/6), -halfSize*sin(M_PI/6)));
        poly.append(QPointF(halfSize*cos(5*M_PI/6), -halfSize*sin(5*M_PI/6)));
        poly.append(QPointF(halfSize*cos(3*M_PI/2), -halfSize*sin(3*M_PI/2)));

        QPainterPath path;
        path.addPolygon(poly);
        painter->setBrush(QBrush(QColor(255, 170, 0)));
        painter->setPen(Qt::NoPen);
        painter->drawPath(path);
    } else if (style == CURRENT_NAV) {
        //draw nothing
    }
}

QPainterPath GraphicsPoint::shape() const {
    if(style == CURRENT_NAV || style == CARROT) {
        QPainterPath path;
        return path;
    } else {
        return QGraphicsItem::shape();
    }
}

void GraphicsPoint::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if(ignore_events) {
        event->ignore();
        return;
    }
    GraphicsObject::mousePressEvent(event);
    pressPos = QPointF(event->pos().x() * scale(), event->pos().y() * scale());
    move_state = PMS_PRESSED;
    changeFocus();
}

void GraphicsPoint::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if(ignore_events) {
        event->ignore();
        return;
    }
    if(move_state == PMS_PRESSED) {
        QPointF dp = event->pos() - pressPos;
        double d = sqrt(dp.x()*dp.x() + dp.y()*dp.y());
        if(d > qApp->property("MAP_MOVE_HYSTERESIS").toInt() && editable) {
            move_state = PMS_MOVED;
            changeFocus();
        }
    } else if(move_state == PMS_MOVED) {
        setPos(event->scenePos() - pressPos);
        emit(pointMoved(event->scenePos() - pressPos));
    }
}

void GraphicsPoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if(ignore_events) {
        event->ignore();
        return;
    }
    if(move_state == PMS_PRESSED) {
        emit(objectClicked(event->scenePos()));
    }
    else if(move_state == PMS_MOVED) {
        emit(pointMoveFinished(event->scenePos() - pressPos));
    }
    move_state = PMS_IDLE;
    changeFocus();
}

void GraphicsPoint::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if(ignore_events) {
        event->ignore();
        return;
    }
    emit(objectDoubleClicked(event->scenePos()));
}

void GraphicsPoint::changeFocus() {
    if(!isHighlighted()) {
        current_color = &color_unfocused;
    } else {
        switch (move_state) {
        case PMS_IDLE:
            current_color = &color_idle;
            break;
        case PMS_PRESSED:
            current_color = &color_pressed;
            break;
        case PMS_MOVED:
            current_color = &color_moved;
            break;
        }
    }
    update();
}
