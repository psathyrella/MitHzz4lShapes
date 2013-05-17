#ifndef UPDOWNHISTS
#define UPDOWNHISTS

#include "RooRealVar.h"
#include "RooDataHist.h"
#include "TH1F.h"

using namespace std;

class UpDownHists
{
public:
  UpDownHists(TString name, TString weight, TString var, float varLo, float varHi, int nBinsVar);
  bool fill(double val, double wCe, double wLo, double wHi);
  void makeErrorHists(); // Make histograms whose content in each bin is the size of the
                         // uncertainty in the normalized cross section from this source

  TString var;
  TH1F *hCe,*hLo,*hHi;
  TH1F *hErrLo,*hErrHi;
};

#endif
