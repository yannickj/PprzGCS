#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <QObject>
#include <QColor>
#include "flightplan.h"
#include "setting_menu.h"
#include "airframe.h"
#include "point2dlatlon.h"
#include "pprz_dispatcher.h"
#include "aircraft_status.h"

class Aircraft
{
public:
    Aircraft();
    Aircraft(QString id, QColor color, QString icon, QString name, FlightPlan fp, shared_ptr<SettingMenu> setting_menu, Airframe air);
    ~Aircraft();

    QColor getColor(){return color;}
    QString getId(){return ac_id;}
    QString getIcon(){return icon;}
    QString name() {return _name;}
    FlightPlan& getFlightPlan() {return flight_plan;}
    //SettingMenu& getSettingMenu() {return setting_menu;}
    shared_ptr<SettingMenu> getSettingMenu() {return setting_menu;}
    Airframe& getAirframe() {return airframe;}
    AircraftStatus* getStatus() {return status;}

    Point2DLatLon getPosition() {return position;}
    void setPosition(Point2DLatLon pos) {position = pos;}

    void setSetting(shared_ptr<Setting>, float value);
    void setSetting(uint8_t setting_no, float value);

private:
    QString ac_id;
    QColor color;
    QString icon;
    QString _name;
    FlightPlan flight_plan;
    shared_ptr<SettingMenu> setting_menu;
    Airframe airframe;

    Point2DLatLon position;

    AircraftStatus* status;


};

#endif // AIRCRAFT_H
