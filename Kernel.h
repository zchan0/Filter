class Kernel {
public:
	double **weights;

	Kernel();
	~Kernel();

	int getWidth() const;
	int getHeight() const;
	double getScale() const;

	void setScale(double s = 0);
	void setSize(int w, int h);
	void readKernelFile(const std::string filenname);
	void printKernel() const;
	
private:
	int width, height;
	double scale;
	double getWeights() const;
};