#include "Nuisance.h"

Nuisance::Nuisance(TString name, RooRealVar *xvar_, bool useVarVals, double val, double error):
  xvar(xvar_)
{
  assert(xvar);
  mean  = new RooRealVar("mean_"+name,"",  (useVarVals) ? xvar->getVal() : val);
  mean->setConstant();
  sigma = new RooRealVar("sigma_"+name,"", (useVarVals) ? xvar->getError() : error);
  sigma->setConstant();
  pdf = new RooGaussian("pdf_"+name,"", *xvar, *mean, *sigma);
}
