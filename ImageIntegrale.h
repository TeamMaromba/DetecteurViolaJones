#ifndef IMAGEINTEGRALEH
#define IMAGEINTEGRALEH

#include <vector>
#include <thread>
#include "CImg.h"


// IntegralImage of an array
std::vector<std::vector<double> > IntegralImage(std::vector<std::vector<double> >& data);

// Read an image
std::vector<double> ReadData(const char *filepath);

// Read an image outputs a grayscale matrix
std::vector<std::vector<double> > DataToMatrix(const char *filepath);

// Print a vector of vectors
void printData(std::vector<std::vector<double> >& data);

//Q1.1
std::vector<std::vector<long>> SAT(const cimg_library::CImg<long>& img);    //Summed Area Table

//Q1.2
class feature{
public:
	int x,y,w,h;
	char type;

	feature(int,int,int,int,char);
};

long calcFeat(std::vector<std::vector<long>>& sat, feature& f);

void featVect(std::vector<feature>& feats, char type,int& wMax, int& hMax);

std::vector<feature> distFeat(int& widht, int& height);

//Q2.1
class classifier{
public:
	double w1,w2;
	classifier();
	int calc(double);
};

void readImgs(int& nTasks, int taskId,bool pos,int& nPos, std::vector<std::vector<std::vector<long>>>& iis ,std::string& rep);

std::vector<std::vector<std::vector<long> > > distII(int& nPos,int& nNeg, std::string rep,int& nTasks);

void train(int& nTasks, int taskId,int& nPos, std::vector<std::vector<std::vector<long>>>& tables, std::vector<classifier>& classf, std::vector<feature>& feats );

void parTrain(int& nTasks,int& nPos, std::vector<std::vector<std::vector<long>>>& tables, std::vector<classifier>& classf, std::vector<feature>& feats);


//Q2.2+
bool error(classifier& classf, bool clas,feature& feat,std::vector<std::vector<long>>& sat);

int chooseClasf(double& error, std::vector<classifier>& classf, std::vector<double>& weights, std::vector<bool>& c, std::vector<feature>& feats, std::vector<std::vector<std::vector<long>>>& tables);

void updateWeights(std::vector<double>& weights,double& alfak,classifier& classf, std::vector<std::vector<std::vector<long>>>& tables, int& nTasks);

std::vector<double> boost();

int F(std::vector<double>& weights,std::vector<classifier>& classf, std::vector<std::vector<long>>& sat,double theta);

//TODO
/*fazer:
 * chooseClasf
 * updateWeights
 * MAIN
 *
 * conferir:
 * distII (readImgs)
 * calcFeat
 * train
 */


#endif
