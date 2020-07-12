#include "distancebetweenlonandlat.h"

DistanceBetweenLonAndLat::DistanceBetweenLonAndLat()
{

}


DistanceBetweenLonAndLat::~DistanceBetweenLonAndLat()
{

}

double DistanceBetweenLonAndLat::HaverSin(double thera)
{
    double v = sin(thera / 2);
    return v * v;
}

//角度转换为弧度
double DistanceBetweenLonAndLat::ConvertDegreesToRadians(double degrees)
{
    return degrees * M_PI / 180;
}

//弧度转换为角度
double DistanceBetweenLonAndLat::ConvertRadiansToDegrees(double radian)
{
    return radian * 180.0 / M_PI;
}

//计算两个经纬度坐标间的距离(单位：米)
//参数：lon1为经度1；lat1为纬度1；lon2为经度2；lat2为纬度2
double DistanceBetweenLonAndLat::get_distance(double lon1, double lat1, double lon2, double lat2)
{
    lat1 = ConvertDegreesToRadians(lat1);
    lon1 = ConvertDegreesToRadians(lon1);
    lat2 = ConvertDegreesToRadians(lat2);
    lon2 = ConvertDegreesToRadians(lon2);

    double vLon = fabs(lon1 - lon2);
    double vLat = fabs(lat1 - lat2);
    double h = HaverSin(vLat) + cos(lat1) * cos(lat2) * HaverSin(vLon);
    double distance = 2 * EARTH_RADIUS * asin(sqrt(h));

    return distance * 1000;
}

//两个经纬度间连线与正北方向的夹角
//参数：lon1为经度1；lat1为纬度1；lon2为经度2；lat2为纬度
double DistanceBetweenLonAndLat::get_angle(double lon1, double lat1, double lon2, double lat2)
{
    double x = lat1 - lat2;
    double y = lon1 - lon2;
    double angle = -1;
    if (y == 0 && x > 0)
        angle = 0;
    if (y == 0 && x < 0)
        angle = 180;
    if (x == 0 && y > 0)
        angle = 90;
    if (x == 0 && y < 0)
        angle = 270;
    if (angle == -1)
    {
        double dislon = get_distance(lon1, lat2, lon2, lat2);
        double dislat = get_distance(lon2, lat1, lon2, lat2);
        if (x > 0 && y > 0)
            angle = atan2(dislon, dislat) / M_PI * 180;
        if (x < 0 && y > 0)
            angle = atan2(dislat, dislon) / M_PI * 180 + 90;
        if (x < 0 && y < 0)
            angle = atan2(dislon, dislat) / M_PI * 180 + 180;
        if (x > 0 && y < 0)
            angle = atan2(dislat, dislon) / M_PI * 180 + 270;
    }
    return angle;
}

//将角度限制在[0,360),0--北，90--东，180--南，270--西；与指南针一致
double DistanceBetweenLonAndLat::AngleSpecification(double angle)
{
    double curAngle;
    if (angle < 0)
    {
        curAngle = fmod((angle + 360), 360);
    }
    else {
        curAngle = fmod(angle, 360);
    }
    return curAngle;
}
