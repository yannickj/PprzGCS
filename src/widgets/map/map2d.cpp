#include "map2d.h"
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <string>
#include <iostream>
#include "point2dlatlon.h"
#include "point2dtile.h"
#include <QtXml>
#include <QFile>
#include "utils.h"

Map2D::Map2D(QWidget *parent) : QGraphicsView(parent), numericZoom(0.0), zoom(10.0), tileSize(0), minZoom(0.0), maxZoom(21.0)
{
    loadConfig("://tile_sources.xml");
    toggleTileProvider(QString("Google"), true, 0, 1);

    int maxxy = 1 << static_cast<int>(maxZoom);

    scene = new QGraphicsScene(-500, -500, tileSize*maxxy, tileSize*maxxy, parent);
    setScene(scene);

    setDragMode(QGraphicsView::ScrollHandDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setBackgroundBrush(QBrush(Qt::darkGreen));


    Point2DLatLon initLatLon(43.4625, 1.2732);

    centerLatLon(initLatLon);


}

void Map2D::toggleTileProvider(QString providerName, bool enable, int zValue, qreal opacity) {
    if(enable) {
        if(tile_providers.find(providerName) != tile_providers.end()) {
            // reactivate
            //tile_providers[providerName]->
        } else {
            // create it
            if(tileSize == 0) {
                tileSize = sourceConfigs[providerName]->tileSize;
            }
            tile_providers[providerName] = new TileProvider(*sourceConfigs[providerName], zValue, tileSize, this);
            tile_providers[providerName]->setopacity(opacity);
            connect(tile_providers[providerName], SIGNAL(displayTile(TileItem*, TileItem*)), this, SLOT(handleTile(TileItem*, TileItem*)));
            //updateTiles();
        }
    }
    else {
        if(tile_providers.find(providerName) != tile_providers.end()) {
            // desactivate it
        } else {
            throw std::runtime_error("Can't desactivate something that don't exists!");
        }
    }
}

void Map2D::centerLatLon(Point2DLatLon latLon) {
    centerOn(scenePoint(latLon, zoomLevel()));
}

void Map2D::loadConfig(QString filename) {
    QDomDocument xmlLayout;
    QFile f(filename);
    if(!f.open(QIODevice::ReadOnly)) {
        std::cout << "Error reading file " << filename.toStdString() << "!" << std::endl;
    }
    xmlLayout.setContent(&f);
    f.close();

    QDomElement root = xmlLayout.documentElement();
    QString rootTag = root.tagName();
    if (rootTag != "sources") {
        std::cout << "Root tag expected to be \"sources\". Is this a tileSource file ?" << std::endl;
    }

    for(int i=0; i<root.childNodes().length(); i++) {
        if(root.childNodes().item(i).isElement()) {
            QDomElement ele = root.childNodes().item(i).toElement();

            int zoomMin = ele.attribute("zoomMin", "0").toInt();
            int zoomMax = ele.attribute("zoomMax", "19").toInt();

            int xMin = ele.attribute("xMin", "0").toInt();
            int xMax = ele.attribute("xMax", "0").toInt();
            int yMin = ele.attribute("yMin", "0").toInt();
            int yMax = ele.attribute("xMax", "0").toInt();

            QString name = ele.attribute("name");
            sourceConfigs[name] = TileProviderConfig::builder{}.
                setName(name).
                setDir(ele.attribute("dir")).
                setAddr(ele.attribute("addr")).
                setPosZoom(ele.attribute("posZoom").toInt()).
                setPosX(ele.attribute("posX").toInt()).
                setPosY(ele.attribute("posY").toInt()).
                setZoomMin(zoomMin).
                setZoomMax(zoomMax).
                setXMin(xMin).
                setXMax(xMax).
                setYMin(yMin).
                setYMax(yMax).
                setTileSize(ele.attribute("tileSize").toInt()).
                setFormat(ele.attribute("format"))
                .newBuild();
        }
    }
}

void Map2D::resizeEvent(QResizeEvent *event){
    QGraphicsView::resizeEvent(event);
    updateTiles();
}

void Map2D::wheelEvent(QWheelEvent* event) {
    setResizeAnchor(QGraphicsView::NoAnchor);
    int curZoom = zoomLevel();

    if(event->delta() > 0) {
        zoom += 0.5;
    } else {
        zoom -= 0.5;
    }
    zoom = clamp(zoom, minZoom, maxZoom);

    // save initial numericZoom
    double  numZoomIni = numericZoom;

    // for tileProvider in tileProviders...
    int nextZoomLevel = zoomLevel();
    numericZoom = zoom - nextZoomLevel;

    double scaleFactor = pow(2, numericZoom) / pow(2, numZoomIni);

    // mouse pos in scene
    QPointF oldPos = mapToScene(event->pos());
    // lat lon point pointed by the mouse (at the current zoomLevel)
    Point2DLatLon poi(tilePoint(oldPos, curZoom));

    scale(scaleFactor, scaleFactor);    // apply scale
    // mouse pos in scene after scale
    QPointF newPos = mapToScene(event->pos());
    // position of the poi in scene coordinates for the new zoom
    QPointF poi_scene = scenePoint(poi, nextZoomLevel);

    QPointF delta = newPos - poi_scene;
    translate(delta.x(), delta.y());

    for(auto elt: tile_providers) {
        elt.second->setZoomLevel(nextZoomLevel);
    }

    updateTiles();
}

void Map2D::mouseMoveEvent(QMouseEvent *event) {
    QGraphicsView::mouseMoveEvent(event);
    if(event->buttons() & Qt::LeftButton) {
        updateTiles();
    }
}

void Map2D::updateTiles() {
    QPointF center = mapToScene(QPoint(width()/2,height()/2));

    int xCenter = static_cast<int>(center.x()/tileSize);
    int yCenter = static_cast<int>(center.y()/tileSize);
    int N = std::max(width(), height()) / (tileSize);

    for(auto elt: tile_providers) {

        Point2DTile coor = Point2DTile(xCenter, yCenter, zoomLevel());
        elt.second->fetch_tile(coor, coor);

        for(int n=0; n<N+1; n++) {
            for(int i=-n; i<=n; i++) {
                coor = Point2DTile(xCenter + i, yCenter + n, zoomLevel());
                elt.second->fetch_tile(coor, coor);
                coor = Point2DTile(xCenter + i, yCenter - n, zoomLevel());
                elt.second->fetch_tile(coor, coor);
            }
            for(int j=1-n; j<n; j++) {
                coor = Point2DTile(xCenter + n, yCenter + j, zoomLevel());
                elt.second->fetch_tile(coor, coor);
                coor = Point2DTile(xCenter - n, yCenter + j, zoomLevel());
                elt.second->fetch_tile(coor, coor);
            }
        }
    }
}

void Map2D::handleTile(TileItem* tileReady, TileItem* tileObj) {
    if(tileReady->hasData()){
        if(!tileObj->isInScene()) {    // Not in scene, so lets add it
            scene->addItem(tileObj);
            tileObj->setInScene(true);
        }
        if(!tileObj->isVisible()) {    // in scene but hidden, lets show it. TODO: what if this slot is called just atfer a zoom change ?
            if(tileObj->coordinates().zoom() == zoomLevel()) {
                tileObj->show();
            }
        }

        QPointF pos = QPointF(
            tileSize*(tileObj->coordinates().x()),
            tileSize*(tileObj->coordinates().y())
        );
        tileObj->setPos(pos);
    } else {
        std::cout << "WHAAAAT ? Why I am receiving this signal but I have no data ?" << std::endl;
    }
}

Point2DTile Map2D::tilePoint(QPointF scenePos, int zoom) {
    return Point2DTile(scenePos.x()/tileSize, scenePos.y()/tileSize, zoom);
}

QPointF Map2D::scenePoint(Point2DTile tilePoint) {
    return QPointF(tilePoint.x()*tileSize, tilePoint.y()*tileSize);
}

QPointF Map2D::scenePoint(Point2DLatLon latlon, int zoomLvl) {
    Point2DTile tile_pos = Point2DTile(latlon, zoomLvl);
    return scenePoint(tile_pos);
}
