#include "Image.h"
#include "MathHelpers.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdlib>


REAL_TYPE kA = (REAL_TYPE)0.3;
REAL_TYPE kD = (REAL_TYPE)0.4;
REAL_TYPE kS = (REAL_TYPE)0.3;

int MAX_DEPTH = 3;
const int AASamples = 6;

Vector<Vector3> findClosestIntersection(const Ray& ray, 
                                    const Vector<Sphere>& spheres,
                                    const Vector<Plane>& planes) {
  Vector<Vector3> result;
  int closestSphereIndex = 0;
  int closestPlaneIndex = 0;
  
  REAL_TYPE smallestSolutionSphere = 50000;
  REAL_TYPE smallestSolutionPlane = 50000;

  //at^2 + bt + c = 0
  size_t count = spheres.size();
  Vector<REAL_TYPE> solutions;
  Vector<size_t> index;
  
  for(size_t i = 0; i < count; ++i) {
    REAL_TYPE a = ray.direction.dot(ray.direction);
    REAL_TYPE b = 2 * ray.direction.dot(ray.origin - spheres[i].center);
    REAL_TYPE c = (ray.origin - spheres[i].center).dot(ray.origin - spheres[i].center) - spheres[i].radius * spheres[i].radius;
    REAL_TYPE discriminant = b * b - 4 * a * c;

    if(discriminant < 0) {
      continue;
    }

    REAL_TYPE sqrtDiscriminant = std::sqrt(discriminant);
    REAL_TYPE t1 = (-b - sqrtDiscriminant) / (2 * a);
    if(t1 > 0.001 && t1 < 50000) {
      solutions.push_back(t1);
      index.push_back(i);
    }
    else {
      REAL_TYPE t2 = (-b + sqrtDiscriminant) / (2 * a);
      if(t2 > 0.001 && t2 < 50000) {
        solutions.push_back(t2);
        index.push_back(i);
      }
    }
  }

  //Find closest t for spheres
  if(!solutions.empty()) {
    REAL_TYPE tmp = solutions[0];
    int j = 0;
    for(size_t i = 1; i < solutions.size(); ++i) {
      if(solutions[i] < tmp) {
        tmp = solutions[i];
        j = (int)i;
      }
    }
    closestSphereIndex = (int)index[j];
    smallestSolutionSphere = solutions[j];
  }

  //Planes
  count = planes.size();
  solutions.clear();
  index.clear();
  for(size_t i = 0; i < count; ++i) {
    REAL_TYPE a = planes[i].normal.dot(planes[i].point - ray.origin);
    REAL_TYPE b = planes[i].normal.dot(ray.direction);
    REAL_TYPE t = a/b;
    if(t > 0.001 && t < 50000) {
      solutions.push_back(t);
      index.push_back(i);
    }
  }
  
  //Find closest t for planes
  if(!solutions.empty()) {
    REAL_TYPE tmp = solutions[0];
    int j = 0;
    for(size_t i = 1; i < solutions.size(); ++i) {
      if(solutions[i] < tmp) {
        tmp = solutions[i];
        j = (int)i;
      }
    }
    closestPlaneIndex = (int)index[j];
    smallestSolutionPlane = solutions[j];
  }
  if(smallestSolutionPlane < smallestSolutionSphere) {
    result.push_back(ray.where(smallestSolutionPlane)); //intersection point in surface
    result.push_back(planes[closestPlaneIndex].normal);  //normal in intersection point
    result.push_back(planes[closestPlaneIndex].color);   //color of the object
    result.push_back(planes[closestPlaneIndex].coeffs);  //coefficients of the object
    result.push_back(Vector3(-1, 0, 0));                 //indicate that it is a plane
  }
  else if(smallestSolutionSphere < smallestSolutionPlane) {
    Vector3 intersectionPoint = ray.where(smallestSolutionSphere);
    Vector3 normal = (intersectionPoint - spheres[closestSphereIndex].center).getNormalized();
    result.push_back(intersectionPoint);                 //intersection point in surface
    result.push_back(normal);                            //normal in intersection point
    result.push_back(spheres[closestSphereIndex].color); //color of the object
    result.push_back(spheres[closestSphereIndex].coeffs);//coefficients of the object
    result.push_back(Vector3(1, 0, 0));                  //indicate that it is a sphere
  }

  //02:18:00

  return result;
}

const Color WHITE(255, 255, 255);
const REAL_TYPE M_PI = (REAL_TYPE)3.14159265358979323846;

bool isInShadow(const Vector3& point,
                const Light& light,
                const Vector<Sphere>& spheres,
                const Vector<Plane>& planes) {
  Ray lightRay(point, light.position - point);
  size_t count = spheres.size();

  for(size_t i = 0; i < count; ++i) {
    REAL_TYPE a = lightRay.direction.dot(lightRay.direction);
    REAL_TYPE b = 2 * lightRay.direction.dot(lightRay.origin - spheres[i].center);
    REAL_TYPE c = (lightRay.origin - spheres[i].center).dot(lightRay.origin - spheres[i].center) - spheres[i].radius * spheres[i].radius;
    REAL_TYPE discriminant = b * b - 4 * a * c;
    if(discriminant < 0) {
      continue;
    }
    REAL_TYPE sqrtDiscriminant = std::sqrt(discriminant);
    REAL_TYPE t1 = (-b - sqrtDiscriminant) / (2 * a);
    if(t1 > 0.001) {
      return true;
    }
    else {
      REAL_TYPE t2 = (-b + sqrtDiscriminant) / (2 * a);
      if(t2 > 0.001) {
        return true;
      }
    }
  }


  return false;
}

Color calculateColor(const Vector<Vector3> intersection,
                     const Light& light,
                     const Vector<Sphere>& spheres,
                     const Vector<Plane>& planes,
                     const Vector3& currentDir) {
  Color result;
  Color ambient, diffuse, specular, defaultColor = intersection[2];
  REAL_TYPE kA = intersection[3].x;
  REAL_TYPE kD = intersection[3].y;
  REAL_TYPE kS = intersection[3].z;

  Vector3 relativePos = light.position - intersection[0];
  Vector3 lightDir = relativePos.getNormalized();
  ambient = defaultColor * light.ambientIntensity * kA;
  
  bool shadow = isInShadow(intersection[0], light, spheres, planes);
  if(!shadow) {
    REAL_TYPE lightDistance = relativePos.length();
    auto attenuation = light.intensity / (lightDistance * lightDistance);
    diffuse = defaultColor * kD * attenuation * std::max((REAL_TYPE)0.f, intersection[1].dot(lightDir));
    Vector3 I = lightDir * -1;
    Vector3 N = intersection[1];
    Vector3 reflectDir = I - 2.f * (I.dot(N)) * N;
    specular = WHITE * kS * attenuation * pow(std::max((REAL_TYPE)0.0, reflectDir.dot(currentDir * -1)), (REAL_TYPE)20);
  }

  result = ambient + diffuse + specular;
  result.x = std::min((REAL_TYPE)255, result.x);
  result.y = std::min((REAL_TYPE)255, result.y);
  result.z = std::min((REAL_TYPE)255, result.z);
  return result;
}

Color findColor(const Ray& ray, 
                const Vector<Sphere>& spheres,
                const Vector<Plane>& planes,
                const Light& light,
                int maxDepth) {
  Color colorResult(0, 0, 0);
  Color noIntersectionColor(255, 255, 255);
  int depth = 0;
  Ray currentRay = ray;
  while(depth < maxDepth) {
    Vector<Vector3> intersectionPoint = findClosestIntersection(currentRay, spheres, planes);
    if(!intersectionPoint.empty()) {
      if(intersectionPoint[4].x == -1) {//Plane, no reflection
        maxDepth = 1;
      }
      
      depth = depth + 1;
      colorResult = colorResult + calculateColor(intersectionPoint, light, spheres, planes, currentRay.direction);
      auto& interNormal = intersectionPoint[1];
      currentRay.direction = currentRay.direction - 2 * (currentRay.direction.dot(interNormal)) * interNormal;
      currentRay.origin = intersectionPoint[0];
      
    }
    else {
      break;
    }

  }

  if(depth != 0) {
    return colorResult / (REAL_TYPE)depth;
  }
  else {
    return noIntersectionColor;
  }
  return colorResult;
}

int main() {
  Vector<Sphere> spheres;
  spheres.emplace_back(Vector3(-036, -28, 220), 10, Vector3(139, 0, 0), kA, kD, kS);
  spheres.emplace_back(Vector3(-055, -23, 230), 35, Vector3(255, 0, 0), kA, kD, kS);
  spheres.emplace_back(Vector3(-107, -39, 157), 10, Vector3(255, 69, 0), kA, kD, kS);
  spheres.emplace_back(Vector3( 013, -11, 235), 10, Vector3(255, 215, 0), kA, kD, kS);
  spheres.emplace_back(Vector3(-151, -27, 220), 14, Vector3(128, 128, 0), kA, kD, kS);
  
  Vector3 norm1 (-5, 0, -4), point1(1300, 500, 500), planeColor(230, 182, 200);
  Vector3 norm2(5, 0, -4), point2(-1000, 500, 500);
  Vector3 norm3(0, 0, 1), point3(0, 0, 0);
  Vector3 normal(0, 5, -1), point(0, -60, 120);

  Plane wall1(norm1, point1, planeColor, kA, kD, kS);
  Plane wall2(norm2, point2, planeColor, kA, kD, kS);
  Plane wall3(norm3, point3, planeColor, kA, kD, kS);
  Plane floor(normal, point, planeColor, kA, kD, kS);

  Vector<Plane> planes {
    floor, 
    wall1,
    wall2,
    wall3
  };

  Vector3 eye(0, 0, 0);
  Vector3 lightPos(400, 400, 400);
  REAL_TYPE lightIntensity = 585000;
  REAL_TYPE ambientIntensity = 1;
  
  Light light(lightPos, lightIntensity, ambientIntensity);

  const int w = 1920;
  const int h = 1080 ;

  Image image;
  image.create(w, h, 32);

  //01:35:00
  //04:00:00
  REAL_TYPE screenHeight = 100;
  Vector3 upperLeftCorner(50, 50, 100);
  REAL_TYPE ratio = (REAL_TYPE)h / screenHeight;

  for(int y = 0; y < h; ++y) {
    for(int x = 0; x < w; ++x) {
      Color pixelColor;
      
      for(int j = 1; j<=AASamples; ++j)
      //int j = 1;
      {
        float dx = (float)rand() / RAND_MAX;
        float dy = (float)rand() / RAND_MAX;


        Vector3 tmpPixel((-x + dx) / ratio, (-y + dy) / ratio, 0);
        Vector3 pixel = upperLeftCorner + tmpPixel;
        
        Ray currentRay(eye, pixel - eye);
        pixelColor = pixelColor + findColor(currentRay, spheres, planes, light, MAX_DEPTH);
      }
      ColorImg pixelColorImg;
      pixelColorImg.r = (unsigned char)std::min((REAL_TYPE)255, pixelColor.x / (REAL_TYPE)AASamples);
      pixelColorImg.g = (unsigned char)std::min((REAL_TYPE)255, pixelColor.y / (REAL_TYPE)AASamples);
      pixelColorImg.b = (unsigned char)std::min((REAL_TYPE)255, pixelColor.z / (REAL_TYPE)AASamples);
      pixelColorImg.a = 255;
      image.setPixel({x, y}, pixelColorImg);
    }
  }

  image.encode("output.bmp");


  return 0;
}