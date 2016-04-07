#ifndef __FEATURE_COLLECTION_HPP__
#define __FEATURE_COLLECTION_HPP__

#include <string>
#include <vector>

#include "dictionary.hpp"

struct FeatureGeometry {
    virtual std::string toGeoJSON() const = 0;
};

struct Feature {
    std::string id_ ;
    Dictionary properties_ ;
    std::shared_ptr<FeatureGeometry> geometry_ ;
    std::string toGeoJSON() const ;
};

using Point = std::pair<float, float> ;
using PointList = std::vector<Point> ;

struct PointGeometry: public FeatureGeometry {
    PointGeometry(float x, float y): coordinates_(std::make_pair(x, y)) {}
    Point coordinates_ ;
    std::string toGeoJSON() const ;
};

struct LineStringGeometry: public FeatureGeometry {
    LineStringGeometry(const PointList &p): coordinates_(p) {}
    PointList coordinates_ ;
    std::string toGeoJSON() const ;
};

struct MultiPointGeometry: public FeatureGeometry {
    PointList coordinates_ ;
    std::string toGeoJSON() const ;
};

struct PolygonGeometry: public FeatureGeometry {
    std::vector<PointList> coordinates_ ;
    std::string toGeoJSON() const ;
};

struct MyltiPolygonGeometry: public FeatureGeometry {
    std::vector< std::vector<PointList> > coordinates_ ;
    std::string toGeoJSON() const ;
};

struct GeometryCollection: public FeatureGeometry {
    std::vector< std::shared_ptr<FeatureGeometry> > geometries_ ;
    std::string toGeoJSON() const ;
};

struct FeatureCollection {

    std::vector<Feature> features_ ;
    std::string toGeoJSON() const ;
};


#endif
