#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <iostream>
#include <cmath>
#include <fstream>

#include "ImageIO.h"
#include "Kernel.h"

// Special char
static const unsigned char ESC = 27;

// Window size
static const int Width  = 640;
static const int Height = 480;

// Window handles 
static int windowOrigin;
static int windowFilted;

// ImageIO handles
static std::string input, output, filt;
static ImageIO ioOrigin = ImageIO();
static ImageIO ioFilted = ImageIO();

// Filt handles
static Kernel kernel = Kernel();

bool getFileNameFromCommandLine(int argc, char* argv[]) {
  switch (argc) {
  case 3:
  	input = argv[1];
    filt  = argv[2];
    
    if (argv[3] != NULL) {
    	output = argv[3];
    } else {
    	output = "output.png";
    }

    return true;
    break;
  default:
    std::cerr << "Usage: filt in.png filter.filt [out.png]" << std:: endl;
    exit(1);
  	return false;
    break;
  }
}

void loadImage() {
	ioOrigin.loadImage(input);
	ioFilted.loadImage(input);
}

unsigned char getColorValue(RGBAPixel pixel, int channel) {
	switch(channel) {
		case 0: return pixel.r; break;
		case 1: return pixel.g; break;
		case 2: return pixel.b; break;
		case 3: return pixel.a; break;
		default: return 0; break;
	}
}

// Reflective padder
void getPixel(int x, int y, int kx, int ky, int& row, int& col) {
	row = y + ky;
	col = x + kx;
	if (row < 0) {
		row = -1 - row;
	} else if (row > ioFilted.getInHeight() - 1) {
		row -= 2 * ky - 1;
	}
	if (col < 0) {
		col = -1 - col;
	} else if (col > ioFilted.getInWidth() - 1) {
		col -= 2 * kx - 1;
	}
}

void convolve() {
	int iw = ioFilted.getInWidth();
	int ih = ioFilted.getInHeight();
	int kw = kernel.getWidth();
	int kh = kernel.getHeight();

	double **pixmap; // Store intermediate results
	pixmap = new double*[RGB_NCHANNELS];
	for (int i = 0; i < RGB_NCHANNELS; ++i) {
		pixmap[i] = new double[iw * ih];
	}

	double sum;
	int row, col;
	// Calculate sum for every primary color
	for (int channel = 0; channel < RGB_NCHANNELS; ++channel) {
		for (int i = 0; i < ih; ++i) {
			for (int j = 0; j < iw; ++j) {
				// Do convolve	
				sum = 0.0;
				for (int ki = - kh / 2; ki <= kh / 2; ++ki) {
					for (int kj = - kw / 2; kj <= kw / 2; ++kj) {
						getPixel(j, i, kj, ki, row, col);
						sum += kernel.weights[ki + kh / 2][kj + kw / 2] * (getColorValue(ioFilted.inPixmap[row * ih + col], channel) / 255.0);
					}
				}
				pixmap[channel][i * ih + j] = sum;	
			}
		}
	}

	// Convert back to 0 ~ 255 range
	double val; // for calculation in loop
	for (int channel = 0; channel < RGB_NCHANNELS; ++channel) {
		for (int i = 0; i < iw * ih; ++i) {
			val = std::abs(pixmap[channel][i]) / kernel.getScale() * 255.0;
			val = val > 255.0 ? 255 : val;

			switch(channel) {
				case 0: ioFilted.inPixmap[i].r = (unsigned char)val; break;
				case 1: ioFilted.inPixmap[i].g = (unsigned char)val; break;
				case 2: ioFilted.inPixmap[i].b = (unsigned char)val; break;
			}	
		}			
	}		

	glutPostRedisplay();
}

void displayOriginWindow() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ioOrigin.drawImage();
}

void displayFiltedWindow() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ioFilted.drawImage();
}

void handleReshape(int width, int height) {
	glViewport(0, 0, width, height);

  	glMatrixMode(GL_PROJECTION);
	// Clear the projection matrix 
	glLoadIdentity();
  	gluOrtho2D(0,  width, 0, height);
	glMatrixMode(GL_MODELVIEW);	
}

void handleKeyboard(unsigned char key, int x, int y) {
	switch(key) {
	case 'c': case 'C': convolve(); break;
    case 'q': case 'Q': case ESC: exit(0); break;
  }	
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

  	if (getFileNameFromCommandLine(argc, argv)) {
		loadImage();	
		kernel.readKernelFile(filt);
	}

	// Origin image window
	windowOrigin = glutCreateWindow("Original Image");
	glutDisplayFunc(displayOriginWindow);
	glutKeyboardFunc(handleKeyboard);
	glutReshapeFunc(handleReshape);

	// Filt image window
	windowFilted = glutCreateWindow("Filted Image");
	glutDisplayFunc(displayFiltedWindow);
	glutKeyboardFunc(handleKeyboard);
	glutReshapeFunc(handleReshape);

	glutMainLoop();

	return 0;
}