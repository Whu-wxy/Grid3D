#ifndef DISTANCEBETWEENLONANDLAT_H
#define DISTANCEBETWEENLONANDLAT_H

#include <math.h>
#define M_PI 3.14159265358979323846
#define EARTH_RADIUS 6371.0

//Haversine formula半正矢量公式来计算球面上任意两个坐标间的距离
//vincenty公式  精度很高能达到0.5毫米，但是很慢

class DistanceBetweenLonAndLat
{
public:
    DistanceBetweenLonAndLat();
    ~DistanceBetweenLonAndLat();

    double get_distance(double lon1, double lat1, double lon2, double lat2);
    double get_angle(double lon1, double lat1, double lon2, double lat2);
    double AngleSpecification(double angle);


protected:
    double HaverSin(double thera);
    double ConvertDegreesToRadians(double degrees);
    double ConvertRadiansToDegrees(double radian);

};

#endif // DISTANCEBETWEENLONANDLAT_H
