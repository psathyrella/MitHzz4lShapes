#include "addpdfupdowns.h"

//=== MAIN =================================================================================================
int main(int argc, char** argv) 
{
  
  //
  // args
  //--------------------------------------------------------------------------------------------------------------
  ControlFlags ctrl;
  parse_args( argc, argv, ctrl );
  ctrl.dump();

  //
  // File I/O
  //--------------------------------------------------------------------------------------------------------------
  vector<string> fnamev;
  vector<TFile*> filev;
  vector<TTree*> treev;

  ifstream f(ctrl.inputfile.c_str());
  string fname;
  while (f >> fname) { 
    if( !(strncmp( fname.c_str(), "#", 1 ) ) ) continue; // skip commented lines
    cout << "adding inputfile : " << fname.c_str() << endl;
    fnamev.push_back(fname.c_str());
    filev.push_back(TFile::Open(fname.c_str()));              assert(filev.back()->IsOpen());
    treev.push_back((TTree*)filev.back()->Get("angletuple"));  assert(treev.back());
  }

  //
  // Setup
  //--------------------------------------------------------------------------------------------------------------
  const char * ofname;
  if( strcmp( ctrl.outputfile.c_str(), "") ) ofname = ctrl.outputfile.c_str();
  else ofname = "tmp.root";

  // copy a file to get a clean ttree
  TString copyname(TString(fnamev[0]).ReplaceAll(".root","-COPY.root"));
  TString com("cp "+fnamev[0]+" "+copyname);
  system(com);
  TFile copyfile(copyname);
  TTree *copytree = (TTree*)copyfile.Get("angletuple");

  TFile *file = new TFile(ofname, "RECREATE");
  TTree *angletuple = copytree->CloneTree();
  float w_pdf_lo,w_pdf_hi;
  TBranch *w_pdf_loBr = angletuple->Branch("w_pdf_lo",	&w_pdf_lo);
  TBranch *w_pdf_hiBr = angletuple->Branch("w_pdf_hi",	&w_pdf_hi);

  vector<TBranch*> wbranchv;
  vector<float> w_pdfv;
  for(int imem=0; imem<fnamev.size(); imem++) w_pdfv.push_back(float(0));
  for(int imem=0; imem<fnamev.size(); imem++) wbranchv.push_back(treev[imem]->GetBranch("w_pdf"));
  for(int imem=0; imem<fnamev.size(); imem++) treev[imem]->SetBranchAddress("w_pdf",&w_pdfv[imem]);

  for(unsigned ientry=0; ientry<angletuple->GetEntries(); ientry++) {
    angletuple->GetEntry(ientry);
    float sum2_lo=0,sum2_hi=0;
    wbranchv[0]->GetEntry(ientry); // get central value
    float central = w_pdfv[0];
    if(ctrl.debug) cout << "central: " << central << endl;
    for(int imem=1; imem<fnamev.size(); imem++) {
      wbranchv[imem]->GetEntry(ientry);
      float delta = w_pdfv[imem] - central;
      if(ctrl.debug) cout << "\t delta: " << delta << endl;
      // add diff for this member pdf to upper or lower sum-of-squares
      if(delta<0) sum2_lo += delta*delta;
      else        sum2_hi += delta*delta;
      if(ctrl.debug) cout << "\tsums: " << sum2_lo << "\t" << sum2_hi << endl;
    }

    w_pdf_lo = (central - sqrt(sum2_lo))/central; w_pdf_loBr->Fill();
    if(ctrl.debug) cout << "\tw_pdf_lo: " << w_pdf_lo << endl;
    w_pdf_hi = (central + sqrt(sum2_hi))/central; w_pdf_hiBr->Fill();
    if(ctrl.debug) cout << "\tw_pdf_hi: " << w_pdf_hi << endl;
  }

  // for(int imem=0; imem<fnamev.size(); imem++) filev[imem]->Close();

  file->Write();
  file->Close();

  copyfile.Close();
  return 0;
} 



