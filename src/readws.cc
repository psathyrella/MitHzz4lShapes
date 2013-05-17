// #ifndef __CINT__
// #include "RooGlobalFunc.h"
// #endif
// #include "RooRealVar.h"
// #include "RooGaussian.h"
// #include "RooConstVar.h"
// #include "RooProdPdf.h"
// #include "RooAbsReal.h"
// #include "RooPlot.h"
// #include "TCanvas.h"
// #include "TAxis.h"
// #include "TH1.h"
// #include "TStyle.h"
// #include "TFile.h"
// #include "RooWorkspace.h"
// #include "RooDataSet.h"
// #include "RooDataHist.h"
// #include "RooHistPdf.h"
// #include "RooRealVar.h"
// #include "RooArgSet.h"

// // #include "HZZ2L2QRooPdfs.h"

using namespace RooFit;
using namespace std;

#include <vector>
#include <iomanip>
#include <map>

//----------------------------------------------------------------------------------------
bool testPtr(void *ptr, RooWorkspace *ws)
{
  if(!ptr) {
    ws->Print();
    return false;
  } else
    return true;
}
//----------------------------------------------------------------------------------------
TString chanStr(int chan, bool shortName=true)
// some genius redefined the order of these before choosing variable names in the workspaces
{
  TString chanStr;
  if(chan==1)      chanStr = "4mu";
  else if(chan==2) chanStr = "4e";
  else if(chan==3) chanStr = "2e2mu";
  else assert(0);

  if(shortName) chanStr.ReplaceAll("mu","m");

  return chanStr;
}
//----------------------------------------------------------------------------------------
void readOneWs(int chan, TString tev, int jettag)
{
  TString fname("Shapes/data/chmartin/126/hzz4l_"+chanStr(chan,false)+"S_"+tev+"TeV_"+(Long_t)jettag+".input.root");

  assert(fopen(fname.Data(),"r"));

  TFile wsf(fname);
  RooWorkspace *ws = (RooWorkspace*)(wsf.Get("w"));
  if(!ws) { wsf.ls(); assert(0); }
  // ws->allVars().Print();
  ws->Print();
  return;

  vector<TString> procs,wsNames;
  vector<vector<TString> > parNames,wsParNames;
  vector<vector<float> > parVals;

  ifstream ifs("Shapes/data/offic-ws.conf");
  assert(ifs.is_open());
  string line;
  while(getline(ifs,line)) {
    if(line[0]=='#') continue;
    stringstream ss(line);
    if(line[0] == '$') {
      TString dummy,proc,wsName;
      ss >> dummy >> proc >> wsName;
      procs.push_back(proc);
      wsNames.push_back(wsName);
      vector<TString> pNtmp,wspNtmp;
      vector<float> pvtmp;
      parNames.push_back(pNtmp);
      wsParNames.push_back(wspNtmp);
      parVals.push_back(pvtmp);
      continue;
    }

    TString parName,wsParName;
    ss >> parName >> wsParName;
    wsParName.ReplaceAll("_chan_",TString("_")+(Long_t)chan+"_");
    wsParName.ReplaceAll("_tev_","_"+tev+"_");
    wsParName.ReplaceAll("_jettag",TString("_")+(Long_t)jettag);
    if(procs.back() != "lljj")
      wsParName = "CMS_"+wsParName;
    parNames.back().push_back(parName);
    wsParNames.back().push_back(wsParName);
    parVals.back().push_back(0);
  }

  vector<RooAbsPdf*> pdfs;
  for(unsigned ip=0; ip<procs.size(); ip++) {
    ofstream ofs("Shapes/data/params/"+procs[ip]+"-"+chanStr(chan,true)+"-"+tev+"tev.conf");
    assert(ofs.is_open());
    // ofs << "$ " << procs[ip] << endl;
    RooAbsPdf *pdf(ws->pdf(wsNames[ip]));
    assert(pdf);
    pdfs.push_back(pdf);
    if(parNames[ip].size() == 0) {
      pdf->printArgs(cout);
      cout << endl << endl;
      continue;
    }
    for(unsigned iv=0; iv<parNames[ip].size(); iv++) {
      RooAbsArg *srv(pdf->findServer(wsParNames[ip][iv]));
      if(!srv) { cout << "ERROR: " << wsParNames[ip][iv] << " not found" << endl; return; }
      RooFormulaVar *rfv = (RooFormulaVar*)srv;
      parVals[ip][iv] = rfv->getVal();
      cout
	<< setw(15) << procs[ip]
	<< setw(15) << wsNames[ip]
	<< setw(15) << parNames[ip][iv]
	<< setw(45) << wsParNames[ip][iv]
	<< setw(15) << parVals[ip][iv]
	<< endl;
      ofs
	<< setw(15) << parNames[ip][iv]
	<< setw(15) << parVals[ip][iv]
	<< endl;
    }
  }

  ofs.close();

  return;
}
//----------------------------------------------------------------------------------------
void readws()
// NOTE: if you compile this it tends not to work because the source for the pdf headers changed at some point
{
  gROOT->ProcessLine("gSystem->AddIncludePath(\"-I$ROOFITSYS/include/\")");
  gROOT->ProcessLine("gSystem->Load(\"libRooFit\")");
  gROOT->ProcessLine("gSystem->Load(\"libHiggsAnalysisCombinedLimit.so\")");

  vector<TString> tevs;
  vector<int> chans,jettags;
  chans.push_back(1);
  chans.push_back(2);
  chans.push_back(3);

  tevs.push_back("7");
  tevs.push_back("8");

  jettags.push_back(0);

  for(unsigned ich=0; ich<chans.size(); ich++) {
    for(unsigned itev=0; itev<tevs.size(); itev++) {
      for(unsigned ijtg=0; ijtg<jettags.size(); ijtg++) {
	readOneWs(chans[ich], tevs[itev], jettags[ijtg]);
      }
    }
  }
}
