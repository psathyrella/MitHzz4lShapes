// Script to check HCP rates from 110-140
// By: Chris Martin

#include "TCanvas.h"
#include <iostream>

void debugRates()
{
  TCanvas* canv = new TCanvas("canv","canv",1000,600);
  gROOT->ProcessLine("gSystem->AddIncludePath(\"-I$ROOFITSYS/include/\")");
  gROOT->ProcessLine("gSystem->Load(\"libRooFit\")");
  gROOT->ProcessLine("gSystem->Load(\"libHiggsAnalysisCombinedLimit.so\")");

  TString chanStr,chanInt;
  // chanStr = "4e";   chanInt = "0";
  // chanStr = "4mu";   chanInt = "1";
  chanStr = "2e2mu"; chanInt = "2";

  TFile* f_new_untagged = new TFile("Shapes/data/chmartin/126/hzz4l_"+chanStr+"S_8TeV_0.input.root","OPEN");
  RooWorkspace* ws = (RooWorkspace*)f_new_untagged->Get("w");
  RooRealVar* m_new_untagged = (RooRealVar*)ws->var("CMS_zz4l_mass");
  RooPlot* fr = m_new_untagged->frame();

  TFile mefile("Angles/root/training/shapes/gfH/BDT.gfH.126.root");
  TTree *zznt = (TTree*)mefile.Get("zznt");
  TH1D *hme = new TH1D("hme","", 60, 106, 141);
  hme->Sumw2();
  zznt->Draw("m4l>>hme", "w*(m4l>106)*(m4l<141)*19600*won*woff*npuw*(channel=="+chanInt+")*(ZZpt>0)*(ZZpt<80)*(nJets<2)");
  // zznt->Draw("m4l>>hme", "(channel==0)*npuw");

  RooDataHist ds("ds", "", *m_new_untagged, hme);
  ds.plotOn(fr);

  ((RooDoubleCB*)ws->pdf("signalCB_ggH"))->plotOn(fr,	RooFit::LineStyle(1), RooFit::LineColor(1));
  ((RooDoubleCB*)ws->pdf("signalCB_VBF"))->plotOn(fr,	RooFit::LineStyle(2), RooFit::LineColor(1));
  ((RooDoubleCB*)ws->pdf("signalCB_ttH"))->plotOn(fr,	RooFit::LineStyle(3), RooFit::LineColor(1));
  ((RooDoubleCB*)ws->pdf("signalCB_WH"))->plotOn(fr,	RooFit::LineStyle(4), RooFit::LineColor(1));
  ((RooDoubleCB*)ws->pdf("signalCB_ZH"))->plotOn(fr,	RooFit::LineStyle(5), RooFit::LineColor(1));
  ((RooqqZZPdf_v2*)ws->pdf("bkg_qqzzTmp"))->plotOn(fr,	RooFit::LineStyle(1), RooFit::LineColor(4));
  ((RooggZZPdf_v2*)ws->pdf("bkg_ggzzTmp"))->plotOn(fr,	RooFit::LineStyle(1), RooFit::LineColor(6));
  ((RooLandau*)ws->pdf("bkg_zjetsTmp"))->plotOn(fr,	RooFit::LineStyle(2), RooFit::LineColor(6));
  fr->Draw();
  canv->SaveAs("/afs/cern.ch/user/d/dkralph/www/foo.png");

  return;
}
