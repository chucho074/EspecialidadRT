/**
 * @file    Image.cpp
 * @author  Jesus Alberto Del Moral Cupil
 * @e       edgv24c.jmoral@uartesdigitales.edu.mx
 * @date    04/01/25
 */
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION

#pragma pack(push, 2)
struct MY_BITMAPFILEHEADER {
  unsigned short bfType;
  unsigned long  bfsize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned long  bfOffBits;
};
#pragma pack(pop)

struct MY_BITMAPCOREHEADER {
  unsigned long  bcSize;
  unsigned long bcWidth;
  unsigned long bcHeigh;
  unsigned short bcPlanes;
  unsigned short bcBitCount;
};

struct MY_BITMAPSAVEHEADER {
  
  MY_BITMAPCOREHEADER coreHeader;
  unsigned long biCompression;
  unsigned long biSizeImage;
  long biXPelsPerMeter;
  long biYPelsPerMeter;
  unsigned long biClrUsed;
  unsigned long biClrImportant;
};



void 
Image::decode(Path inFilePath) {
  m_brga = true;
  fstream imgFile(inFilePath, ios::in | ios::binary | ios::ate);
  if(!imgFile.is_open()) {
    //decode("Models/missingTextureV2.png");
    __debugbreak();
    return;
  }

  auto fileSize = imgFile.tellg();
  imgFile.seekp(ios::beg);

  MY_BITMAPFILEHEADER fileHeader;
  imgFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(MY_BITMAPFILEHEADER));

  if(fileHeader.bfType != 0x4D42) {
    //decode("Models/missingTextureV2.png");
    __debugbreak();
    return; //Not a BMP
  }

  MY_BITMAPCOREHEADER infoHeader;
  imgFile.read(reinterpret_cast<char*>(&infoHeader), sizeof(MY_BITMAPCOREHEADER));

  imgFile.seekg(fileHeader.bfOffBits);

  create(infoHeader.bcWidth, infoHeader.bcHeigh, infoHeader.bcBitCount);

  int padding = getPitch() % 4;
  int lineMemoryWidth = getPitch();
  if (padding) {
    padding = 4 - padding;
    lineMemoryWidth += padding;
  }

  for (int line = m_height - 1; line >= 0; --line) {
    imgFile.seekp(lineMemoryWidth * line + fileHeader.bfOffBits);

    imgFile.read(reinterpret_cast<char*>(&m_pixels[getPitch() * (getHeight() - 1 - line)]), getPitch());
    
  }

}

void 
Image::encode(Path inFilePath) {
  fstream imgFile(inFilePath, ios::out | ios::binary);
  MY_BITMAPFILEHEADER fileHeader;
  MY_BITMAPSAVEHEADER bmpInfo;
  memset(&fileHeader, 0, sizeof(MY_BITMAPFILEHEADER));
  memset(&bmpInfo, 0, sizeof(MY_BITMAPSAVEHEADER));

  int padding = getPitch() % 4;
  int lineMemoryWidth = getPitch();
  if(padding) {
    padding = 4 - padding;
    lineMemoryWidth += padding;
  }

  int headerSize = sizeof(MY_BITMAPFILEHEADER) + sizeof(MY_BITMAPSAVEHEADER);

  fileHeader.bfType = 0x4D42; // 'BM'
  fileHeader.bfsize = headerSize + lineMemoryWidth * m_height;
  fileHeader.bfOffBits = headerSize;

  bmpInfo.coreHeader.bcSize = sizeof(MY_BITMAPSAVEHEADER);
  bmpInfo.coreHeader.bcWidth = m_width;
  bmpInfo.coreHeader.bcHeigh = m_height;
  bmpInfo.coreHeader.bcPlanes = 1;
  bmpInfo.coreHeader.bcBitCount = m_bpp;
  
  bmpInfo.biXPelsPerMeter = 3780;
  bmpInfo.biYPelsPerMeter = 3780;

  imgFile.write(reinterpret_cast<char*>(&fileHeader), sizeof(MY_BITMAPFILEHEADER));
  imgFile.write(reinterpret_cast<char*>(&bmpInfo), sizeof(MY_BITMAPSAVEHEADER));


  char paddBuffer[3] = {0, 0, 0};
  for(int line = m_height - 1; line >= 0; --line) {
    imgFile.write(reinterpret_cast<char*>(&m_pixels[getPitch() * line]), getPitch());
    if(padding) {
      imgFile.write(paddBuffer, padding);
    }
  }

}

void 
Image::clearColor(const ColorImg& inColor) {
  for(int y = 0; y < m_height; ++y ) { //Filas
    for (int x = 0; x < m_width; ++x) { //Columnas
      setPixel({x, y}, inColor);
    }
  }
}

void 
Image::bitBlt(Image& src, 
              Vector2i inPos, 
              Rect inSrcRect, 
              ColorImg* inColorKey) {
  
  if (inPos.x >=  getWidth() || inPos.y >= getHeight()) {
    return;
  }
  
  if(0 == inSrcRect.EndX) inSrcRect.EndX = src.getWidth();
  if(0 == inSrcRect.EndY) inSrcRect.EndY = src.getHeight();

  if(inPos.x < 0) {
    inSrcRect.IniX -= inPos.x;
    inPos.x = 0;
  }
  if(inPos.y < 0) {
    inSrcRect.IniY -= inPos.y;
    inPos.y = 0;
  }

  int realWidth = inSrcRect.EndX - inSrcRect.IniX;
  int realHeight = inSrcRect.EndY - inSrcRect.IniY;
  
  if (realWidth < 0 || realHeight < 0) {
    return;
  }

  if (inPos.x + realWidth > getWidth()) {
    realWidth -= (inPos.x + realWidth) - getWidth();
  }

  if(inPos.y + realHeight > getHeight()) {
    realHeight -= (inPos.y + realHeight) - getHeight();
  }

  for(int i = 0; i < realHeight; ++i) {
    for(int j = 0; j < realWidth; ++j) {
      
      ColorImg pixelColor = src.getPixel({inSrcRect.IniX + j, inSrcRect.IniY + i});
      if (inColorKey) {
        if (pixelColor == *inColorKey) {
          continue;
        }
      }
      setPixel({j + inPos.x, i + inPos.y}, pixelColor);
      
    }
  }
}

void 
Image::line(Vector2i inIniPos, Vector2i inEndPos, const ColorImg& inColor) {

  clipLine(inIniPos, inEndPos, inColor);

  //Sacando la pendiente de la linea.
  float dx = inEndPos.x - inIniPos.x;
  float dy = inEndPos.y - inIniPos.y;

  float steps = std::max(abs(dx), abs(dy));
  
  float xInc = dx / steps;
  float yInc = dy / steps;

  float x = inIniPos.x;
  float y = inIniPos.y;

  for (int i = 0; i < int(steps); ++i) { 
    setPixel({int(x), int(y)}, inColor);
    x += xInc;
    y += yInc;
  }
}

void 
Image::bresenhamline(Vector2i inIniPos, Vector2i inEndPos, const ColorImg& inColor) {
  
  clipLine(inIniPos, inEndPos, inColor);

  int dx = abs(inEndPos.x - inIniPos.x);
  int dy = abs(inEndPos.y - inIniPos.y);

  int sx = inIniPos.x < inEndPos.x ? 1 : -1;
  int sy = inIniPos.y < inEndPos.y ? 1 : -1;

  int err = dx - dy;

  int e2;

  while(inIniPos != inEndPos) {
    setPixel(inIniPos, inColor);

    e2 = 2*err;
    if(e2 > -dy) {
      err -= dy;
      inIniPos.x += sx;
    }
    if(e2 < dx) {
      err+=dx;
      inIniPos.y += sy;
    }
  }
}

void 
Image::bresenhamCircle(Vector2i inPos, int inRadius, const ColorImg& inColor) {
  int xMin = 0;
  int yMin = 0;
  int xMax = m_width;
  int yMax = m_height;

  int x = inRadius;
  int y = 0;
  int err = 0;

  while (x >= y) {
    setPixel({inPos.x + x, inPos.y + y}, inColor);
    setPixel({inPos.x + y, inPos.y + x}, inColor);
    setPixel({inPos.x - y, inPos.y + x}, inColor);
    setPixel({inPos.x - x, inPos.y + y}, inColor);
    setPixel({inPos.x - x, inPos.y - y}, inColor);
    setPixel({inPos.x - y, inPos.y - x}, inColor);
    setPixel({inPos.x + y, inPos.y - x}, inColor);
    setPixel({inPos.x + x, inPos.y - y}, inColor);

    if (err <= 0) {
      y += 1;
      err += 2 * y + 1;
    }
    if (err > 0) {
      x -= 1;
      err -= 2 * x + 1;
    }
  }
}

ColorImg 
Image::getPixel(Vector2i inPos) {
  ColorImg outColor;
  int pixelPos = (inPos.y * getPitch()) + (inPos.x * getBpp());
  if (inPos < 0 || inPos.x >= m_width || inPos.y >= m_height) {
    return outColor;
  }

  outColor.r = m_pixels[pixelPos + 2];
  outColor.g = m_pixels[pixelPos + 1];
  outColor.b = m_pixels[pixelPos + 0];
  outColor.a = 255;

  return outColor;
}

void 
Image::setPixel(Vector2i inPos, const ColorImg& inColor) {
  
  if(inPos < 0 || inPos.x >= m_width || inPos.y >= m_height) {
    return;
  }
  
  int pixelPos = (inPos.y * getPitch()) + (inPos.x * getBpp());

  m_pixels[pixelPos + 2] = inColor.r;
  m_pixels[pixelPos + 1] = inColor.g;
  m_pixels[pixelPos + 0] = inColor.b;

  if(4 == getBpp()) {
    m_pixels[pixelPos + 3] = inColor.a;
  }
}

bool 
Image::clipLine(Vector2i& inIniPos, Vector2i& inEndPos, const ColorImg& inColor) {
  int xMin = 0;
  int yMin = 0;
  int xMax = 1920;
  int yMax = 1080;

  int code0 = computeRegionCode(inIniPos, xMin, yMin, xMax, yMax);
  int code1 = computeRegionCode(inEndPos, xMin, yMin, xMax, yMax);

  while (true) {
    if(!(code0 | code1)) { //Ambos puntos dentro del rectangulo
      return true;
    }
    else if (code0 & code1) { //Ambos puntos fuera del rectangulo
      return false;
    }
    else {
      int x, y;
      int codeOut = code0 ? code0 : code1;
      if(codeOut & RegionCode::kTOP) {
        x = inIniPos.x + (inEndPos.x - inIniPos.x) * (yMax - inIniPos.y) / (inEndPos.y - inIniPos.y);
        y = yMax;
      }
      else if(codeOut & RegionCode::kBOTTOM) {
        x = inIniPos.x + (inEndPos.x - inIniPos.x) * (yMin - inIniPos.y) / (inEndPos.y - inIniPos.y);
        y = yMin;
      }
      else if (codeOut & RegionCode::kRIGHT) {
        y = inIniPos.y + (inEndPos.y - inIniPos.y) * (xMin - inIniPos.x) / (inEndPos.x - inIniPos.x);
        x = xMax;
      }
      else if (codeOut & RegionCode::kLEFT) {
        y = inIniPos.y + (inEndPos.y - inIniPos.y) * (xMin - inIniPos.x) / (inEndPos.x - inIniPos.x);
        x = xMin;
      }
      if(codeOut == code0) {
        inIniPos.x = x;
        inIniPos.y = y;
        code0 = computeRegionCode(inIniPos, xMin, yMin, xMax, yMax);
      }
      else {
        inEndPos.x = x;
        inEndPos.y = y;
        code1 = computeRegionCode(inEndPos, xMin, yMin, xMax, yMax);
      }
    }
  }

  return false;
}
