#include "waypoint.h"
#include <math.h>
#include <iostream>
#include "coordinatestransform.h"
#include "gcs_utils.h"

using namespace tinyxml2;

Waypoint::Waypoint(string name, uint8_t id) :
    type(ABSOLUTE), id(id), lat(0), lon(0), origin(nullptr), alt(0), name(name)
{

}

Waypoint::Waypoint(string name, uint8_t id, double lat, double lon, double alt):
    Waypoint(name, id)
{
    type = ABSOLUTE;
    this->lat = lat;
    this->lon = lon;
    this->alt = alt;
    alt_type = ALT;
}

Waypoint::Waypoint(string name, uint8_t id, double lat, double lon, double alt, shared_ptr<Waypoint> orig, WpAltType altType):
    Waypoint(name, id)
{
    type = RELATIVE;
    this->alt = alt;
    this->lat = lat;
    this->lon = lon;
    origin = orig;
    this->alt_type = altType;
}


Waypoint::Waypoint(XMLElement* wp, uint8_t wp_id, shared_ptr<Waypoint> orig, double defaultAlt) {
    const char* name_p = wp->Attribute("name");
    const char* lat_p = wp->Attribute("lat");
    const char* lon_p = wp->Attribute("lon");
    const char* x_p = wp->Attribute("x");
    const char* y_p = wp->Attribute("y");
    const char* alt_p = wp->Attribute("alt");
    const char* height_p = wp->Attribute("height");

    Waypoint::WpAltType altType = Waypoint::WpAltType::ALT;
    double alt;
    if(height_p != nullptr) {
        altType = Waypoint::WpAltType::HEIGHT;
        alt = stod(height_p);
    } else if(alt_p != nullptr) {
        alt = stod(alt_p);
    } else {
        alt = defaultAlt;
    }

    name = name_p;
    id = wp_id;

    if(lat_p != nullptr && lon_p != nullptr) {

        type = ABSOLUTE;
        this->lat = parse_coordinate(lat_p);
        this->lon = parse_coordinate(lon_p);
        this->alt = alt;
        alt_type = ALT;

        if(altType == Waypoint::WpAltType::HEIGHT) {
            throw runtime_error("Unimplemented! Can't (yet) create absolute waypoint with relative height!");
        }
    }
    else if(x_p !=nullptr && y_p != nullptr) {
        CoordinatesTransform::get()->relative_to_wgs84(orig->getLat(), orig->getLon(), stod(x_p), stod(y_p), &this->lat, &this->lon);

        type = RELATIVE;
        this->alt = alt;
        this->origin = orig;
        this->alt_type = altType;
    } else {
        throw runtime_error("You must specify either x/y or lat/lon!");
    }

    for(auto att=wp->FirstAttribute(); att != nullptr; att=att->Next()) {
        xml_attibutes[att->Name()] = att->Value();
    }
}


double Waypoint::getLon() const {
    return lon;
}

double Waypoint::getLat() const {
    return lat;
}

void Waypoint::setLat(double lat) {
        this->lat = lat;

}

void Waypoint::setLon(double lon) {
    this->lon = lon;
}


ostream& operator<<(ostream& os, const Waypoint& wp) {

    os << wp.name << " : ";
    if(wp.type == Waypoint::ABSOLUTE) {
        os << wp.lat << "N, " << wp.lon << "E";
    } else {
        os << "not implemented!";//<< wp.x << "m, " << wp.y << "m";
    }

    os << ", " << wp.alt;

    if(wp.alt_type == Waypoint::ALT) {
        os << "m AMSL";
    } else {
        os << "m AGL";
    }

    return os;
}


