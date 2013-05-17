#ifndef CORREL
#define CORREL

#include <vector>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <math.h>
#include <stdlib.h>

#include "TGraph.h"
#include "TCanvas.h"

using namespace std;

// keeps track of quantities you need to calculate the correlation of two variables
class Correl
{
public:
  Correl();
  void fill(double x, double y);
  double calcPearson(bool print=true);
  vector<unsigned> makeRankVec(vector<float> &vec, vector<unsigned> &ranks);
  double calcSpearman(bool print=true);
  void plot(TString png="");

  int nEvts;
  double xSum,ySum;
  vector<float> xv,yv;
  vector<unsigned> xRanks,yRanks;
};

#endif
