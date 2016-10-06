#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#include "Kernel.h"

Kernel::Kernel() {
	width = height = 0;
	scale = 0.0;
	weights = NULL; 
}

Kernel::~Kernel() {
	if (weights != NULL) {
		for (int i = 0; i < height; ++i) {
			delete [] weights[i];
		}
		delete weights;
	}
}

int Kernel::getWidth() const {
	return width;
}

int Kernel::getHeight() const {
	return height;
}

double Kernel::getScale() const {
	return scale;
}

double Kernel::getWeights() const {
	double val; // for calculation in loop
	double negsum = 0;
	double possum = 0;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			val = weights[i][j];
			if (val >= 0) {
				possum += val;
			} else {
				negsum += val;
			}
		}
	}
	if (possum + negsum == 0) {
		return 1;
	} else {
		return std::max(possum, std::fabs(negsum));
	}
}

/**
 * Set s default value is 0, if s = 0 ( setScale() ), use getWeights() to calculate weights, otherwise, assign s to scale
 * @param s scale, did not divide 1
 */
void Kernel::setScale(double s) {
	if (s != 0) {
		scale = s;
	} else {
		scale = getWeights();
	}
}

void Kernel::setSize(int w, int h) {
	width  = w;
	height = h;

	weights = new double*[h];
	for (int i = 0; i < h; ++i) {
		weights[i] = new double[w];
	}

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			weights[i][j] = 0;
		}
	}
}

/**
 * setup kernel from file(filename)
 * file format must be
 * line 1: kernel-size scale-factor 
 * line 2: the first row of kernel-size weights 
 * ... 
 * line kernel-size + 1: the last row of kernel-size weights
 * 
 */
void Kernel::readKernelFile(const std::string filename) {
	std::ifstream file(filename);
	std::string str;
	std::vector<std::string> v;	

	if (file.is_open()) {
		while(!file.eof()) {
			str = " ";
			file >> str;
			if (str != " ") {
				v.push_back(str);
			}
		}
		file.close();
	}

	int row, col;
	for (int i = 0; i < v.size(); ++i) {
		switch (i) {
			case 0: 
				width = height = stoi(v[i]);
				setSize(width, height);
				break;
			case 1: 
				setScale(stod(v[i])); 
				break;
			default:
				row = (i - 2) / width;
				col = (i - 2) - row * width;
				weights[row][col] = stod(v[i]);
		}
	}

	// Use weights to calculate scale, ignore the one read from file
	setScale(getWeights());
}

void Kernel::printKernel() const {
	std::cout << "size: " << width << " X "<< height << " scale: 1 / " << scale << std::endl;
	std::cout << std::fixed;
	std::cout.precision(3);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			std::cout << weights[i][j] << "\t";
		}
		std::cout << std::endl;
	}
}