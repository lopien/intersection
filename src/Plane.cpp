///////////////////////////////////////////////////////////////////////////////
// Plane.cpp
// =========
// class for a 3D plane with normal vector (a,b,c) and a point (x0,y0,z0)
// ax + by + cz + d = 0,  where d = -(ax0 + by0 + cz0)
//
// NOTE:
// 1. The default plane is z = 0 (a plane on XY axis)
// 2. The distance is the length from the origin to the plane
//
// Dependencies: Vector3, Line
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-01-19
// UPDATED: 2017-06-29
///////////////////////////////////////////////////////////////////////////////


#include "Plane.h"
#include <iostream>
// плоскость по умолчанию z = 0 (плоскость по оси XY)
Plane::Plane() : normal(Vector3(0,0,1)), d(0), normalLength(1), distance(0)
{
}

Plane::Plane(float a, float b, float c, float d)
{
    set(a, b, c, d);
}

Plane::Plane(const Vector3& normal, const Vector3& point)
{
    set(normal, point);
}

// выводим себя с 4-мя коэффициентами нормализованного уравнения плоскости
// aX + bY + cZ + d = 0
// где (a, b, c) - единичный вектор нормали, d = - (ax0 + by0 + cz0)
void Plane::printSelf() const
{
    std::cout << "Plane(" << normal.x << ", " << normal.y << ", " << normal.z
            << ", " << d << ")" << std::endl;
}

void Plane::set(float a, float b, float c, float d)
{
    normal.set(a, b, c);
    this->d = d;
    // вычисление расстояния
    normalLength = sqrtf(a*a + b*b + c*c);
    distance = -d / normalLength;
}

void Plane::set(const Vector3& normal, const Vector3& point)
{
    this->normal = normal;
    normalLength = normal.length();
    d = -normal.dot(point);         // -(a*x0 + b*y0 + c*z0)
    distance = -d / normalLength;
}

// вычисляем кратчайшее расстояние от заданной точки P до плоскости
// Если расстояние отрицательное, точка находится в противоположной стороне плоскости.
// D = (a * Px + b * Py + c * Pz + d) / sqrt(a*a + b*b + c*c)
float Plane::getDistance(const Vector3& point)
{
    float dot = normal.dot(point);
    return (dot + d) / normalLength;
}

// нормаль
// делим каждый коэффициент на длину нормали
void Plane::normalize()
{
    float lengthInv = 1.0f / normalLength;
    normal *= lengthInv;
    normalLength = 1.0f;
    d *= lengthInv;
    distance = -d;
}

// находим точку пересечения
// подставляем точку на прямой в уравнение плоскости, затем решаем 
// точка на линии: (x0 + x * t, y0 + y * t, z0 + z * t)
// плоскость: a * X + b * Y + c * Z + d = 0
// a*(x0 + x*t) + b*(y0 + y*t) + c*(z0 + z*t) + d = 0
// a*x0 + a*x*t + b*y0 + b*y*t + c*z0 + c*z*t + d = 0
// (a*x + b*x + c*x)*t = -(a*x0 + b*y0 + c*z0 + d)
// t = -(a*x0 + b*y0 + c*z0 + d) / (a*x + b*x + c*x)
Vector3 Plane::intersect(const Line& line) const
{
    // из строки = p + t * v
    Vector3 p = line.getPoint();        // (x0, y0, z0)
    Vector3 v = line.getDirection();    // (x,  y,  z)

    float dot1 = normal.dot(p);         // a*x0 + b*y0 + c*z0
    float dot2 = normal.dot(v);         // a*x + b*y + c*z

    // если знаменатель = 0, пересечения нет
    if(dot2 == 0)
        return Vector3(NAN, NAN, NAN);

    // исщем t = -(a*x0 + b*y0 + c*z0 + d) / (a*x + b*y + c*z)
    float t = -(dot1 + d) / dot2;

    // находим точку пересечения
    return p + (v * t);
}

// найти линию пересечения двух плоскостей
// P1: N1 точка p + d1 = 0 (a1*X + b1*Y + c1*Z + d1 = 0)
// P2: N2 точка p + d2 = 0 (a2*X + b2*Y + c2*Z + d2 = 0)
// L: p0 + a * V, где
// V - вектор направления линии пересечения = (a1, b1, c1) x (a2, b2, c2)
// p0 - точка, которая находится на L, а также на P1 и P2
// p0 можно найти, решив линейную систему из 3-х плоскостей
// P1: N1 точка p + d1 = 0 (дано)
// P2: N2 точка p + d2 = 0 (дано)
// P3: V точка p = 0 (выбрано, где d3 = 0)
// Используем формулу пересечения трех плоскостей, чтобы найти p0;
// p0 = ((-d1 * N2 + d2 * N1) x V) / V точка V
Line Plane::intersect(const Plane& rhs) const
{
    // находим вектор направления линии пересечения
    Vector3 v = normal.cross(rhs.getNormal());

    // если |direction| = 0, две плоскости параллельны (не пересекаются)
    // возвращаем строку с NaN
    if(v.x == 0 && v.y == 0 && v.z == 0)
        return Line(Vector3(NAN, NAN, NAN), Vector3(NAN, NAN, NAN));

    // находим точку на линии, которая также находится в обеих плоскостях
    // выбираем простую плоскость, где d = 0: ax + by + cz = 0
    float dot = v.dot(v);                       // V dot V
    Vector3 n1 = rhs.getD() * normal;           // d2 * N1
    Vector3 n2 = -d * rhs.getNormal();          //-d1 * N2
    Vector3 p = (n1 + n2).cross(v) / dot;       // (d2*N1-d1*N2) X V / V dot V

    return Line(v, p);
}

//определить, пересекается ли он с линией
bool Plane::isIntersected(const Line& line) const
{
    // вектор направления линии
    Vector3 v = line.getDirection();

    // скалярное произведение с нормалью к плоскости
    float dot = normal.dot(v);  // a*Vx + b*Vy + c*Vz

    if(dot == 0)
        return false;
    else
        return true;
}

// определить, пересекается ли он с другой плоскостью
bool Plane::isIntersected(const Plane& plane) const
{
    // check if 2 plane normals are same direction
    // проверяем, совпадают ли направления нормали 2 плоскостей
    Vector3 cross = normal.cross(plane.getNormal());
    if(cross.x == 0 && cross.y == 0 && cross.z == 0)
        return false;
    else
        return true;
}
