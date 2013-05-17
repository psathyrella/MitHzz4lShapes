#ifndef SPLOTHOLDER
#define SPLOTHOLDER

#include <vector>
#include <utility>
#include <sstream>
#include <tuple>

#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TH1D.h"
#include "TLine.h"
#include "RooStats/SPlot.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooArgList.h"

#include "UpDownHists.h"
#include "Unfolder.h"

class Splotholder
{
public:
  Splotholder(TString name_, TString channel_, TString plotdir_, RooRealVar *discr, RooRealVar *xvar_, int nBinsX, float xMin, float xMax,
	      RooDataSet *dsDiscr, RooAbsPdf *pdf, RooDataSet *ds2d_,
	      RooArgList &yieldVars_,
	      vector<tuple<TString,TString,TString,TString,TH1D*,TH1D*> > &components_,
	      vector<UpDownHists*> &errors_);
  ~Splotholder();
  // void fillResultHists(RooUnfoldResponse *unfResp);
  void fillResultHists(vector<TString> &subChans, map<TString,RooUnfoldResponse*> &unfResps, TString unfMethod);
  TH1D *unfoldHist(TH1D *hist, RooUnfoldResponse* unfResp, TString unfMethod);
  vector<TH1D*>& getResultV(bool unfolded=false);
  vector<TH1D*>& getOrigV(bool unfolded=false);
  void prepOrigHist(TH1D *hist, int color, double &ymax);
  void prepResultHist(TH1D *hist, int color, double &ymax);
  TH1D getChi2(TString name, TH1D *h1, TH1D *h2, double &chi, double &yminChi2, double &ymaxChi2);
  void combineEach(vector<TH1D*> &hists, vector<TH1D*> &combHists);
  void combineChannels();
  void plot(Splotholder *sp2=0, bool unfolded=false);
  void setMcErrors();

  TString name,plotdir,channel;
  int nBinsX;
  float xMin,xMax;
  RooRealVar *xvar;
  RooDataSet *ds2d;
  vector<tuple<TString,TString,TString,TString,TH1D*,TH1D*> > *components;
  RooArgList *yieldVars;
  vector<UpDownHists*> *errors;
  RooStats::SPlot *splot;
  vector<TString> chans;
  vector<TString> fillComboStrs; // will a separate hist with each channel for this hist, then combine them? Or only fill with a single channel. Yes, I know this is getting too damn complicated.
  vector<TString> plotComboStrs; // combine this hist together in which category?
  vector<RooRealVar*> yieldVarV;
  vector<TH1D*> hOrigV,hGenOrigV,hResultV,hUnfResultV,hCombOrigV,hCombGenOrigV,hCombResultV,hCombUnfResultV;
  vector<TH1D*> histsToDelete; // keep track of hists we need to delete in the destructor
};

#endif
