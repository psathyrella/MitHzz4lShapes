#ifndef NUISANCE
#define NUISANCE

#include "TString.h"
#include "RooRealVar.h"
#include "RooGaussian.h"

using namespace RooFit;

class Nuisance
{
public:
  Nuisance(TString name, RooRealVar *xvar_, bool useVarVals=true, double val=-99, double error=-99);
  RooRealVar *mean,*sigma,*xvar;
  RooGaussian *pdf;
};

#endif
