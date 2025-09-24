#pragma once

#include <cmath>

template<typename Real>
class Vector3_T {
 public:
  Vector3_T() = default;
  Vector3_T(Real x, Real y, Real z) : x(x), y(y), z(z) {};

  Vector3_T operator+(const Vector3_T& v) const {
    return Vector3_T(x + v.x, y + v.y, z + v.z);
  }

  Vector3_T operator-(const Vector3_T& v) const {
    return Vector3_T(x - v.x, y - v.y, z - v.z);
  }

  Vector3_T operator*(Real scalar) const {
    return Vector3_T(x * scalar, y * scalar, z * scalar);
  }

  Vector3_T operator*(const Vector3_T& other) const {
    return Vector3_T(x * other.x, y * other.y, z * other.z);
  }

  Vector3_T operator/(Real scalar) const {
    return Vector3_T(x / scalar, y / scalar, z / scalar);
  }

  Real length() const {
    return std::sqrt(x * x + y * y + z * z);
  }

  Vector3_T getNormalized() const {
    Real len = length();
    return Vector3_T(x / len, y / len, z / len);
  }

  Real dot(const Vector3_T& v) const {
    return x * v.x + y * v.y + z * v.z;
  }

  Vector3_T cross(const Vector3_T& v) const {
    return Vector3_T(
      y * v.z - z * v.y,
      z * v.x - x * v.z,
      x * v.y - y * v.x
    );
  }

  Real x = 0, y = 0, z = 0;
};

template<typename Real>
Vector3_T<Real> operator*(Real scalar, const Vector3_T<Real>& v) {
  Vector3_T<Real> result;
  result = v * scalar;
  return result;
}

template<typename Real>
Vector3_T<Real> operator/(Real scalar, const Vector3_T<Real>& v) {
  Vector3_T<Real> result;
  result = v / scalar;
  return result;
}

template<typename Real>
class Sphere_T {
 public: 
  Sphere_T() = default;
  Sphere_T(const Vector3_T<Real>& center, 
           Real radius, 
           Vector3_T<Real> color, 
           Real kA, Real kD, Real kS)
  : center(center), 
    radius(radius), 
    color(color), 
    coeffs(kA, kD, kS) {}
  
  /*bool intersects(const Sphere& other) const {
    Real distanceSquared = (center - other.center).dot(center - other.center);
    Real radiusSum = radius + other.radius;
    return distanceSquared <= radiusSum * radiusSum;
  }*/
  
  Vector3_T<Real> center;
  Vector3_T<Real> color;
  Vector3_T<Real> coeffs; // Ambient, Diffuse, Specular
  Real radius;
};

template<typename Real>
class Plane_T {
 public:
  Plane_T() = default;
  Plane_T(const Vector3_T<Real>& norm, 
        const Vector3_T<Real>& point, 
        const Vector3_T<Real>& color,
        Real kA, Real kD, Real kS) 
    : point(point),
      normal(norm.getNormalized()),
      color(color), 
      coeffs(kA, kD, kS) {}
  
  Vector3_T<Real> point;  // A point on the plane
  Vector3_T<Real> normal; // The normal vector of the plane
  Vector3_T<Real> color;
  Vector3_T<Real> coeffs; // Ambient, Diffuse, Specular
};

template<typename Real>
class Triangle_T {
 public:
  Triangle_T() = default;
  Triangle_T(const Vector3_T<Real>& v0, 
             const Vector3_T<Real>& v1, 
             const Vector3_T<Real>& v2,
             Vector3_T<Real> color,
             Real kA, Real kD, Real kS) 
      : v0(v0), v1(v1), v2(v2),
        normal((v1 - v0).getNormalized().cross((v2 - v0).getNormalized())),
        color(color), 
        cm((v0 + v1 + v2) / (Real)3),
        coeffs(kA, kD, kS) {}
  
  bool isInside(const Vector3_T<Real>& point) const {
    if(((v1 - v0) && (point - v0)).dot((v1 - v0) & (cm - v0)) < 0.0000001) {
      return false;
    }
    
    if(((v2 - v0) && (point - v0)).dot((v1 - v0) & (cm - v0)) < 0.0000001) {
      return false;
    }
  }
  
  Vector3_T<Real> v0, v1, v2; // Vertices of the triangle
  Vector3_T<Real> normal;
  Vector3_T<Real> cm;
  Vector3_T<Real> color;
  Vector3_T<Real> coeffs; // Ambient, Diffuse, Specular
};  

template<typename Real>
class Ray_T {
 public:
  Ray_T() = default;
  Ray_T(const Vector3_T<Real>& origin, const Vector3_T<Real>& direction) 
     : origin(origin), direction(direction.getNormalized()) {}

  Vector3_T<Real> where(Real t) const {
    return origin + direction * t;
  }


  Vector3_T<Real> origin;
  Vector3_T<Real> direction;
};


template<typename Real>
class Light_T {
 public:
  Light_T() = default;
  Light_T(const Vector3_T<float>& position, Real intensity, Real ambientIntensity)
    : position(position), intensity(intensity), ambientIntensity(ambientIntensity) {}
  
  
  Vector3_T<Real> position;
  Real intensity; 
  Real ambientIntensity;
};


#define REAL_TYPE float
using Vector3  = Vector3_T<REAL_TYPE>;
using Color    = Vector3_T<REAL_TYPE>;
using Sphere   = Sphere_T<REAL_TYPE>;
using Plane    = Plane_T<REAL_TYPE>;
using Triangle = Triangle_T<REAL_TYPE>;
using Ray      = Ray_T<REAL_TYPE>;
using Light    = Light_T<REAL_TYPE>;
