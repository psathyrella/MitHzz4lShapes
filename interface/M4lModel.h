#ifndef M4LMODEL
#define M4LMODEL

#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <map>

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooAddPdf.h"
#include "RooHistPdf.h"

using namespace RooFit;
using namespace std;

class M4lModel
// NOTE: need to merge these pdf classes with those in MitHzz4l/Limit, but for now they've got some inconsistenvies so I'll put it off
{
public:
  void init(TString name_, TString channelStr_, RooRealVar *m4l_);
  M4lModel(TString name_, TString channelStr_, RooRealVar *m4l_);
  M4lModel(TString name_, TString channelStr_, RooRealVar *m4l_, RooDataHist *dh);
  M4lModel(TString name_, TString channelStr_, RooRealVar *m4l_, vector<pair<float,M4lModel*> > &yieldPdfs);
  ~M4lModel();
  void readConfig(TString conf);
  virtual void makeNuisPdfs() {}
  virtual void fixNuisances(bool constant) {}
  virtual void setVal(TString name, float val) {}
  void dump();

  TString name,channelStr;
  RooRealVar *m4l; // in general we don't own this
  map<TString,RooRealVar*> vars;
  RooAbsPdf *pdf; // don't own this one since we don't create it here
  RooAddPdf *sumPdf; // we do own this though
  RooHistPdf *hp; // ...aaaaaaaand this one
  vector<RooRealVar*> yieldVars; // coefficients to each component of RooAddPdf
  vector<M4lModel*> compPdfs;
  vector<TString> compNames;
  RooArgSet *nuisancePdfs;
};

#endif
