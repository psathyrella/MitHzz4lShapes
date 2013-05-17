#include <cassert>
#include "TFile.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TLegend.h"
#include <iomanip>
#include <iostream>
#include "TCanvas.h"
#include "TChain.h"
#include "TH1F.h"
#include "TString.h"
#include "TStyle.h"
#include <vector>
#include "Shapes/interface/ShapeHeaders.h"
using namespace std;

//----------------------------------------------------------------------------------------

void makeTheHists(TString var, int nBins, float varMin, float varMax, TString type, TString outfilemode)
{
  TString plotLabel(var);
  TString basedir("/afs/cern.ch/work/d/dkralph/powheg/production");
  TCanvas c1;

  bool writefile(true);

  vector<TString> procs,fnames;
  vector<TString> rens,facs,pdfs,resums,labs;
  vector<TString> mHs;
  if(type=="renfac") {
    rens.push_back("1.0");  facs.push_back("1.0");
    rens.push_back("0.5");  facs.push_back("0.5");
    rens.push_back("2.0");  facs.push_back("2.0");

    // ZZ 
    procs.push_back("zz4e");	  fnames.push_back(basedir+"/ZZ/s12-zz4e-renXXX-facXXX-pdfXXX-dkr/merged.root");
    procs.push_back("zz4m");	  fnames.push_back(basedir+"/ZZ/s12-zz4m-renXXX-facXXX-pdfXXX-dkr/merged.root");
    procs.push_back("zz2e2m");	  fnames.push_back(basedir+"/ZZ/s12-zz2e2m-renXXX-facXXX-pdfXXX-dkr/merged.root");
    // gg_H
    mHs.push_back("126");
    for(unsigned imH=0; imH<mHs.size(); imH++) {
      procs.push_back("h"+mHs[imH]+"zz4l");
      fnames.push_back(  basedir+"/gg_H/s12-h"+mHs[imH]+"zz4l-gf-renXXX-facXXX-pdfXXX-dkr/merged.root");
    }

  } else if(type=="pdf") {
    for(long ipdf=10800; ipdf<10853; ipdf++)
      pdfs.push_back(TString("")+ipdf);

    // ZZ
    procs.push_back("zz4e");	  fnames.push_back(basedir+"/ZZ/s12-zz4e-renXXX-facXXX-pdfXXX-dkr/merged.root");
    procs.push_back("zz4m");	  fnames.push_back(basedir+"/ZZ/s12-zz4m-renXXX-facXXX-pdfXXX-dkr/merged.root");
    procs.push_back("zz2e2m");	  fnames.push_back(basedir+"/ZZ/s12-zz2e2m-renXXX-facXXX-pdfXXX-dkr/merged.root");
    // gg_H
    mHs.push_back("126");
    for(unsigned imH=0; imH<mHs.size(); imH++) {
      procs.push_back("h"+mHs[imH]+"zz4l");
      fnames.push_back(  basedir+"/gg_H/s12-h"+mHs[imH]+"zz4l-gf-renXXX-facXXX-pdfXXX-dkr/merged.root");
    }
  } else if(type=="hres") {
    resums.push_back("ce");
    resums.push_back("lo");
    resums.push_back("hi");
    resums.push_back("powheg");

    mHs.push_back("126");
    for(unsigned imH=0; imH<mHs.size(); imH++) {
      procs.push_back("h"+mHs[imH]+"zz4l");
      fnames.push_back(  basedir+"/gg_H/s12-h"+mHs[imH]+"zz4l-gf-resumXXX-hres-v2-dkr/unmerged/hres-hists.root"); // use topToRoot.C to convert from .top files
    }
  } else assert(0);

  vector<TChain> chains;
  vector<TH1F*> hists;
  TH1F *lohist=0,*hihist=0,*cehist=0,*hresHist=0;
  vector<int> cols,styles;
  for(unsigned ist=1; ist<10; ist++) {
    cols.push_back(ist);
    styles.push_back(ist);
  }

  TFile *outfile(0);
  if(writefile) outfile = new TFile(var+"4l_weights.root",outfilemode);

  unsigned istop(0);
  if(type=="pdf")         istop = pdfs.size();
  else if(type=="renfac") istop = rens.size();
  else if(type=="hres")   istop = resums.size();
  for(unsigned iproc=0; iproc<fnames.size(); iproc++) {
    cout << "starting proc: " << procs[iproc] << endl;
    cout << " hists size: " << hists.size() << endl;
    hists.clear();
    cout << " hists size: " << hists.size() << endl;

    for(unsigned iscale=0; iscale<istop; iscale++)  {
      TString ren(""),fac(""),pdf(""),resum("");
      if(type=="renfac") {
	pdf = "10800";
	ren = rens[iscale];
	fac = facs[iscale];
	labs.push_back("ren "+ren+", fac "+fac);
      } else if(type=="pdf") {
	pdf = pdfs[iscale];
	ren = "1.0";
	fac = "1.0";
	labs.push_back("mem: "+pdf);
      } else if(type=="hres") {
	resum = resums[iscale];
	labs.push_back("resum: "+resum);
      }

      TChain chain("Events","Events");

      TString fname(fnames[iproc]);
      fname.ReplaceAll("renXXX","ren"+ren);
      fname.ReplaceAll("facXXX","fac"+fac);
      fname.ReplaceAll("pdfXXX","pdf"+pdf);
      if(resum=="powheg") {
	fname.ReplaceAll("-resumXXX-hres-v2-dkr/unmerged/hres-hists.root","-ren1.0-fac1.0-pdf10800-dkr/merged.root"); // get the central powheg file
      } else {
	fname.ReplaceAll("resumXXX","resum"+resum);
      }
	
      if(!fopen(fname,"r")) {
	cout << "  " << fname << " not found, skipping" << endl;
	continue;
      }
      if(type=="hres") {
	if(resum=="powheg") {
	  chain.AddFile(fname);
	  assert(hists.size()>0); // get the binning from the previous hres hists
	  int nbinsHres = hists.back()->GetNbinsX();
	  double xminHres = hists.back()->GetXaxis()->GetXmin();
	  double xmaxHres = hists.back()->GetXaxis()->GetXmax();
	  hists.push_back(new TH1F(ren+fac+pdf+resum,labs.back()+";ZZ "+var+" [GeV];",nbinsHres,xminHres,xmaxHres));
	  chain.Draw(var+"4l>>"+ren+fac+pdf+resum);
	} else {
	  TFile infile(fname);
	  assert(infile.IsOpen());
	  TH1F *hist = (TH1F*)infile.Get(var+"3456");
	  assert(hist);
	  hist->SetDirectory(0);
	  infile.Close();
	  hists.push_back(new TH1F(*hist));
	  hists.back()->SetNameTitle(ren+fac+pdf+resum,labs.back()+";ZZ "+var+" [GeV];");
	}
	if(resum=="ce")          hresHist = hists.back();
	else if(resum=="lo")     lohist = hists.back();
	else if(resum=="hi")     hihist = hists.back();
	else if(resum=="powheg") cehist = hists.back();
      } else {
	hists.push_back(new TH1F(ren+fac+pdf+resum,labs.back()+";ZZ "+var+" [GeV];",nBins,varMin,varMax));
	if(type=="renfac") {
	  if(ren=="1.0" && fac=="1.0")      cehist = hists.back();
	  else if(ren=="0.5" && fac=="0.5") lohist = hists.back();
	  else if(ren=="2.0" && fac=="2.0") hihist = hists.back();
	}	
	chain.AddFile(fname);
	
	chain.Draw(var+"4l>>"+ren+fac+pdf+resum);
      }
      hists.back()->SetDirectory(0);
      scaleHist(hists.back());
    }

    if(type=="pdf") {
      cehist = makeEnvelopeHist(hists, "central");
      lohist = findStrangestHist(hists);
      hihist = makeOppo(lohist, cehist);
    }

    //
    // Plot all variations
    //
    TLegend legend(0.6,0.6,0.9,0.9);
    legend.SetBorderSize(0);
    legend.SetFillColor(0);
    int ii=0;
    bool first(true);
    for(unsigned iscale=0; iscale<hists.size(); iscale++)  {
      cout << "  " << setw(3) << iscale << " hist entries: " << hists[ii]->GetEntries() << " ( " << hists[ii]->Integral(0,hists[ii]->GetNbinsX()+2) << " ) " << hists[ii]->GetTitle() << endl;
      if(hists[ii]->GetEntries() == 0) { cout << hists[ii]->GetName() << " has no entries" << endl; ii++; continue; }
      if(first) {
	TString tmpTitle(hists[ii]->GetTitle());
	// hists[ii]->GetYaxis()->SetRangeUser(0);
	hists[ii]->Draw("same");
	hists[ii]->SetTitle(tmpTitle);
	first = false;
      } else {
	hists[ii]->SetLineColor(cols[ii%cols.size()]);
	hists[ii]->SetLineWidth(4);
	hists[ii]->SetLineStyle(styles[ii%styles.size()]);
	hists[ii]->Draw("same");
      }
      legend.AddEntry(hists[ii],hists[ii]->GetTitle(),"L");
      ii++;
    }
    legend.Draw();
    TString plotdir("/afs/cern.ch/user/d/dkralph/www/MitHzz4l/Shapes/plots/"+plotLabel);
    gSystem->mkdir(plotdir+"/"+type+"/plots/", kTRUE);
    c1.SaveAs(plotdir+"/"+type+"/plots/"+procs[iproc]+"-all.png");

    //
    // make ratio hists
    //
    assert(cehist && lohist && hihist);
    TH1F *hLoWgts = makeRatio(lohist,cehist);
    hLoWgts->SetNameTitle("LoWgts_"+procs[iproc]+"_"+type, procs[iproc]+";ZZ "+var+" [GeV];");
    TH1F *hHiWgts = makeRatio(hihist,cehist);
    hHiWgts->SetNameTitle("HiWgts_"+procs[iproc]+"_"+type, procs[iproc]+";ZZ "+var+" [GeV];");
    TH1F *hHresWgts=0;
    if(type=="hres") {
      assert(hresHist);
      hHresWgts = makeRatio(hresHist,cehist);
      hHresWgts->SetNameTitle("CeWgts_"+procs[iproc]+"_"+type, procs[iproc]+";ZZ "+var+" [GeV];");
    }
    if(writefile) {
      outfile->cd();
      hLoWgts->Write();
      hHiWgts->Write();
      if(hHresWgts)
	hHresWgts->Write();
    }

    //
    // plot weights
    //
    legend.Clear();
    double ymin(min(hLoWgts->GetMinimum(), hHiWgts->GetMinimum()));
    double ymax(max(hLoWgts->GetMaximum(), hHiWgts->GetMaximum()));
    hLoWgts->GetYaxis()->SetRangeUser(0.8*ymin, 1.2*ymax);
    if(var=="pt")
      hLoWgts->GetXaxis()->SetRangeUser(0,0.5*varMax);
    legend.AddEntry(hLoWgts,"low","L");
    legend.AddEntry(hHiWgts,"high","L");
    hLoWgts->SetLineWidth(4);
    hLoWgts->SetLineColor(kRed);
    hHiWgts->SetLineWidth(4);
    hHiWgts->SetLineColor(kBlue);
    hLoWgts->Draw();
    hHiWgts->Draw("same");
    if(hHresWgts) {
      hHresWgts->SetLineColor(823);
      hHresWgts->Draw("same");
      legend.AddEntry(hHresWgts,"central hres","L");
    }
    legend.Draw();
    c1.SaveAs(plotdir+"/"+type+"/plots/"+procs[iproc]+"-ratios.png");

    //
    // plot central, upper, and lower hists
    //
    legend.Clear();
    cehist->GetXaxis()->SetTitle("ZZ "+var+" [GeV]");
    cehist->SetLineColor(kBlack);
    lohist->SetLineColor(kRed);
    hihist->SetLineColor(kBlue);
    cehist->SetLineStyle(1);
    lohist->SetLineStyle(kDashed);
    hihist->SetLineStyle(kDashed);
    cehist->SetLineWidth(4);
    lohist->SetLineWidth(4);
    hihist->SetLineWidth(4);
    legend.AddEntry(cehist,cehist->GetTitle(),"L");
    legend.AddEntry(lohist,lohist->GetTitle(),"L");
    legend.AddEntry(hihist,hihist->GetTitle(),"L");
    cehist->Draw();
    lohist->Draw("same");
    hihist->Draw("same");
    if(hresHist) {
      hresHist->SetLineColor(823);
      hresHist->SetLineWidth(4);
      hresHist->Draw("same");
      legend.AddEntry(hresHist,hresHist->GetTitle(),"L");
    }
    legend.Draw();
    c1.SetLogy();
    c1.SaveAs(plotdir+"/"+type+"/plots/"+procs[iproc]+"-hilo_log.png");

    c1.SetLogy(false);
    cehist->GetYaxis()->SetRangeUser(0,1.2*cehist->GetMaximum());
    if(var=="pt")
      cehist->GetXaxis()->SetRangeUser(0,0.5*varMax);
    cehist->Draw();
    lohist->Draw("same");
    hihist->Draw("same");
    if(hresHist) {
      hresHist->Draw("same");
    }
    legend.Draw();
    c1.SaveAs(plotdir+"/"+type+"/plots/"+procs[iproc]+"-hilo.png");
  }

  if(writefile)
    outfile->Close();
}
//----------------------------------------------------------------------------------------
void makePt4lHists()
{
  TString var("pt");
  float varMin(0),varMax(100);
  int nBins(100);
  // TString var("y");
  // float varMin(-6),varMax(6);
  // int nBins(10);
  makeTheHists(var, nBins, varMin, varMax, "renfac", "recreate");
  makeTheHists(var, nBins, varMin, varMax, "pdf",    "update");
  makeTheHists(var, nBins, varMin, varMax, "hres",   "update");
}
