#define RGB_NCHANNELS  3
#define RGBA_NCHANNELS 4
#define GREY_NCHANNELS 1

typedef struct {
  unsigned char r, g, b, a;
} RGBAPixel;

typedef struct {
  unsigned char r, g, b;
} RGBPixel;

class ImageIO {
  public:
    ImageIO();
    ~ImageIO();

    void loadImage(const std::string fileName);
    void exportImage(const std::string fileName);
    void drawImage();

    int getInWidth() const;
    int getInHeight() const;
    int getNChannels() const; 

    RGBAPixel *inPixmap;
    
  private:
    int width, height; // loaded image size, for draw image
    int nchannels;
    unsigned char *greyPixmap;
    RGBAPixel *outPixmap;
    RGBPixel  *rgbInPixmap, *rgbOutPixmap;

    int getNChannelsByFormat(const std::string fileName);
    void buildMap();
    void convertRGBToRGBA(int iw, int ih);
    void convertRGBAToRGB(int iw, int ih);
    void convertGreyToRGBA(int iw, int ih);
    void convertRGBAToGrey(unsigned char* greyPixmap, int iw, int ih);
    void convertPixmapToUpperLeft(unsigned char* pixmap, int nchannels, int width, int height);
};

