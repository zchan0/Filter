#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <iostream>

#include "ImageIO.h"

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
    case 'q': case 'Q': case ESC: exit(0); break;
  }	
}

// Filter


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

  	if (getFileNameFromCommandLine(argc, argv)) {
		loadImage();
	}

	// Origin image window
	windowOrigin = glutCreateWindow("Original Image");
	glutDisplayFunc(displayOriginWindow);
	glutKeyboardFunc(handleKeyboard);
	glutReshapeFunc(handleReshape);

	// Filt image window
	windowFilted = glutCreateWindow("Filted Image");
	glutDisplayFunc(displayFiltedWindow);
	glutReshapeFunc(handleReshape);

	glutMainLoop();

	return 0;
}