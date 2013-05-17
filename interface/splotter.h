#ifndef SPLOTTER
#define SPLOTTER

#include <sstream>

#include "RooGaussian.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooPlot.h"
#include "RooArgSet.h"
#include "RooPolynomial.h"
#include "RooAddPdf.h"
#include "RooConstVar.h"
#include "RooMsgService.h"
#include "RooExtendPdf.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooFitResult.h"
#include "RooCategory.h"
#include "RooSimultaneous.h"
#include "RooGenericPdf.h"
#include "RooProdPdf.h"

#include "RooStats/SPlot.h"

#include "TF1.h"
#include "TLegend.h"
#include "TFile.h"
#include "TH1F.h"
#include "TROOT.h"
#include "TH2F.h"
#include "TString.h"
#include "TRandom.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TMatrix.h"

#include "sinfo.h"
#include "SigModel.h"
#include "Nuisance.h"
#include "Model.h"

using namespace std;
using namespace RooFit;

//========================================================================================
class splotter
{
public:
  splotter(TString name_, TString config);
  ~splotter();
  void readConfig(TString config);
  void printRooFitResult(RooFitResult *fitres);

  map<TString,vector<sinfo> *> samples;
  map<TString,Model*> models;

  bool noData,absVal,unfold;
  double massMin,massMax,xMin,xMinPlot,xMax,m4lBinWidth,xBinWidth,cutVarMin,cutVarMax;
  int nBinsM4l,nBinsX,nDataMax;
  TString name,plotdir,xVarName,xVarTitle,cutVarName,cutVarMinStr,cutVarMaxStr;
  TString unfMethod;
  TString tmpFileName;

  RooRealVar *mass,*xVar,*N_S_Tot,*totWgt,*nSigExp_4e,*nSigExp_4m,*nSigExp_2e2m;

  RooStats::SPlot *sData,*sMc;
};

#endif
