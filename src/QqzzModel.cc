#include "QqzzModel.h"

//----------------------------------------------------------------------------------------
QqzzModel::QqzzModel(TString name_, TString channelStr_, RooRealVar *m4l_, TString conf, RooDataHist *dh):
  M4lModel(name_, channelStr_, m4l_),
  qqzzPdf(0)
{
  vars["a0" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a0",  "",  115.3,       0,   200);
  vars["a1" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a1",  "",   21.96,      0,   200);
  vars["a2" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a2",  "",  122.8,       0,   200);
  vars["a3" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a3",  "",    0.03479,   0,     1);
  vars["a4" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a4",  "",  185.5,       0,   200);
  vars["a5" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a5",  "",   12.67,      0,   200);
  vars["a6" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a6",  "",   34.81,      0,   100);
  vars["a7" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a7",  "",    0.1393,    0,     1);
  vars["a8" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a8",  "",   66,         0,   200);
  vars["a9" ] = new RooRealVar(name+"-"+channelStr_+"-"+"a9",  "",    0.07191,   0,     1);
  vars["a10"] = new RooRealVar(name+"-"+channelStr_+"-"+"a10", "",   94.11,      0,   200);
  vars["a11"] = new RooRealVar(name+"-"+channelStr_+"-"+"a11", "",   -5.111,  -100,   100);
  vars["a12"] = new RooRealVar(name+"-"+channelStr_+"-"+"a12", "", 4834,         0, 10000);
  vars["a13"] = new RooRealVar(name+"-"+channelStr_+"-"+"a13", "",    0.2543,    0,     1);

  qqzzPdf = new RooqqZZPdf_v2(name_, name_, *m4l_, // ugly ass way to pass parameters
			      *vars["a0" ],
			      *vars["a1" ],
			      *vars["a2" ],
			      *vars["a3" ],
			      *vars["a4" ],
			      *vars["a5" ],
			      *vars["a6" ],
			      *vars["a7" ],
			      *vars["a8" ],
			      *vars["a9" ],
			      *vars["a10"],
			      *vars["a11"],
			      *vars["a12"],
			      *vars["a13"]);
  pdf = qqzzPdf;
  readConfig(conf);
}
//----------------------------------------------------------------------------------------
QqzzModel::~QqzzModel()
{
  if(qqzzPdf) delete qqzzPdf;
  for(map<TString,RooRealVar*>::iterator it=vars.begin(); it!=vars.end(); it++) {
    delete (*it).second;
  }
}
