#ifndef SIGMODEL
#define SIGMODEL

#include <string>
#include <sstream>

#include "RooCBShape.h"
#include "RooGaussian.h"
#include "RooFormulaVar.h"
#include "RooHistPdf.h"

#include "HZZ2L2QRooPdfs.h"

#include "Nuisance.h"
#include "M4lModel.h"

class SigModel : public M4lModel
{
public:
  void init(TString type_);
  SigModel(TString name_, TString channelStr_, RooRealVar *m4l_, TString type, RooDataHist *dhSigM4l, TString conf="");
  SigModel(TString name_, TString channelStr_, RooRealVar *m4l_, vector<pair<float,M4lModel*> > &yieldPdfs);
  ~SigModel();
  void makeNuisPdfs();
  void setConstant(bool constant=true);
  void fixNuisances(bool constant=true);
  void shiftParam(TString param, int updown);
  void setVal(TString name, float val);

  TString type;
  RooFormulaVar *mean,*sigma;
  RooCBShape *cb;
  RooDoubleCB *doubleCb;
  RooHistPdf *histPdf;
  RooFitResult *fitres;
  Nuisance *meanNuis,*sigmaNuis,*alphaNuis,*ennNuis;

private:
  float dm4lUncert; // width in gev
  float dsigmaUncert; // *fraction* of *value*, e.g. 0.1
};
#endif
