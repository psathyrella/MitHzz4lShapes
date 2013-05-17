#ifndef SHAPEHEADERS
#define SHAPEHEADERS

#include <cassert>
#include "TFile.h"
#include "TROOT.h"
#include "TLegend.h"
#include <iostream>
#include "TCanvas.h"
#include "TChain.h"
#include "TH1F.h"
#include "TString.h"
#include <vector>
#include "RooFit.h"
#include "RooRealVar.h"
using namespace std;
void scaleHist(TH1 *hist)
{
  double integral = hist->Integral(0,hist->GetNbinsX()+2);
  if(integral != 0)
    hist->Scale(1./ integral);
}
//----------------------------------------------------------------------------------------
TH1F* makeRatio(TH1F *hNumer, TH1F *hDenom)
{
  TH1F *ratio = new TH1F(*hNumer);
  ratio->Divide(hNumer,hDenom);
  for(int ibin=0; ibin<hNumer->GetNbinsX()+2; ibin++) {
    if(hNumer->GetBinContent(ibin)==0 || hNumer->GetBinContent(ibin)==0)
      ratio->SetBinContent(ibin,1);
  }
  return ratio;
}
//----------------------------------------------------------------------------------------
TH1F* makeOppo(TH1F *hStrange, TH1F *hce)
{
  TH1F *hoppo = new TH1F(*hStrange);
  hoppo->SetNameTitle("oppo","high;ZZ pT [GeV];");
  for(int ibin=0; ibin<hStrange->GetNbinsX()+2; ibin++) {
    float central = hce->GetBinContent(ibin);
    float strange = hStrange->GetBinContent(ibin);
    hoppo->SetBinContent(ibin,2*central - strange);
  }

  return hoppo;
}
//----------------------------------------------------------------------------------------
TH1F* makeEnvelopeHist(vector<TH1F*> &hists, TString which)
// if which is:
//        central: return hist with each bin the mean of the hists
//        lo:      return hist with each bin the lowest of the hists
//        hi:      same, but highest
// NOTE: this doesn't really give what you want -- after normalization the hi
//       and lo tend to be pretty central
{
  assert(hists.size()>0);
  TH1F *hnew = new TH1F(*hists[0]);
  hnew->SetNameTitle(which,which+";;");
  hnew->Reset();
  for(int ibin=0; ibin<hnew->GetNbinsX()+2; ibin++) {
    double tot(0),lo(0),hi(0);
    for(unsigned ih=0; ih<hists.size(); ih++) {
      double icont = hists[ih]->GetBinContent(ibin);
      tot += icont;
      if(ih==0 || icont < lo)
	lo = icont;
      if(ih==0 || icont > hi)
	hi = icont;
    }
    double val;
    if(which=="central") {
      val = tot/hists.size();
    } else if(which=="lo") {
      val = lo;
    } else if(which=="hi") {
      val = hi;
    } else assert(0);
      
    hnew->SetBinContent(ibin, val);
  }
  scaleHist(hnew);
  
  return hnew;
}
//----------------------------------------------------------------------------------------
TH1F* findStrangestHist(vector<TH1F*> &hists)
// find hist which deviates the most (by chi-2) from the central one
{
  assert(hists.size() > 0);
  TH1F *hce = makeEnvelopeHist(hists, "central");

  TH1F *hStrange=0;
  double maxChi2(0);
  for(unsigned ih=0; ih<hists.size(); ih++) {
    double chi2(0);
    for(int ibin=0; ibin<hce->GetNbinsX()+2; ibin++) {
      double icont = hists[ih]->GetBinContent(ibin);
      double icent = hce->GetBinContent(ibin);
      double diff = fabs(icont - icent);
      chi2 += diff*diff;
    }
    if(!hStrange || chi2 > maxChi2) {
      hStrange = hists[ih];
      maxChi2 = chi2;
    }
  }

  assert(hStrange);
  hStrange->SetNameTitle("Strange","low;;");
  return hStrange;
}

#endif      
