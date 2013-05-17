#include "SigModel.h"

//----------------------------------------------------------------------------------------
void SigModel::init(TString type_)
{
  type		= type_;
  mean		= 0;
  sigma		= 0;
  cb		= 0;
  doubleCb	= 0;
  histPdf	= 0;
  fitres	= 0;
  meanNuis	= 0;
  sigmaNuis	= 0;
  alphaNuis	= 0;
  ennNuis	= 0;
  dm4lUncert	= .2;   // 1 gev in official carads
  dsigmaUncert	= .05; // 0.2 (20%) in official cards
}
//----------------------------------------------------------------------------------------
SigModel::SigModel(TString name_, TString channelStr_, RooRealVar *m4l_, TString type_, RooDataHist *dhSigM4l, TString conf):
  M4lModel(name_, channelStr_, m4l_)
{
  init(type_);
  
  // mean
  vars["m0"]	 = new RooRealVar("m0",  "",                  121, 131); // Want the same mean for all the channels
  vars["dm4l"]	 = new RooRealVar("dm4l","",                  -5,  5);  // Want the same mean for all the channels
  mean           = new RooFormulaVar("mean","", TString(vars["m0"]->GetName())+" + "+vars["dm4l"]->GetName(), RooArgList(*vars["m0"],*vars["dm4l"]));
  // width
  vars["s0"]	 = new RooRealVar("s0_"+channelStr_, "",      0,  5);  // But separate sigma, alpha, and enn for each
  vars["dsigma"] = new RooRealVar("dsigma_"+channelStr_, "",  -1,  1);  // But separate sigma, alpha, and enn for each
  sigma          = new RooFormulaVar("sigma_"+channelStr_,"", TString(vars["s0"]->GetName())+" + "+vars["dsigma"]->GetName(), RooArgList(*vars["s0"],*vars["dsigma"]));
  // tail
  vars["alpha"]	 = new RooRealVar("alpha_"+channelStr_, "",  0,  5);
  vars["enn"]	 = new RooRealVar("enn_"+channelStr_,   "",  0,  10);
  vars["alpha2"] = new RooRealVar("alpha2_"+channelStr_,"",  0,  5);
  vars["enn2"]	 = new RooRealVar("enn2_"+channelStr_,  "",  0,  20); // NOTE: changed limit from 10 to 20, need to make sure it doesn't make the fits fail

  if(conf!="") {
    readConfig(conf);
  } else {
    if(channelStr_=="4e") {
      vars["m0"]->setVal(   125.4);
      vars["s0"]->setVal(     2.36);
      vars["enn"]->setVal(    4.54);
      vars["alpha"]->setVal(  1.07);
      if(type=="doubleCb") {
	vars["enn"]->setVal(    6.7);
	vars["enn2"]->setVal(   7.4);
      }
    } else if(channelStr_=="4m") {
      vars["m0"]->setVal(   126);
      vars["s0"]->setVal(     1.65);
      vars["enn"]->setVal(    2.64);
      vars["alpha"]->setVal(  1.38);
      if(type=="doubleCb") {
	vars["enn"]->setVal(    3.0);
	vars["enn2"]->setVal(   4.8);
      }
    } else if(channelStr_=="2e2m") {
      vars["m0"]->setVal(   125.8);
      vars["s0"]->setVal(     1.94);
      vars["enn"]->setVal(    3.87);
      vars["alpha"]->setVal(  1.17);
      if(type=="doubleCb") {
	vars["enn"]->setVal(    9.79);
	vars["enn2"]->setVal(   2.47);
      }
    } else assert(0);
  }

  if(type=="singleCb") {
    cb = new RooCBShape(name_, name_, *m4l, *mean, *sigma, *vars["alpha"], *vars["enn"]);
    pdf = cb;
  } else if(type=="doubleCb") {
    doubleCb = new RooDoubleCB(name_, name_, *m4l, *mean, *sigma, *vars["alpha"], *vars["enn"], *vars["alpha2"], *vars["enn2"]);
    pdf = doubleCb;
  } else if(type=="template") {
    assert(dhSigM4l);
    histPdf = new RooHistPdf(name_,"", *m4l, *dhSigM4l);
    pdf = histPdf;
  } else { cout << "BAD TYPE: " << type << endl; assert(0); }
}
//----------------------------------------------------------------------------------------
SigModel::SigModel(TString name_, TString channelStr_, RooRealVar *m4l_, vector<pair<float,M4lModel*> > &yieldPdfs):
  M4lModel(name_, channelStr_, m4l_, yieldPdfs)
{
  init("doubleCb");
}
//----------------------------------------------------------------------------------------
SigModel::~SigModel()
{
  if(mean)         delete mean;
  if(sigma)        delete sigma;
  if(cb)	   delete cb;
  if(doubleCb)	   delete doubleCb;
  if(histPdf)	   delete histPdf;
  if(meanNuis)	   delete meanNuis;
  if(sigmaNuis)	   delete sigmaNuis;
  mean		= 0;
  sigma		= 0;
  cb		= 0;
  doubleCb	= 0;
  histPdf	= 0;
  meanNuis	= 0;
  sigmaNuis	= 0;
  
  for(map<TString,RooRealVar*>::iterator it=vars.begin(); it!=vars.end(); it++) {
    delete (*it).second;
  }
}
//----------------------------------------------------------------------------------------
void SigModel::setConstant(bool constant)
{
  if(type=="template") return;

  for(map<TString,RooRealVar*>::iterator it=vars.begin(); it!=vars.end(); it++) {
    (*it).second->setConstant(constant);
  }
}
//----------------------------------------------------------------------------------------
void SigModel::makeNuisPdfs()
{
  if(type=="template") return;

  if(!sumPdf) { // if we're just a sum of previous pdfs, we don't need to make new nuisances
    assert(vars.find("dm4l") != vars.end());
    assert(vars.find("dsigma") != vars.end());
    meanNuis  = new Nuisance("meanNuis_"+name,  vars["dm4l"],   false, 0, dm4lUncert);                   // 1 gev width
    sigmaNuis = new Nuisance("sigmaNuis_"+name, vars["dsigma"], false, 0, dsigmaUncert*sigma->getVal()); // 20% width
  }
  nuisancePdfs = new RooArgSet(name);
  if(!sumPdf) {
    nuisancePdfs->add(*meanNuis->pdf);
    nuisancePdfs->add(*sigmaNuis->pdf);
  } else {
    for(unsigned ic=0; ic<compPdfs.size(); ic++) { // NOTE: yieldVars not the same size as compPdfs
      compPdfs[ic]->makeNuisPdfs();
      nuisancePdfs->add(*compPdfs[ic]->nuisancePdfs);
    }
  }
}
//----------------------------------------------------------------------------------------
void SigModel::fixNuisances(bool constant)
{
  if(!sumPdf) {
    assert(vars.find("dm4l") != vars.end());
    assert(vars.find("dsigma") != vars.end());
    vars["dm4l"]->setConstant(constant);
    vars["dsigma"]->setConstant(constant);
  } else {
    for(unsigned ic=0; ic<compPdfs.size(); ic++) { // NOTE: yieldVars not the same size as compPdfs
      compPdfs[ic]->fixNuisances(constant);
    }
  }
}
//----------------------------------------------------------------------------------------
void SigModel::shiftParam(TString param, int updown)
// shift the parameter up/down by its one-sigma uncertainty
{
  assert(updown==-1 || updown==1);

  if(param=="mean") {
    bool wasConst(vars["dm4l"]->isConstant());
    float newVal(vars["dm4l"]->getVal() + updown*dm4lUncert);
    cout << "shifting dm4l: " << vars["dm4l"]->getVal() << " ---> " << newVal << endl;
    vars["dm4l"]->setConstant(false);
    vars["dm4l"]->setVal(newVal);
    vars["dm4l"]->setConstant(wasConst);
  } else if(param=="sigma") {
    bool wasConst(vars["dsigma"]->isConstant());
    float newVal(vars["dsigma"]->getVal() + updown*dsigmaUncert*sigma->getVal());
    cout << "shifting dsigma: " << vars["dsigma"]->getVal() << " ---> " << newVal << endl;
    vars["dsigma"]->setConstant(false);
    vars["dsigma"]->setVal(newVal);
    vars["dsigma"]->setConstant(wasConst);
  } else assert(0);
}
//----------------------------------------------------------------------------------------
void SigModel::setVal(TString name, float val)
{
  if(!sumPdf) {
    assert(vars.find(name) != vars.end());
    vars[name]->setVal(val);
  } else {
    for(unsigned ic=0; ic<compPdfs.size(); ic++) // NOTE: yieldVars not the same size as compPdfs
      compPdfs[ic]->setVal(name,val);
  }
}
