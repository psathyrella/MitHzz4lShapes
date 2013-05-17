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
#include <vector>
#include "Shapes/interface/ShapeHeaders.h"
using namespace std;

#define VARLIST_GENINFO "weight/F:genQid/I:pid_1/I:pid_2/I:x_1/F:x_2/F:id_1/I:id_2/I:vmass_a/F:vpt_a/F:veta_a/F:vphi_a/F:vmass_b/F:vpt_b/F:veta_b/F:vphi_b/F:id_1_a/I:id_2_a/I:id_1_b/I:id_2_b/I:pt_1_a/F:eta_1_a/F:phi_1_a/F:pt_2_a/F:eta_2_a/F:phi_2_a/F:pt_1_b/F:eta_1_b/F:phi_1_b/F:pt_2_b/F:eta_2_b/F:phi_2_b/F:pt_zz/F:y_zz/F:phi_ZZ/F:m_zz/F"

typedef struct { 
  float weight;				// event weight
  int   genQid;                         // largest abs(id) of quark in the hard interaction
  int   pid_1, pid_2;			// parton ID
  float x_1, x_2;			// parton momentum fraction
  int   id_a, id_b;	    		// boson IDs
  float vmass_a, vpt_a, veta_a, vphi_a;	// boson A info
  float vmass_b, vpt_b, veta_b, vphi_b;	// boson B info
  int   id_1_a, id_2_a;			// lepton/quark IDs
  int   id_1_b, id_2_b;			// lepton/quark IDs
  float pt_1_a, eta_1_a, phi_1_a;       // lepton info
  float pt_2_a, eta_2_a, phi_2_a;  
  float pt_1_b, eta_1_b, phi_1_b;
  float pt_2_b, eta_2_b, phi_2_b;  

  float pt_zz, y_zz, phi_zz, m_zz;

} GenInfoStruct;
//----------------------------------------------------------------------------------------
void powheg_to_jhu()
{
  // make histogram to reweight jhu's pT to powheg's
  TString label("morio");
  TString path("/afs/cern.ch/work/d/dkralph/MitHzz4l/Selection/root/"+label);
  TFile f1(path+"/s12-h126zz4l-gf-v7a/merged.root"); assert(f1.IsOpen());
  TFile f2(path+"/s12-x126zz4l-0m-v7c/merged.root"); assert(f1.IsOpen());

  TTree *t1 = (TTree*) f1.Get("zznt"); assert(t1);
  TTree *t2 = (TTree*) f2.Get("zznt"); assert(t2);

  TH1F h1("h1","h1",100,0,130);
  TH1F h2("h2","h2",100,0,130);

  t1->Draw("pt_zz>>h1");
  t2->Draw("pt_zz>>h2");

  scaleHist(&h1);
  scaleHist(&h2);
  TH1F *hratio = makeRatio(&h1, &h2);
  hratio->SetNameTitle("hratio","hratio");
  TFile outfile("data/pt4l/powheg_to_jhu.root","recreate");
  hratio->Write();
  outfile.Close();

  // GenInfoStruct gen;
  // t2->SetBranchAddress("geninfo", &gen);
  // for(unsigned ientry=0; ientry<t2->GetEntries(); ientry++) {
  //   t2->GetEntry(ientry);
  //   cout << gen.pt_zz << endl;
  //   break;
  // }
  
  TCanvas can;
  h1.Draw();
  h2.SetLineColor(kRed);
  h2.Draw("same");
  TLegend leg(.7,.7,.9,.9);
  leg.AddEntry(&h1, "powheg");
  leg.AddEntry(&h2, "jhu");
  leg.Draw();
  leg.SetFillColor(0);
  // hratio->Draw();
  can.SaveAs("~/www/foo.png");

}
