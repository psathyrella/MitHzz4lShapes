#ifndef UNFOLDER
#define UNFOLDER

#include <iostream>
#include <iomanip>
#include <cassert>
#include "TH1D.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "RooUnfold/src/RooUnfoldResponse.h"
#include "RooUnfold/src/RooUnfoldBayes.h"
#include "RooUnfold/src/RooUnfoldSvd.h"

using namespace std;

class Unfolder
{
public:
  Unfolder(TString name_, TString title_, TString plotdir_, TString xVarName_, TString xVarTitle_, int nBinsX, float xMin, float xMax);
  Unfolder(TString name_, TString title_, TString plotdir_, TString xVarName_, TString xVarTitle_, RooUnfoldResponse *unfResp_, TH1D *hRespX_, TH1D *hGenX_, TString method);
  ~Unfolder();
  void fill(int ievt, double xVar, double genXVar, double wgt);
  void unfold(TString method);
  RooUnfold *getUnf(TString method);
  void print();
  void draw();

  TString name,title,plotdir,xVarName,xVarTitle;
  TH1D *hGenX;  // gen distribution
  TH1D *hRespX; // response, i.e. reconstructed detector distribution
  TH1D *hUnfX; // unfolded back to gen level (i.e. 'reconstruction' of gen from reco)
  RooUnfoldResponse *unfResp;
  RooUnfoldBayes *bayes;
  RooUnfoldSvd *svd;
};

#endif
