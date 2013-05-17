#include "M4lModel.h"

//----------------------------------------------------------------------------------------
void M4lModel::init(TString name_, TString channelStr_, RooRealVar *m4l_)
{
  name         = name_;
  channelStr   = channelStr_;
  m4l          = m4l_;
  pdf          = 0;
  sumPdf       = 0;
  hp           = 0;
  nuisancePdfs = 0;
}
//----------------------------------------------------------------------------------------
M4lModel::M4lModel(TString name_, TString channelStr_, RooRealVar *m4l_)
{
  M4lModel::init(name_, channelStr_, m4l_);
}
//----------------------------------------------------------------------------------------
M4lModel::M4lModel(TString name_, TString channelStr_, RooRealVar *m4l_, RooDataHist *dh)
{
  M4lModel::init(name_, channelStr_, m4l_);
  hp = new RooHistPdf("dh-"+name,"", *m4l_, *dh);
  pdf = hp;
}
//----------------------------------------------------------------------------------------
M4lModel::M4lModel(TString name_, TString channelStr_, RooRealVar *m4l_, vector<pair<float,M4lModel*> > &yieldPdfs)
{
  M4lModel::init(name_, channelStr_, m4l_);
  RooArgList coeffs;
  RooArgList pdfs;
  float total(0);
  for(unsigned ic=0; ic<yieldPdfs.size(); ic++) total += yieldPdfs[ic].first;
  for(unsigned ic=0; ic<yieldPdfs.size(); ic++) {
    if(ic < yieldPdfs.size()-1) {
      // need the RooAddPdf below to *not* be extended, so that we can make the extended pdf later with N_S
      RooRealVar *tmpvar = new RooRealVar(TString("coeff")+"-"+yieldPdfs[ic].second->name, "", yieldPdfs[ic].first / total); // set constant to *ratio* of expected yields (divide by first one)
      coeffs.add(*tmpvar);
      yieldVars.push_back(tmpvar);
    }
    pdfs.add(*yieldPdfs[ic].second->pdf);
    compPdfs.push_back(yieldPdfs[ic].second);
    compNames.push_back(yieldPdfs[ic].second->pdf->GetName());
  }
  
  sumPdf = new RooAddPdf("sum-"+name_, "", pdfs, coeffs);
  pdf = sumPdf;
}
//----------------------------------------------------------------------------------------
M4lModel::~M4lModel()
{
  if(sumPdf)       delete sumPdf;
  if(hp)           delete hp;
  if(nuisancePdfs) delete nuisancePdfs;
  sumPdf        = 0;
  hp            = 0;
  nuisancePdfs	= 0;
}
//----------------------------------------------------------------------------------------
void M4lModel::readConfig(TString conf)
{
  ifstream ifs(conf);
  if(!ifs.is_open()) {
    cout << "ERROR: file " << conf << " does not exist" << endl;
    assert(0);
  }
  string line;
  while(getline(ifs,line)) {
    if(line[0]=='#') continue;
    stringstream ss(line);
    TString varname;
    float varval;
    ss >> varname >> varval;
    assert(vars.find(varname) != vars.end());
    vars[varname]->setVal(varval);
    vars[varname]->setConstant();
  }
}
//----------------------------------------------------------------------------------------
void M4lModel::dump()
{
  cout << "M4lModel::dump() " << name << endl;
  pdf->Print();
  for(map<TString,RooRealVar*>::iterator it=vars.begin(); it!=vars.end(); it++) {
    cout << (*it).second->isConstant() << "  ";
    (*it).second->Print();
  }  
}
