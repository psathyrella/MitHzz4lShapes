#include "GgzzModel.h"

//----------------------------------------------------------------------------------------
GgzzModel::GgzzModel(TString name_, TString channelStr_, RooRealVar *m4l_, TString conf, RooDataHist *dh):
  M4lModel(name_, channelStr_, m4l_),
  ggzzPdf(0)
{
  vars["a0"] = new RooRealVar(name+"-"+channelStr_+"-"+"a0", "", 115.3,     0,    200);
  vars["a1"] = new RooRealVar(name+"-"+channelStr_+"-"+"a1", "",  21.96,    0,    200);
  vars["a2"] = new RooRealVar(name+"-"+channelStr_+"-"+"a2", "", 122.8,     0,    200);
  vars["a3"] = new RooRealVar(name+"-"+channelStr_+"-"+"a3", "",   0.03479, 0,      1);
  vars["a4"] = new RooRealVar(name+"-"+channelStr_+"-"+"a4", "", 185.5,     0,    200);
  vars["a5"] = new RooRealVar(name+"-"+channelStr_+"-"+"a5", "",  12.67,    0,    200);
  vars["a6"] = new RooRealVar(name+"-"+channelStr_+"-"+"a6", "",  34.81,    0,    100);
  vars["a7"] = new RooRealVar(name+"-"+channelStr_+"-"+"a7", "",   0.1393,  0,      1);
  vars["a8"] = new RooRealVar(name+"-"+channelStr_+"-"+"a8", "",  66,       0,    200);
  vars["a9"] = new RooRealVar(name+"-"+channelStr_+"-"+"a9", "",   0.07191, 0,      1);

  ggzzPdf = new RooggZZPdf_v2(name_, name_, *m4l,
			      *vars["a0"],
			      *vars["a1"],
			      *vars["a2"],
			      *vars["a3"],
			      *vars["a4"],
			      *vars["a5"],
			      *vars["a6"],
			      *vars["a7"],
			      *vars["a8"],
			      *vars["a9"]);
  pdf = ggzzPdf;
  readConfig(conf);
}
//----------------------------------------------------------------------------------------
GgzzModel::~GgzzModel()
{
  if(ggzzPdf) delete ggzzPdf;
  for(map<TString,RooRealVar*>::iterator it=vars.begin(); it!=vars.end(); it++) {
    delete (*it).second;
  }
}
