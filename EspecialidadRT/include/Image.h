/**
 * @file    Image.h
 * @author  Jesus Alberto Del Moral Cupil
 * @e       edgv24c.jmoral@uartesdigitales.edu.mx
 * @date    04/01/25
 */
#pragma once
#include <fstream>
#include <filesystem>

 //FileSystem
namespace fsys = std::filesystem;

//Path
using Path = std::filesystem::path;
using std::fstream;
using std::ios;

template<typename T>
using Vector = std::vector<T>;

struct ColorImg {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;

  bool 
  operator!=(const ColorImg& inColor) {
    if (r != inColor.r &&
        g != inColor.g &&
        b != inColor.b &&
        a != inColor.a) {
      return true;
    }
    return false;
  }

  bool 
  operator==(const ColorImg& inColor) {
    if (r == inColor.r &&
        g == inColor.g &&
        b == inColor.b) {
      return true;
    }
    return false;
  }
};

static ColorImg Black  {0,   0,   0,   255};
static ColorImg White  {255, 255, 255, 255};
static ColorImg Red    {255, 0,   0,   255};
static ColorImg Green  {0,   255, 0,   255};
static ColorImg Blue   {0,   0,   255, 255};
static ColorImg Purple {255, 0,   255, 255};
static ColorImg Grey   {30,  30,  30,  255};

class Vector2i {
 public:
  Vector2i() = default;
  Vector2i(int inX, int inY) : x(inX), y(inY) {}
  Vector2i(float inX, float inY) : x((int)inX), y((int)inY) {}
  ~Vector2i() = default;

  bool
  operator<(const Vector2i& other) const {
    return (x < other.x) || (x == other.x && y < other.y);
  }

  bool
  operator<=(int scalar) const {
    return (x <= scalar) && (y <= scalar);
  }

  bool
  operator<(int scalar) const {
    return (x < scalar) && (y < scalar);
  }

  bool
  operator<=(const Vector2i& other) const {
    return (x <= other.x) && (y <= other.y);
  }
  
  bool
  operator>(const Vector2i& other) const {
    return (x > other.x) || (x == other.x && y > other.y);
  }
  
  bool
  operator>(int scalar) const {
    return (x > scalar) && (y > scalar);
  }

  bool
  operator>=(const Vector2i& other) const {
    return (x >= other.x) && (y >= other.y);
  }
  
  bool
  operator==(const Vector2i& other) const {
    return (x == other.x) && (y == other.y);
  }
  
  bool 
  operator==(int scalar) const {
    return (x == scalar) && (y == scalar);
  }

  bool
  operator!=(const Vector2i& other) const {
    return (x != other.x) || (y != other.y);
  }

  bool
  operator!=(int scalar) const {
    return (x != scalar) || (y != scalar);
  }

  Vector2i
  operator-(const Vector2i& other) const {
    return Vector2i(x - other.x, y - other.y);
  }

  Vector2i
  operator+(const Vector2i& other) const {
    return Vector2i(x + other.x, y + other.y);
  }

  Vector2i
  operator*(int scalar) const {
    return Vector2i(x * scalar, y * scalar);
  }
  
  Vector2i
  operator/(int scalar) const {
    return Vector2i(x / scalar, y / scalar);
  }
  
  float x = 0, y = 0;
};

class Rect {
public:
  Rect(int inIniX, int inIniY, int inEndX, int inEndY) : IniX(inIniX), IniY(inIniY), EndX(inEndX), EndY(inEndY) {};
  ~Rect() = default;

  int IniX;
  int IniY;
  int EndX;
  int EndY;
  const static Rect Zero;

};

namespace TEXTURE_ADDRESS {
  enum E {
    kCLAMP = 0,
    kWRAP,
    kMIRROR,
    kBORDER,
    kMIRROR_ONCE,
    kNUMOFTYPES
  };
}

namespace RegionCode {
  enum E {
    kINSIDE = 0,
    kLEFT = 1,
    kRIGHT = 2,
    kBOTTOM = 4,
    kTOP = 8,
  };
}

class Image {
 public:
  Image() = default;

  ~Image() {
    
  };

  void
  create(int inWidth, int inHeight, int inbpp) {
    m_width = inWidth;
    m_height = inHeight;
    m_bpp = inbpp;
    m_pixels.resize(getPitch() * getHeight());
  }
  
  void
  decode(Path inFilePath);

  void
  encode(Path inFilePath);

  void
  clearColor(const ColorImg& inColor);

  /**
   * @brief       Set the bit information of a given img.
   * @param       src               The image to copy. 
   * @param       inPos             The m_position to set the image. 
   * @param       inRect            The rect to take of the image (src).
   * @param       inColorKey        The color to omit (transparency).
   */
  void
  bitBlt(Image& src, 
         Vector2i inPos, 
         Rect inRect = Rect::Zero,
         ColorImg* inColorKey = nullptr);


  void
  line(Vector2i inIniPos, Vector2i inEndPos, const ColorImg& inColor);

  void
  bresenhamline(Vector2i inIniPos, Vector2i inEndPos, const ColorImg& inColor);

  void
  bresenhamCircle(Vector2i inPos, int inRadius, const ColorImg& inColor);

  int
  getWidth() const {
    return m_width;
  }

  int 
  getHeight() const {
    return m_height;
  }

  int 
  getBpp() const {
    return m_bpp >> 3;
  }

  int 
  getPitch() const {
    return m_width * getBpp(); //Densidad de informacion por linea
  }

  const unsigned char*
  getPixels() const {
    return m_pixels.data();
  }

  ColorImg 
  getPixel(Vector2i inPos);

  void 
  setPixel(Vector2i inPos, const ColorImg& inColor);

  int 
  computeRegionCode(Vector2i inPos, int xMin, int yMin, int xMax, int yMax) {
    int code = RegionCode::kINSIDE;
    if (inPos.x < xMin) { code |= RegionCode::kLEFT; }
    else if (inPos.x > xMax) { code |= RegionCode::kRIGHT; }

    if (inPos.y < yMin) { code |= RegionCode::kBOTTOM; }
    else if (inPos.y > yMax) { code |= RegionCode::kTOP; }

    return code;
  }

  bool
  clipLine(Vector2i& inIniPos, Vector2i& inEndPos, const ColorImg& inColor);


 public:
  //Resolution
  int m_width;
  int m_height;
  int m_bpp; //Bits per pixel

  bool m_brga = false;

  //unsigned char * m_pixels = nullptr; //in bytes
  Vector<unsigned char> m_pixels; //in bytes
};
