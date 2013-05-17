#ifndef QQZZMODEL
#define QQZZMODEL

#include "M4lModel.h"
#include "HZZ4LRooPdfs.h"

class QqzzModel : public M4lModel
// NOTE: need to merge these pdf classes with those in MitHzz4l/Limit, but for now they've got some inconsistenvies so I'll put it off
{
public:
  QqzzModel(TString name_, TString channelStr_, RooRealVar *m4l_, TString conf="", RooDataHist *dh=0);
  ~QqzzModel();
  void makeNuisPdfs() {}
  void fixNuisances(bool constant) {}
  void setVal(TString name, float val) {}

  RooqqZZPdf_v2 *qqzzPdf;
};

#endif
