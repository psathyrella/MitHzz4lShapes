#ifndef LLJJMODEL
#define LLJJMODEL

#include "RooLandau.h"

#include "M4lModel.h"

class LljjModel: public M4lModel
{
public:
  LljjModel(TString name_, TString channelStr_, RooRealVar *m4l_, TString conf="", RooDataHist *dh=0);
  ~LljjModel();
  void makeNuisPdfs() {}
  void fixNuisances(bool constant) {}
  void setVal(TString name, float val) {}

  RooLandau *lljjPdf;
};

#endif
