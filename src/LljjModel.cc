#include "LljjModel.h"

//----------------------------------------------------------------------------------------
LljjModel::LljjModel(TString name_, TString channelStr_, RooRealVar *m4l_, TString conf, RooDataHist *dh):
  M4lModel(name_, channelStr_, m4l_),
  lljjPdf(0)
{
  vars["mean"]  = new RooRealVar(name+"-"+channelStr_+"-"+"mean",  "", 0, 300);
  vars["sigma"] = new RooRealVar(name+"-"+channelStr_+"-"+"sigma", "", 0, 100);

  lljjPdf = new RooLandau(name_, name_, *m4l_, *vars["mean"], *vars["sigma"]);
  pdf = lljjPdf;
  readConfig(conf);
}
//----------------------------------------------------------------------------------------
LljjModel::~LljjModel()
{
  if(lljjPdf) delete lljjPdf;
  for(map<TString,RooRealVar*>::iterator it=vars.begin(); it!=vars.end(); it++) {
    delete (*it).second;
  }
}
