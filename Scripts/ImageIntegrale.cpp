#include <iostream>
#include <vector>
#include <cstdlib>
#include <sstream>
#include "CImg.h"
#include "ImageIntegrale.h"
#include "math.h"
#include <thread>
#include <ctime>


void printData(std::vector<std::vector<long> >& data) {
	using namespace std;
	
    for (vector<vector<long> >::iterator it = data.begin(); it != data.end();
         ++it) {
      // loop over columns
      for (vector<long>::iterator jt = it->begin(); jt != it->end(); ++jt) {
        // separate fields using spaces
        cout << " " << *jt;
      }
      cout << endl;
    }
}


std::vector<std::vector<long>> SAT(const cimg_library::CImg<long>& img){
    using namespace cimg_library;
	int height=img.height(),width=img.width();
    std::vector<std::vector<long> > sat(width, std::vector<long>(height,0));
    long init=0;

    for(int y=0;y<height;y++){//init first row
        init+=img(0,y,0);
        sat[0][y] = init;
    }

    for (int x=1; x<width; x++) {
        //row initialization
        sat[x][0] = sat[x-1][0] + img(x,0,0);
        
        //column iteration
        for (int y=1; y<height; y++)
			sat[x][y] = img(x,y,0) + sat[x-1][y] + sat[x][y-1] - sat[x-1][y-1];

    }
    return sat;
}


feature::feature(int xx, int yy, int ww,int hh,char c){
	x=xx;
	y=yy;
	w=ww;
	h=hh;
	type=c;
}


long calcFeat(std::vector<std::vector<long>>& sat, feature& f){
	switch(f.type){
	case 'a':
		return 2*sat[f.x+f.w/2][f.y]-2*sat[f.x+f.w/2][f.y+f.h]-sat[f.x][f.y]+sat[f.x][f.y+f.h]-sat[f.x+f.w][f.y]+sat[f.x+f.w][f.y+f.h];

	case 'b':
		return 2*sat[f.x+f.w][f.y+f.h/2]-2*sat[f.x][f.y+f.h/2]+sat[f.x][f.y]+sat[f.x][f.y+f.h]-sat[f.x+f.w][f.y]-sat[f.x+f.w][f.y+f.h];

	case 'c':
		return -sat[f.x][f.y]+sat[f.x][f.y+f.h]+sat[f.x+f.w][f.y]-sat[f.x+f.w][f.y+f.h]+2*sat[f.x+f.w/3.0][f.y]-2*sat[f.x+f.w/3.0][f.y+f.h]-2*sat[f.x+2*f.w/3.0][f.y]+2*sat[f.x+2*f.w/3.0][f.y+f.h];

	case 'd':
		return 2*(sat[f.x+f.w/2][f.y]+sat[f.x+f.w/2][f.y+f.h]+sat[f.x][f.y+f.h/2]+sat[f.x+f.w][f.y+f.h/2])-4*sat[f.x+f.w/2][f.y+f.h/2]-sat[f.x][f.y]-sat[f.x+f.w][f.y+f.h]-sat[f.x+f.w][f.y]-sat[f.x][f.y+f.h];
	}
	return 0;
}


void featVect(std::vector<feature>& feats, char type,int& wMax, int& hMax){
	int wMin,hMin, wInc,hInc;
	switch(type){
	case 'a':
		wMin=15;
		wInc=8;
		hMin=7;
		hInc=4;
		break;
	case 'b':
		wMin=7;
		wInc=4;
		hMin=15;
		hInc=8;
		break;
	case 'c':
		wMin=23;
		wInc=12;
		hMin=7;
		hInc=4;
		break;
	case 'd':
		wMin=15;
		wInc=8;
		hMin=15;
		hInc=8;
	}
	for(int w=wMin;w<wMax;w+=wInc){
		for(int h=hMin;h<hMax;h+=hInc){
			for(int x=0;x<wMax-w;x+=4){
				for(int y=0;y<hMax-h;y+=4){
					feats.push_back(feature(x,y,w,h,type));
                }
			}
		}
	}
}


std::vector<feature> distFeat(int& width, int& height){
	std::vector<feature> fA,fB,fC,fD;

	std::thread t1(featVect,std::ref(fA),'a',std::ref(width),std::ref(height));
	std::thread t2(featVect,std::ref(fB),'b',std::ref(width),std::ref(height));
	std::thread t3(featVect,std::ref(fC),'c',std::ref(width),std::ref(height));
	std::thread t4(featVect,std::ref(fD),'d',std::ref(width),std::ref(height));

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	fA.insert(fA.end(),fB.begin(),fB.end());
	fA.insert(fA.end(),fC.begin(),fC.end());
	fA.insert(fA.end(),fD.begin(),fD.end());
	return fA;
}


classifier::classifier(){
	w1=1;
	w2=0;
}


int classifier::calc(double x){
	if(w1*x+w2>=0)
		return 1;
	else
		return -1;
}


void readImgs(int& nTasks, int taskId,bool pos,int& nPos, std::vector<std::vector<std::vector<long>>>& iis ,std::string rep){
	using namespace cimg_library;
	CImg<long> img;
	std::stringstream ss;
	if(pos){
		for(int i=taskId;i<nPos;i+=nTasks){
			ss.str(std::string());
			ss<<rep<<"im"<<i<<".jpg";
			img.load(ss.str().c_str());
			iis[i]=SAT(img);
		}
	}
	else{
		for(int i=nPos+taskId;i<iis.size();i+=nTasks){
			ss.str(std::string());
			ss<<rep<<"im"<<i-nPos<<".jpg";
			img.load(ss.str().c_str());
			iis[i]=SAT(img);
		}
	}

}


std::vector<std::vector<std::vector<long> > > distII(int& nTasks, int& nPos,int& nNeg, std::string rep){
	std::vector<std::vector<std::vector<long> > > ii(nPos+nNeg);
	std::vector<std::thread> pos,neg;
	for(int i=0;i<nTasks;i++)
			pos.push_back(std::thread(readImgs,std::ref(nTasks),i,true,std::ref(nPos),std::ref(ii),rep+"/pos/"));

	for(int i=0;i<nTasks;i++){
			pos[i].join();
			neg.push_back(std::thread(readImgs,std::ref(nTasks),i,false,std::ref(nPos),std::ref(ii),rep+"/neg/"));
	}

	for(int i=0;i<nTasks;i++)
			neg[i].join();

	return ii;
}


void train(int& nTasks, int taskId,int& nPos, std::vector<std::vector<std::vector<long>>>& tables,
		std::vector<classifier>& classf, std::vector<feature>& feats, double& eps, int& K ){
	srand(taskId+time(NULL));
	double r,xki;
	int rr,c,h;

	for(int i=taskId;i<feats.size();i+=nTasks){ //each classifier is trained in K-steps
		for(int k=0;k<K;k++){
			r=(double) rand()/(double)RAND_MAX;
			rr=(int)(r*tables.size());
            
			xki=calcFeat(tables[rr],feats[i]);
			h=classf[i].calc(xki);

			if(rr<nPos)
				c=1;
			else
				c=-1;
            
			classf[i].w1-=eps*(h-c)*xki;
			classf[i].w2-=eps*(h-c);
		}
	}
}


void parTrain(int& nTasks,int& nPos, std::vector<std::vector<std::vector<long>>>& tables,
		std::vector<classifier>& classf, std::vector<feature>& feats , double& eps, int& K){
	std::vector<std::thread> threads;
	for(int i=0;i<nTasks;i++)
		threads.push_back(std::thread(train,std::ref(nTasks),i,std::ref(nPos),std::ref(tables),std::ref(classf),std::ref(feats),std::ref(eps),std::ref(K)));

	for(int i=0;i<nTasks;i++)
		threads[i].join();
}


bool error(classifier& classf, int c,feature& feat,std::vector<std::vector<long>>& sat){
	return classf.calc(calcFeat(sat,feat))!=c ;
}

void chooseClasf(int& nTasks, int taskId, int& nPos, double& currerr,int& ind,
std::vector<classifier>& classf, std::vector<double>& weights,
                 std::vector<feature>& feats, std::vector<std::vector<std::vector<long>>>& tables){

	// Modifies error and ind to the
	// min err and his index
	double locerr = 0;
	
	for(int i = taskId; i < classf.size(); i+=nTasks)
	{
		// Initialize the weighted error
		for(int j = 0; j < weights.size(); ++j)
		{
			if(j<nPos) { /////////////////// CHANGED HERE from (i<nPos) to (j<nPos)
				if(error(classf[i], 1 , feats[i], tables[j]))
					locerr += weights[j];
			} else {
				if(error(classf[i], -1 , feats[i], tables[j]))
					locerr += weights[j];
			}

		}
		
		
		if(i==taskId) { // Initialize error
			currerr = locerr;
			ind = taskId;
		} else if(locerr < currerr) { // Take the min
			currerr = locerr;
			ind = i;			
		}
		
		// reinit
		locerr = 0;
	}
}

int parChooseClasf(int& nTasks, int& nPos, double& currerr,
std::vector<classifier>& classf, std::vector<double>& weights,
std::vector<feature>& feats, std::vector<std::vector<std::vector<long>>>& tables) {
	int ind=0;
	std::vector<std::thread> threads;
	std::vector<double> err(nTasks); // vector of min err for every task
	std::vector<int> indices(nTasks); // Vector of argmin (error)
	
	for(int i=0;i<nTasks;i++)
		threads.push_back(std::thread(chooseClasf,std::ref(nTasks),i,std::ref(nPos),std::ref(err[i]),std::ref(indices[i]),std::ref(classf),std::ref(weights),std::ref(feats),std::ref(tables)));

	for(int i=0;i<nTasks;i++)
		threads[i].join();
	
	for(int i = 0; i < nTasks; ++i)
	{
		if(i==0) { // Init
			currerr = err[i];
			ind = indices[i];
		} else if(err[i] < currerr) { // Take the min
			currerr = err[i]; // Update error
			ind = indices[i]; // Update argmin(error)
		}
	}
	
	return ind;
}

void updateWeights(int& nTasks, int Taskid,std::vector<double>& weights,double& alfak,classifier& classf, std::vector<std::vector<std::vector<long>>>& tables, int nPos, std::vector<feature>& feats, int clas){
    for(int i = Taskid; i < weights.size(); i+=nTasks)
	{
		if(i<nPos){
			if(error(classf,1,feats[clas],tables[i]))
				weights[i] *= exp(alfak);
			else
				weights[i] *= exp(-alfak);
		}
		else{
			if(error(classf,-1,feats[clas],tables[i]))
				weights[i] *= exp(alfak);
			else
				weights[i] *= exp(-alfak);
		}
	}
}

void parUpdateWeights(std::vector<double>& weights,double& alfak,classifier& classf, std::vector<std::vector<std::vector<long>>>& tables, int& nTasks, int& Npos, std::vector<feature>& feats, int& clas){
	std::vector<std::thread> threads;

	for(int i=0;i<nTasks;i++)
		threads.push_back(std::thread(updateWeights, std::ref(nTasks),i,std::ref(weights),std::ref(alfak),std::ref(classf), std::ref(tables), Npos, std::ref(feats), clas));
	
	for(int i=0;i<nTasks;i++)
		threads[i].join();
	
	double sum = 0; // compute sum to normalise
	for(int i = 0; i < weights.size(); ++i)
	{
		sum+= weights[i];
	}
	
	for(int i = 0; i < weights.size(); ++i)
	{ // normalising weigths
		weights[i] /= sum;
	}
}

std::vector<double> boost(int& nTasks,int& nPos, std::vector<classifier>& classf,std::vector<std::vector<std::vector<long>>>& tables, std::vector<feature>& feats){
	std::vector<double> weights(tables.size(),1.0/(2*tables.size()));
	std::vector<double> f(classf.size(),0);
	int clas=0;
	double error,alfak;
	int N=13;  ////Paramet
	std::cout << "Boost steps:" << std::endl;
	for(int k=0;k<N;k++){
		std::cout << "Step : " << k << "/" << N << std::endl;
		clas=parChooseClasf(nTasks,nPos,error,classf,weights,feats,tables);
		std::cout<<"Error:"<<error<<std::endl;
		std::cout<<"Index: "<<clas<<std::endl;
		alfak=log((1-error)/error)/2;
		std::cout<<"AlfaK: "<<alfak<<std::endl;
		f[clas]+=alfak;
		parUpdateWeights(weights,alfak,classf[clas],tables, nTasks,nPos,feats,clas);
	}
	return f;
}

int F(std::vector<double>& alfa, std::vector<feature>& feats,std::vector<classifier>& classf, std::vector<std::vector<long>>& sat,double theta){
	double fNx=0,f=0,w=0,x;
	int h;

	for(int i=0;i<feats.size();i++){
		w=alfa[i];

		if(w){
			f+=w;
			x=calcFeat(sat,feats[i]);
			h=classf[i].calc(x);
			fNx+=w*h;
		}
	}

	if(fNx>=theta*f)
		return 1;
	else
		return -1;
}

void parF(int& nTasks, int taskId, int& nPos, std::vector<long>& fauxNP, std::vector<double>& weights,std::vector<feature>& feats,std::vector<classifier>& classf, std::vector<std::vector<std::vector<long>>>& sats,double theta){
	for(int i=taskId;i<sats.size();i+=nTasks){
		if(i<nPos){
			if(F(weights,feats,classf,sats[i],theta)==-1)
				fauxNP[0]++;//fauxNP[0] = n false negatives
		}
		else{
			if(F(weights,feats,classf,sats[i],theta)==1)
				fauxNP[1]++;//fauxNP[1] = n false positives
		}
	}
}

std::vector<long> test(int& nTasks, std::vector<double>& weights, std::vector<feature>& feats, int& nPos,
		std::vector<classifier>& classf, std::vector<std::vector<std::vector<long>>>& sats,double theta){
	std::vector<std::thread> threads;
	std::vector<std::vector<long>> fauxNP(nTasks,std::vector<long>(2,0));

	for(int i=0;i<nTasks;i++)
		threads.push_back(std::thread(parF,std::ref(nTasks),i,std::ref(nPos),std::ref(fauxNP[i]),std::ref(weights),std::ref(feats),std::ref(classf),std::ref(sats),theta));

	for(int i=0;i<nTasks;i++)
		threads[i].join();

	for(int i=1;i<nTasks;i++){
		fauxNP[0][0]+=fauxNP[i][0];
		fauxNP[0][1]+=fauxNP[i][1];
	}

	return fauxNP[0];
}




