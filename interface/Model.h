#ifndef MODEL
#define MODEL

#include <sstream>
#include <tuple>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <stdio.h>

#include "RooGaussian.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooAbsReal.h"
#include "RooPlot.h"
#include "RooArgSet.h"
#include "RooPolynomial.h"
#include "RooAddPdf.h"
#include "RooConstVar.h"
#include "RooExtendPdf.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooFitResult.h"
#include "RooWorkspace.h"

#include "RooStats/SPlot.h"

#include "TF1.h"
#include "TLegend.h"
#include "TFile.h"
#include "TH1D.h"
#include "TROOT.h"
#include "TH2D.h"
#include "TString.h"
#include "TRandom.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TMatrix.h"

#include "varkeepter.h"
#include "CPlot.h"
#include "MitStyleRemix.h"
#include "sinfo.h"
#include "Nuisance.h"
#include "Splotholder.h"
#include "UpDownHists.h"
#include "Unfolder.h"
#include "Correl.h"
#include "SigModel.h"
#include "QqzzModel.h"
#include "GgzzModel.h"
#include "LljjModel.h"

using namespace std;
using namespace RooFit;

//========================================================================================
class Model
{
public:
  Model(TString name_, TString channelStr_, TString config, RooRealVar *mass_, RooRealVar *xVar_, RooFormulaVar *N_S_, RooRealVar *totWgt);
  ~Model();
  void readConfig(TString config);
  void readMcFiles();
  void makeMcShapes();
  void unfoldEm();
  void readDataFiles();
  void printRooFitResult(RooFitResult *fitres);
  double smear(double xt);
  void addScaleResUncertBand(Splotholder *sCe, Splotholder *sLo, Splotholder *sHi);
  M4lModel *getAnalyticPdf(sinfo &si);
  void plotComponent(TString name, RooAbsPdf *pdf, RooPlot *fr, int icolor, TLegend *leg, vector<TH1D*> &tmpHists);
  void printEvtsPerBin(TH1D *hist=0);

  map<TString,vector<sinfo> *> samples;

  bool noData,unfold,doCorrel,absVal,scaResErr,debug;
  double massMin,massMax,xMin,xMinPlot,xMax,m4lBinWidth,xBinWidth,cutVarMin,cutVarMax;
  double nSigExp,nBkgExp;
  int nBinsM4l,nBinsX,nDataMax;
  unsigned channel;
  TString name,plotdir,label,massMinStr,massMaxStr,xMinStr,xMaxStr,channelStr,xVarName,xVarTitle,cutVarName,cutVarMinStr,cutVarMaxStr;
  TString sigModType,unfMethod,wsOffic;
  TTree *tBothMc,*tdata;
  TString tmpFileName;
  TFile *tmpFile;

  RooFormulaVar *N_S; // signal yield for the pdfs for the simultaneous fit between all channels
  RooRealVar *mass,*xVar,*N_S_Local,*N_BG,*totWgt; // N_S_Local is for the copy of splot local to this Model, i.e. yield is not constrained by other channel yields
  Nuisance *N_SNuis,*N_BGNuis;
  RooArgSet *Nuisances;
  RooDataSet *dsDataM4l,*dsData2d,*dsMcM4l,*dsMc2d,*toyDataM4l,*toyData2d;

  TH1D *hNull,*hEvtsPerBin;
  TH1D *hSigM4l,*hSigX,*hSigGenX,*hBkgM4l,*hBkgX,*hBkgGenX,*hSigChk,*hBkgChk;
  TH2D *hSig2d,*hBkg2d;
  RooDataHist *dhSigM4l,*dhSig2d,*dhBkgM4l,*dhBkg2d;
  SigModel *pdfSigM4l;
  RooHistPdf *pdfSig2d/*,*pdfBkgM4l*/,*pdfBkg2d;
  M4lModel *pdfBkgM4l;
  RooExtendPdf *pdfSigM4l_ext,*pdfSigM4l_ext_Local,*pdfSig2d_ext,*pdfBkgM4l_ext,*pdfBkg2d_ext;
  RooAddPdf *totPdfM4l,*totPdfM4l_Local,*totPDF2d;
  RooFitResult *fitresData;

  Splotholder *sData,*sMc;
  UpDownHists *wonErr,*woffErr,*renfacErr,*pdfErr,*hresErr;

  Unfolder *sigUnfolder,*bkgUnfolder,*allUnfolder;
  Correl *corr;

  map<TString,vector<pair<float,M4lModel*> > > analyticPdfs; // expected yield and pdf for each component. Parameters are read from a txt file whose info is gleaned from the official workspaces
};

#endif
