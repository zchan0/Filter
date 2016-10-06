#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <OpenImageIO/imageio.h>
#include <iostream>

#include "ImageIO.h"

OIIO_NAMESPACE_USING

static std::map<std::string, int>format_nchannels_map;

ImageIO::ImageIO() {
  width = height = nchannels = 0;
  buildMap(); // for every object, miantain one map
}

ImageIO::~ImageIO() {
  delete []inPixmap;
  delete []outPixmap;
  delete []rgbInPixmap;
  delete []rgbOutPixmap;
}

void ImageIO::loadImage(const std::string fileName) {
  ImageInput *inFile = ImageInput::open(fileName);
  if(!inFile) {
    std::cerr << "Could not open " << fileName << ", error = " << geterror() << std::endl; 
    return;
  }

  // to always use RGBA to draw image, need convert RGB to RGBA
  // should first create corrpesponding pixmap(RGB or RGBA pixel), then convert if needed, for read_image function read total image
  const ImageSpec &spec = inFile -> spec();
  nchannels = spec.nchannels;
  width  = spec.width;
  height = spec.height;
  inPixmap = new RGBAPixel[width * height];

  bool readSucceed;
  if (nchannels == RGB_NCHANNELS) {
    rgbInPixmap = new RGBPixel[width * height];
    readSucceed = inFile -> read_image(TypeDesc::UINT8, &rgbInPixmap[0]);
  } else if (nchannels == GREY_NCHANNELS) {
    greyPixmap = new unsigned char[width * height];
    readSucceed = inFile -> read_image(TypeDesc::UINT8, greyPixmap);
  } else if (nchannels == RGBA_NCHANNELS) {
    readSucceed = inFile -> read_image(TypeDesc::UINT8, &inPixmap[0]);
  } else {
    std::cout << "nchannels is " << nchannels << ", not supported" << std::endl;
    return;
  }

  if(!readSucceed) {
    std::cerr << "Could not read image file " << fileName << ", error = " << geterror() << std::endl;
    delete inFile;
    return;
  } 

  // for draw with GL_RGBA
  if (nchannels == RGB_NCHANNELS && rgbInPixmap != NULL) {
    convertRGBToRGBA(width, height);
  }
  if (nchannels == GREY_NCHANNELS && greyPixmap != NULL) {
    convertGreyToRGBA(width, height);
  }

  // inverse pixmap before drawing, because of openGL lower-left coordinate
  // for first argument type is unsigned char*, &inPixmap[0].r is the head address
  convertPixmapToUpperLeft(&inPixmap[0].r, RGBA_NCHANNELS, width, height);

  if(!inFile -> close()) {
    std::cerr << "Could not close " << fileName << ", error = " << geterror() << std::endl;
    delete inFile;
    return;
  }

  delete inFile;
}

void ImageIO::exportImage(const std::string fileName) {
  ImageOutput *outFile = ImageOutput::create(fileName);
  if (!outFile) {
    std::cerr << "Could not create output image for " << fileName << ", error = " << geterror() << std::endl;
    return;
  }
  
  // get image nchannels from input file name
  int nchannels = getNChannelsByFormat(fileName);
  if (nchannels == -1) { 
    std::cout << "Not support for " << fileName << std::endl;
    return;
  }

  // get current window size 
  int w = glutGet(GLUT_WINDOW_WIDTH);
  int h = glutGet(GLUT_WINDOW_HEIGHT);
  outPixmap = new RGBAPixel[w * h];

  // always use GL_RGBA here
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, outPixmap);
  ImageSpec spec(w, h, nchannels, TypeDesc::UINT8);
  
  if (!outFile -> open(fileName, spec)) {
    std::cerr << "Could not open " << fileName << ", error = " << geterror() << std::endl;
    delete outFile;
    return;
  }

  bool writeSucceed;
  if (nchannels == RGB_NCHANNELS) {
    rgbOutPixmap = new RGBPixel[w * h];
    convertRGBAToRGB(w, h);
    // inverse pixmap before writing, because of openGL lower-left coordinate
    convertPixmapToUpperLeft(&rgbOutPixmap[0].r, nchannels, w, h);
    writeSucceed = outFile -> write_image(TypeDesc::UINT8, rgbOutPixmap);
  } else if (nchannels == RGBA_NCHANNELS) { 
    convertPixmapToUpperLeft(&outPixmap[0].r, nchannels, w, h);
    writeSucceed = outFile -> write_image(TypeDesc::UINT8, outPixmap);
  } else if (nchannels == GREY_NCHANNELS) {
    unsigned char* pixmap = new unsigned char[w * h];
    convertRGBAToGrey(pixmap, w, h);
    writeSucceed = outFile -> write_image(TypeDesc::UINT8, pixmap);
  }

  if (!writeSucceed) {
    std::cerr << "Could not write image to " << fileName << ", error = " << geterror() << std::endl;
    delete outFile;
    return;
  }

  if (!outFile -> close()) {
    std::cerr << "Could not close " << fileName << ", error = " << geterror() << std::endl;
    delete outFile;
    return;
  }

  std::cout << "Export succeed!" << std::endl;

  delete outFile;
}

void ImageIO::drawImage() {
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  if(inPixmap != NULL) {
    int w = glutGet(GLUT_WINDOW_WIDTH); 
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // calculate portion of image size to current window size
    // factorX/Y < 1 means current window size need resize image
    // use the smallest factor to fit current window
    double factorX = (double)w / width;
    double factorY = (double)h / height;
    double factor = std::min(1.0, std::min(factorX, factorY));

    // calculate center postion
    // if image width or height small than window's, corresponding raster x or y should be 0
    // if window width or height bigger than image's, corresponding raster x or y should be in center 
    int centerX = factorX < 1 ? 0 : (w - width) / 2;
    int centerY = factorY < 1 ? 0 : (h - height) / 2;

    glPixelZoom(factor, factor);
    glRasterPos2i(centerX, centerY);
   
    //glDrawPixels writes a block of pixels to the frame buffer.
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, inPixmap);
  }

  // swap the back and front buffers so can see what just drew
  glutSwapBuffers();
}

/**
 *
 * This idea credits to Kairui Wang, thanks! *
 */
void ImageIO::convertPixmapToUpperLeft(unsigned char* pixmap, int nchannels, int width, int height) {
  for(size_t j = height - 1; j > (height +1) / 2 - 1; --j) {
    for(size_t i = 0; i < width * nchannels; ++i) {
      std::swap(pixmap[j * width * nchannels + i], pixmap[(height - 1 - j) * width * nchannels + i]);
    }
  }
}

void ImageIO::convertRGBToRGBA(int iw, int ih) {
  for (size_t i = 0; i < iw * ih; ++i) {
    inPixmap[i].r = rgbInPixmap[i].r;
    inPixmap[i].g = rgbInPixmap[i].g;
    inPixmap[i].b = rgbInPixmap[i].b;
    inPixmap[i].a = 255;
  }
}

void ImageIO::convertRGBAToRGB(int iw, int ih) {
  for (size_t i = 0; i < iw * ih; ++i) {
    rgbOutPixmap[i].r = outPixmap[i].r;
    rgbOutPixmap[i].g = outPixmap[i].g;
    rgbOutPixmap[i].b = outPixmap[i].b;
  }
}

void ImageIO::convertGreyToRGBA(int iw, int ih) {
  for (size_t i = 0; i < iw * ih; ++i) {
    inPixmap[i].r = greyPixmap[i];
    inPixmap[i].g = greyPixmap[i];
    inPixmap[i].b = greyPixmap[i];
    inPixmap[i].a = 255;
  }
}

void ImageIO::convertRGBAToGrey(unsigned char* greyPixmap, int iw, int ih) {
  for (size_t i = 0; i < iw * ih; ++i) {
    greyPixmap[i] = inPixmap[i].r;
  }
}

/**
 *
 * Build a map(dictionary), image file format : num of image channels 
 *
 */
void ImageIO::buildMap() {
  format_nchannels_map["ppm"] = 3;
  format_nchannels_map["png"] = format_nchannels_map["jpg"] = format_nchannels_map["tif"] = 4;
}

int ImageIO::getNChannelsByFormat(const std::string fileName) {
  std::string fileFormat;
  const int pos = fileName.find_last_of('.');
  // extract image format from fileName
  fileFormat = fileName.substr(pos + 1); 
  
  return format_nchannels_map[fileFormat] ? : -1;
}

int ImageIO::getInWidth() const {
  return width;
}

int ImageIO::getInHeight() const {
  return height;
}

int ImageIO::getNChannels() const {
  return nchannels;
}
