#include "TH1F.h"
#include "Math/QuantFuncMathCore.h"

#include "CPlot.h"
#include "MitStyleRemix.h"
#include "PlotHeaders.h"
#include "HistHeaders.h"
#include "varkeepter.h"

using namespace std;
float wL,wH;

map<TString,map<TString,TH1D*>* > init_hists(FOFlags &ctrl, TString str="");
//----------------------------------------------------------------------------------------
bool isSignal(TString proc)
{
  if(proc=="higgs")      return true;
  else if(proc=="zz")    return false;
  else if(proc=="fakes") return false;
  else assert(0);
}
//----------------------------------------------------------------------------------------
int main(int argc, char** argv) 
{
  SetStyle();
  wL = 115;
  wH = 132;
  bool dummyStr(true);

  TCanvas cvn("can","can",700,500);
  FOFlags ctrl;
  parse_foargs( argc, argv, ctrl );
  ctrl.outdir = "Shapes/root";
  ctrl.dump();
  vector<CSample*> samplev;
  TString ntupledir(""),label(""),plotLabel(""),jsonFile("");
  int puTarget;
  readConfigFile(ctrl.config, ntupledir, label, plotLabel, jsonFile, puTarget, samplev, &ctrl, init_hists);

  //
  // loop over to get sig/bkg ratio in each mass bin
  //
  TH1F hsig("hsig","",50,106,141);
  TH1F hbkg("hbkg","",50,106,141);
  for(unsigned ics=0; ics<samplev.size(); ics++) {
    CSample *cs = samplev[ics];
    if(cs->name == "data") continue;
    cout << cs->name << endl;
    for(unsigned ifs=0; ifs<(cs->fsv).size(); ifs++) {
      filestuff *fs = (cs->fsv)[ifs];
      cout << "\t" << fs->fname_ << " " << fs->getentries("zznt"); cout << flush;
      for(unsigned ientry=0; ientry<fs->getentries("zznt"); ientry++) {
	fs->getentry(ientry,"","zznt");

	double flatWgt(1);

	if(fs->vk->getval("m4l") < wL || fs->vk->getval("m4l") > wH) continue;
	if(fs->dataset_.Contains("fakes")) {
	  flatWgt *= fs->vk->getval("w");
	} else if(!fs->isdata_) {
	  flatWgt *= fs->lumi_ * fs->vk->getval("w")*fs->vk->getval("won")*fs->vk->getval("woff")*fs->vk->getval("npuw");
	}
	  
	if(isSignal(cs->name)) {
	  hsig.Fill(fs->vk->getval("m4l"), flatWgt);
	} else {
	  hbkg.Fill(fs->vk->getval("m4l"), flatWgt);
	}
      }
      cout << endl;
    }
  }

  //
  // fill real hists
  //
  for(unsigned ics=0; ics<samplev.size(); ics++) {
    CSample *cs = samplev[ics];
    cout << cs->name << endl;
    for(unsigned ifs=0; ifs<(cs->fsv).size(); ifs++) {
      filestuff *fs = (cs->fsv)[ifs];
      cout << "\t" << fs->fname_ << " " << fs->getentries("zznt"); cout << flush;
      for(unsigned ientry=0; ientry<fs->getentries("zznt"); ientry++) {
	fs->getentry(ientry,"","zznt");

	double flatWgt(1);

	if(fs->vk->getval("m4l") < wL || fs->vk->getval("m4l") > wH) continue;
	if(fs->dataset_.Contains("fakes")) {
	  flatWgt *= fs->vk->getval("w");
	} else if(!fs->isdata_) {
	  flatWgt *= fs->lumi_ * fs->vk->getval("w")*fs->vk->getval("won")*fs->vk->getval("woff")*fs->vk->getval("npuw");
	}

	fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "m4l"         , fs->vk->getval("m4l")       , flatWgt, 0, 0);
	fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pt4l"	    , fs->vk->getval("ZZpt")      , flatWgt, 0, 0);
	fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "y4l"	    , fs->vk->getval("ZZy")       , flatWgt, 0, 0);

	int m4lBin(hsig.FindBin(fs->vk->getval("m4l")));
	assert(m4lBin = hbkg.FindBin(fs->vk->getval("m4l")));
	double pSig = hsig.GetBinContent(m4lBin) / (hsig.GetBinContent(m4lBin) + hbkg.GetBinContent(m4lBin));

	fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pSig"     	    , pSig                        , flatWgt*pSig, 0, 0);
	fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pt4lSigWgt"	    , fs->vk->getval("ZZpt")      , flatWgt*pSig, 0, 0);
	fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "y4lSigWgt"	    , fs->vk->getval("ZZy")       , flatWgt*pSig, 0, 0);
	fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pt4lBkgWgt"	    , fs->vk->getval("ZZpt")      , flatWgt*(1-pSig), 0, 0);
	fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "y4lBkgWgt"	    , fs->vk->getval("ZZy")       , flatWgt*(1-pSig), 0, 0);


	if(!cs->isdata && !cs->name.Contains("fakes")) {
	  double pieceWgt(fs->lumi_*fs->vk->getval("w"));
	  fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pt4l_w"    , fs->vk->getval("ZZpt")      , pieceWgt, 0, 0);
	  fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pt4l_won"  , fs->vk->getval("ZZpt")      , pieceWgt*fs->vk->getval("won"),  0, 0);
	  fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pt4l_woff" , fs->vk->getval("ZZpt")      , pieceWgt*fs->vk->getval("woff"), 0, 0);
	  fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pt4l_wnpuw", fs->vk->getval("ZZpt")      , pieceWgt*fs->vk->getval("npuw"), 0, 0);
	  fillHist( cs, getChannel((int)fs->vk->getval("channel")), "", "pt4l_wall" , fs->vk->getval("ZZpt")      , pieceWgt*fs->vk->getval("won")*fs->vk->getval("woff")*fs->vk->getval("npuw"), 0, 0);
	}
      }
      cout << endl;
    }
  }

  vector<TString> typev;
  typev.push_back("all");

  if(ctrl.plotAllCats) {
    typev.push_back("4e");
    typev.push_back("4m");
    typev.push_back("2e2m");
  }
  for(unsigned itype=0; itype<typev.size(); itype++) {
    TString type(typev[itype]);
    TString fullOutDir(ctrl.outdir+"/"+plotLabel+"/"+type);
    gSystem->mkdir(fullOutDir,true);
    assert(samplev.size() > 0);
    bool plotShapesOnly=ctrl.plotWholeSample;
    
    CPlot cplot_wgttest("wgttest","","ZZ pT","events",TString(fullOutDir+"/plots"));

    map<TString,TH1D*>::iterator it_v;
    for(it_v=(*(samplev[0]->hists)[type]).begin(); it_v!=(*(samplev[0]->hists)[type]).end(); it_v++) {
      TString var((*it_v).first);
      CPlot cplot(var,"",(*(samplev[0]->hists)[type])[var]->GetXaxis()->GetTitle(),"events",TString(fullOutDir+"/plots"));
      double ymax(0),stackMax(0);
      for(unsigned isam=0; isam<samplev.size(); isam++) {
	CSample *cs = samplev[isam];
	TH1D *hist = (*(cs->hists)[type])[var];

	if(!cs->isdata && var.Contains("_w")) {
	  int linesty(1);
	  if(var.Contains("_won"))        linesty = 2;
	  else if(var.Contains("_woff"))  linesty = 3;
	  else if(var.Contains("_wnpuw")) linesty = 4;
	  else if(var.Contains("_wall"))  linesty = 5;
	  if(!cs->name.Contains("fakes")) {
	    cplot_wgttest.AddHist1D(hist,cs->legend + " " + var + (dummyStr ? "" : ": ")+integral_str(hist,4,dummyStr), TString("hist"), cs->color, linesty);
	  }
	  continue;
	}

	// if(var=="pt4lSigWgt") {
	//   cout << cs->name << endl;
	//   for(unsigned ibin=1; ibin<hist->GetXaxis()->GetNbins()+1; ibin++) {
	//     double alpha=(1-0.6827);
	//     double N(hist->GetBinContent(ibin));
	//     double L =  (N==0) ? 0  : (ROOT::Math::gamma_quantile(alpha/2,N,1.));
	//     double U =  (N==0) ?  ( ROOT::Math::gamma_quantile_c(alpha,N+1,1.) ) :
	//       ( ROOT::Math::gamma_quantile_c(alpha/2,N+1,1.) );
	//     cout
	//        << setw(12) << ibin
	//        << setw(12) << hist->GetBinContent(ibin)
	//        << setw(12) << hist->GetBinError(ibin)
	//        << setw(12) << L
	//        << setw(12) << U
	//        << endl;
	//     // hist->SetBinError(ibin, U);
	//   }
	// }	
	if(plotShapesOnly)     hist->Scale(1./hist->Integral());
	shiftOverflows(hist);
	ymax = max(ymax,hist->GetMaximum());
	if(plotShapesOnly || (cs->isdata && !(cs->name.Contains("fakes")))) {
	  cplot.AddHist1D(hist,cs->legend+(dummyStr ? "" : ": ")+integral_str(hist,4,dummyStr), cs->isdata ? TString("E") : TString("hist"),cs->color);
	} else {
	  cplot.AddToStack(hist,cs->legend+(dummyStr ? "" : ": ")+integral_str(hist,4,dummyStr),cs->color);
	  stackMax += hist->GetMaximum();
	}
      }
      cplot.SetLogy(false);
      double factor(1.4);
      if(var.Contains("y4l")) {
	factor = 1.7;
	cplot.TransLegend(0.14,0);
      }
      cplot.SetYRange(0,factor*max(ymax,stackMax));
      TString txt("#bf{L = 5 ( + 20 ) fb^{-1} at #sqrt{s} = 7 ( + 8 ) TeV}"); // #sqrt{s} = 7 TeV, L = 5 fb^{-1}; #sqrt{s} = 8 TeV, L = 12 fb^{-1}");
      cplot.AddTextBox(txt,0.16,0.95,0.85,0.99,0);
      if(ymax==0) continue; // writing canvases to disk is expensive -- don't write them unless we actually filled them
      cplot.Draw(&cvn,true,"png");
    }
  cplot_wgttest.Draw(&cvn,true,"png");
  }

  assert(string("ROOT SUCKS") == string("FALSE"));
}
//----------------------------------------------------------------------------------------
map<TString,map<TString,TH1D*>* > init_hists(FOFlags &ctrl, TString str)
{
  map<TString,map<TString,TH1D*>* > hists;
  map<TString,TH1D*> *h_4e   = new map<TString,TH1D*>; hists["4e"]   = h_4e;
  map<TString,TH1D*> *h_4m   = new map<TString,TH1D*>; hists["4m"]   = h_4m;
  map<TString,TH1D*> *h_2e2m = new map<TString,TH1D*>; hists["2e2m"] = h_2e2m;
  map<TString,TH1D*> *h_all  = new map<TString,TH1D*>; hists["all"]  = h_all;

  map<TString,map<TString,TH1D*>* >::iterator it_h;
  for(it_h=hists.begin(); it_h!=hists.end(); it_h++) {
    (*((*it_h).second))["m4l"]		= new TH1D(TString("m4l") +"_"+(*it_h).first+str,";#bf{m4l [GeV]};",	   15,wL,wH);         (*((*it_h).second))["m4l"]->Sumw2();
    (*((*it_h).second))["pt4l"]	        = new TH1D(TString("pt4l") +"_"+(*it_h).first+str,";#bf{ZZ pT};",          15,0,85);          (*((*it_h).second))["pt4l"]->Sumw2();
    (*((*it_h).second))["y4l"]		= new TH1D(TString("y4l") +"_"+(*it_h).first+str,";#bf{ZZ y};",            15,-2.4,2.4);      (*((*it_h).second))["y4l"]->Sumw2();
    (*((*it_h).second))["pt4lSigWgt"]	= new TH1D(TString("pt4lSigWgt") +"_"+(*it_h).first+str,";#bf{ZZ pT};",    15,0,85);          (*((*it_h).second))["pt4lSigWgt"]->Sumw2();
    (*((*it_h).second))["y4lSigWgt"]	= new TH1D(TString("y4lSigWgt") +"_"+(*it_h).first+str,";#bf{ZZ y};",      15,-2.4,2.4);      (*((*it_h).second))["y4lSigWgt"]->Sumw2();
    (*((*it_h).second))["pt4lBkgWgt"]	= new TH1D(TString("pt4lBkgWgt") +"_"+(*it_h).first+str,";#bf{ZZ pT};",    15,0,85);          (*((*it_h).second))["pt4lBkgWgt"]->Sumw2();
    (*((*it_h).second))["y4lBkgWgt"]	= new TH1D(TString("y4lBkgWgt") +"_"+(*it_h).first+str,";#bf{ZZ y};",      15,-2.4,2.4);      (*((*it_h).second))["y4lBkgWgt"]->Sumw2();
														   
    (*((*it_h).second))["pSig"]		= new TH1D(TString("pSig") +"_"+(*it_h).first+str,";#bf{P_{sig}};",        15,0,1);           (*((*it_h).second))["pSig"]->Sumw2();
    // (*((*it_h).second))["m4lSig"]	= new TH1D(TString("m4lSig") +"_"+(*it_h).first+str,";#bf{m4l sig};",      50,wL,wH);         (*((*it_h).second))["m4lSig"]->Sumw2();
    // (*((*it_h).second))["m4lBkg"]	= new TH1D(TString("m4lBkg") +"_"+(*it_h).first+str,";#bf{m4l bkg};",      50,wL,wH);         (*((*it_h).second))["m4lBkg"]->Sumw2();
    (*((*it_h).second))["pt4l_w"]	        = new TH1D(TString("pt4l_w") +"_"+(*it_h).first+str,";#bf{ZZ pT};",          15,0,85);          (*((*it_h).second))["pt4l_w"]->Sumw2();
    (*((*it_h).second))["pt4l_won"]	        = new TH1D(TString("pt4l_won") +"_"+(*it_h).first+str,";#bf{ZZ pT};",        15,0,85);          (*((*it_h).second))["pt4l_won"]->Sumw2();
    (*((*it_h).second))["pt4l_woff"]	        = new TH1D(TString("pt4l_woff") +"_"+(*it_h).first+str,";#bf{ZZ pT};",       15,0,85);          (*((*it_h).second))["pt4l_woff"]->Sumw2();
    (*((*it_h).second))["pt4l_wnpuw"]	        = new TH1D(TString("pt4l_wnpuw") +"_"+(*it_h).first+str,";#bf{ZZ pT};",      15,0,85);          (*((*it_h).second))["pt4l_wnpuw"]->Sumw2();
    (*((*it_h).second))["pt4l_wall"]	        = new TH1D(TString("pt4l_wall") +"_"+(*it_h).first+str,";#bf{ZZ pT};",       15,0,85);          (*((*it_h).second))["pt4l_wall"]->Sumw2();
  }
  return hists;
}

// void guillelmo()
// {
//   bool plotCorrectErrorBars = true;
//   if(plotCorrectErrorBars == true) {
//     TGraphAsymmErrors * g = new TGraphAsymmErrors(_data);
//     for (int i = 0; i < g->GetN(); ++i) {
//       double N = g->GetY()[i];
//       double alpha=(1-0.6827);
//       double L =  (N==0) ? 0  : (ROOT::Math::gamma_quantile(alpha/2,N,1.));
//       double U =  (N==0) ?  ( ROOT::Math::gamma_quantile_c(alpha,N+1,1.) ) :
// 	( ROOT::Math::gamma_quantile_c(alpha/2,N+1,1.) );
//       double Lchk =  (N==0) ? 0  : (ROOT::MathMore::gamma_quantile(alpha/2,N,1.));
//       double Uchk =  (N==0) ?  ( ROOT::MathMore::gamma_quantile_c(alpha,N+1,1.) ) :
//       g->SetPointEYlow(i,double(N)-L);
//       if(N > 0)
// 	g->SetPointEYhigh(i, U-double(N));
//       else
// 	g->SetPointEYhigh(i, 0.0);
//     }
//     g->Draw("P");
//   }
//   else {
//     _data->Draw("ep,same");
//   }
// }
