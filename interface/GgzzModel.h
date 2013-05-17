#ifndef GGZZMODEL
#define GGZZMODEL

#include "HZZ4LRooPdfs.h"

#include "M4lModel.h"

class GgzzModel: public M4lModel
{
public:
  GgzzModel(TString name_, TString channelStr_, RooRealVar *m4l_, TString conf="", RooDataHist *dh=0);
  ~GgzzModel();
  void makeNuisPdfs() {}
  void fixNuisances(bool constant) {}
  void setVal(TString name, float val) {}

  RooggZZPdf_v2 *ggzzPdf;
};

#endif
