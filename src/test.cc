#include <sstream>

#include "RooGaussian.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooPlot.h"
#include "RooArgSet.h"
#include "RooPolynomial.h"
#include "RooAddPdf.h"
#include "RooConstVar.h"
#include "RooExtendPdf.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooFitResult.h"
#include "RooCategory.h"
#include "RooSimultaneous.h"
#include "RooGenericPdf.h"
#include "RooProdPdf.h"

#include "RooStats/SPlot.h"

#include "TF1.h"
#include "TLegend.h"
#include "TFile.h"
#include "TH1F.h"
#include "TROOT.h"
#include "TH2F.h"
#include "TString.h"
#include "TRandom.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TMatrix.h"

#include "sinfo.h"
#include "SigModel.h"
#include "Nuisance.h"
#include "Model.h"

using namespace std;
using namespace RooFit;

int main()
{
  TString plotdir("~/www/ShapesPlots/splot/test/plots");
  TString name("arg");

  RooRealVar xvar("xvar","", 1, 5);
  xvar.setBins(20);
  RooRealVar discrvar("discrvar","", 1, 5);
  discrvar.setBins(20);
  // sig 1
  RooRealVar mvar("mvar","", 2.5);
  RooRealVar svar("svar","", 0.5);
  RooGaussian gaus("gaus","", discrvar, mvar, svar);
  RooGenericPdf xSigPdf("xSigPdf","", "1/(3+xvar)", xvar);
  RooProdPdf sigPdf("sigPdf","", RooArgList(gaus,xSigPdf));
  RooRealVar nSig("nSig","",10,-100,100);
  nSig.removeRange();
  RooExtendPdf sig("sig","", sigPdf, nSig);
  // sig 2
  RooRealVar m2var("m2var","", 4);
  RooRealVar s2var("s2var","", 0.2);
  RooGaussian gaus2("gaus2","", discrvar, m2var, s2var);
  RooGenericPdf xSig2Pdf("xSig2Pdf","", "1/(80+xvar)", xvar);
  RooProdPdf sig2Pdf("sig2Pdf","", RooArgList(gaus2,xSig2Pdf));
  RooRealVar nSig2("nSig2","",10,-100,100);
  nSig2.removeRange();
  RooExtendPdf sig2("sig2","", sig2Pdf, nSig2);
  // bkg 1
  RooGenericPdf discrBkg1("discrBkg1","", "1/discrvar",     discrvar);
  RooGenericPdf xBkg1("xBkg1","", "1/(-.85+xvar)", xvar);
  RooProdPdf bkg1Pdf("bkg1Pdf","", RooArgList(discrBkg1,xBkg1));
  RooRealVar nBkg1("nBkg1","",10,-100,100);
  nBkg1.removeRange();
  RooExtendPdf bkg1("bkg1","", bkg1Pdf, nBkg1);
  // bkg 1
  RooGenericPdf discrBkg2("discrBkg2","", "1/(1+discrvar)", discrvar);
  RooGenericPdf xBkg2("xBkg2","", "1/(-.6+xvar)", xvar);
  RooProdPdf bkg2Pdf("bkg2Pdf","", RooArgList(discrBkg2,xBkg2));
  RooRealVar nBkg2("nBkg2","",10,-100,100);
  nBkg2.removeRange();
  RooExtendPdf bkg2("bkg2","", bkg2Pdf, nBkg2);
  // pdfs
  RooAddPdf pdfCh1("pdfCh1","", RooArgList(sig,bkg1));
  RooAddPdf pdfCh2("pdfCh2","", RooArgList(sig2,bkg2));
  
  RooDataSet *ds2dCh1 = pdfCh1.generate(RooArgSet(xvar,discrvar), 10000);
  RooDataSet *dsdiscrCh1  = new RooDataSet("dsdiscrCh1","", ds2dCh1, discrvar);
  RooDataSet *ds2dCh2 = pdfCh2.generate(RooArgSet(xvar,discrvar), 10000);
  RooDataSet *dsdiscrCh2  = new RooDataSet("dsdiscrCh2","", ds2dCh2, discrvar);

  RooDataSet *dsSig2dCh1 = sig.generate(RooArgSet(xvar,discrvar), 10000);
  RooDataSet *dsSig2dCh2 = sig2.generate(RooArgSet(xvar,discrvar), 10000);
  RooDataSet *dsBkg2dCh1 = bkg1.generate(RooArgSet(xvar,discrvar), 10000);
  RooDataSet *dsBkg2dCh2 = bkg2.generate(RooArgSet(xvar,discrvar), 10000);

  RooDataHist *dhSig2dCh1 = dsSig2dCh1->binnedClone("dhSig2dCh1");
  RooDataHist *dhSig2dCh2 = dsSig2dCh2->binnedClone("dhSig2dCh2");
  RooDataHist *dhBkg2dCh1 = dsBkg2dCh1->binnedClone("dhBkg2dCh1");
  RooDataHist *dhBkg2dCh2 = dsBkg2dCh2->binnedClone("dhBkg2dCh2");

  pdfCh1.fitTo(*ds2dCh1);
  pdfCh2.fitTo(*ds2dCh2);

  TCanvas can;

  RooPlot *xFrCh1 = xvar.frame(Title(xvar.GetName()));
  ds2dCh1->plotOn(xFrCh1);
  sig.plotOn(xFrCh1,LineStyle(kDashed),LineColor(kRed));
  bkg1.plotOn(xFrCh1,LineStyle(kDashed));
  pdfCh1.plotOn(xFrCh1,LineColor(kRed));
  xFrCh1->Draw();
  can.SaveAs(plotdir+"/xvar-ch1.png");
  RooPlot *dFrCh1 = discrvar.frame(Title(discrvar.GetName()));
  ds2dCh1->plotOn(dFrCh1);
  sig.plotOn(dFrCh1,LineStyle(kDashed),LineColor(kRed));
  bkg1.plotOn(dFrCh1,LineStyle(kDashed));
  pdfCh1.plotOn(dFrCh1,LineColor(kRed));
  dFrCh1->Draw();
  can.SaveAs(plotdir+"/discrvar-ch1.png");
  RooPlot *xFrCh2 = xvar.frame(Title(xvar.GetName()));
  ds2dCh2->plotOn(xFrCh2);
  sig2.plotOn(xFrCh2,LineStyle(kDashed),LineColor(kRed));
  bkg2.plotOn(xFrCh2,LineStyle(kDashed));
  pdfCh2.plotOn(xFrCh2,LineColor(kRed));
  xFrCh2->Draw();
  can.SaveAs("~/www/xvar-ch2.png");
  RooPlot *dFrCh2 = discrvar.frame(Title(discrvar.GetName()));
  ds2dCh2->plotOn(dFrCh2);
  sig2.plotOn(dFrCh2,LineStyle(kDashed),LineColor(kRed));
  bkg2.plotOn(dFrCh2,LineStyle(kDashed));
  pdfCh2.plotOn(dFrCh2,LineColor(kRed));
  dFrCh2->Draw();
  can.SaveAs("~/www/discrvar-ch2.png");

  RooCategory cats("cats","");
  cats.defineType("ch1");
  cats.defineType("ch2");
  RooDataSet *dsCombDiscr = new RooDataSet("dsCombDiscr","", discrvar, Index(cats),
					   Import("ch1", *dsdiscrCh1), Import("ch2", *dsdiscrCh2));
  RooDataSet *dsComb2d     = new RooDataSet("dsComb2d","",   RooArgSet(discrvar,xvar), Index(cats),
					    Import("ch1", *ds2dCh1), Import("ch2", *ds2dCh2));

  RooSimultaneous combPdf("combPdf","", cats);
  combPdf.addPdf(pdfCh1, "ch1");
  combPdf.addPdf(pdfCh2, "ch2");

  vector<pair<RooRealVar*,RooDataHist*> > components;
  components.push_back(pair<RooRealVar*,RooDataHist*>(&nSig,dhSig2dCh1));
  components.push_back(pair<RooRealVar*,RooDataHist*>(&nSig2,dhSig2dCh2));
  components.push_back(pair<RooRealVar*,RooDataHist*>(&nBkg1,dhBkg2dCh1));
  components.push_back(pair<RooRealVar*,RooDataHist*>(&nBkg2,dhBkg2dCh2));
  Splotholder sph("fooo","poof", &discrvar, &xvar, xvar.getBins(), xvar.getMin(), xvar.getMax(), dsCombDiscr, &combPdf, dsComb2d, components);


  //
  // note: only plots two components at a time
  //

  double ymax2(0);

  TH1F *hOrigA = sph.hOrigV[0];
  TH1F *hOrigB = sph.hOrigV[2];
  TH1F *hResultA = sph.hResultV[0];
  TH1F *hResultB = sph.hResultV[2];
  
  // initial mc signal distribution
  if(hOrigA->Integral() != 0)
    hOrigA->Scale(1./hOrigA->Integral());
  hOrigA->SetTitle(";ZZ pT;norm.");
  hOrigA->SetLineWidth(4);
  hOrigA->SetFillColor(kOrange+10);
  hOrigA->SetLineColor(kOrange+10);
  hOrigA->SetFillStyle(1);
  hOrigA->SetMarkerSize(0);
  ymax2 = max(ymax2,hOrigA->GetMaximum());

  // mc sig result
  if(hResultA->Integral() != 0)
    hResultA->Scale(1./hResultA->Integral());
  hResultA->SetLineColor(kRed+2);
  hResultA->SetLineWidth(4);
  hResultA->SetMarkerSize(0);
  ymax2 = max(ymax2,hResultA->GetMaximum());

  // initial mc bkg distribution
  if(hOrigB->Integral() != 0)
    hOrigB->Scale(1./hOrigB->Integral());
  hOrigB->SetLineWidth(4);
  hOrigB->SetFillColor(429);
  hOrigB->SetLineColor(429);
  hOrigB->SetFillStyle(1);
  hOrigB->SetMarkerSize(0);
  ymax2 = max(ymax2,hOrigB->GetMaximum());

  // mc bkg result
  if(hResultB->Integral() != 0)
    hResultB->Scale(1./hResultB->Integral());
  hResultB->SetLineColor(kBlue);
  hResultB->SetLineWidth(4);
  hResultB->SetMarkerSize(0);
  ymax2 = max(ymax2,hResultB->GetMaximum());

  TCanvas cvn2;
  cvn2.Divide(1,2);
  cvn2.cd(1);
  gPad->SetPad(0,.2,1,1);

  TH1F hframe2(*hOrigA);
  hframe2.Reset();
  hframe2.SetMaximum(1.2*ymax2);
  hframe2.Draw();
  hOrigA->Draw("e3same");
  hResultA->Draw("esame");
  hOrigB->Draw("e3same");
  hResultB->Draw("esame");

  TLegend leg2(.6,.6,.85,.85);
  leg2.SetFillStyle(0);
  leg2.AddEntry(hOrigA, hOrigA->GetName(), "l");
  leg2.AddEntry(hOrigB, hOrigB->GetName(), "l");
  leg2.AddEntry(hResultA, hResultA->GetName(), "l");
  leg2.AddEntry(hResultB, hResultB->GetName(), "l");
  leg2.Draw();

  cvn2.cd(2);
  gPad->SetPad(0,0,1,.2);
  cvn2.SaveAs(plotdir+"/result-data-"+name+".png");

}
