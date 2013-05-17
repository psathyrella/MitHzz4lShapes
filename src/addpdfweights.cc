#include "addpdfweights.h"
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
  TChain * chain = new TChain("angletuple");
  ifstream f(ctrl.inputfile.c_str());
  string fname;
  while (f >> fname) { 
    if( !(strncmp( fname.c_str(), "#", 1 ) ) ) continue; // skip commented lines
    cout << "adding inputfile : " << fname.c_str() << endl;
    assert(chain->AddFile(fname.c_str()));
  }

  LHAPDF::setVerbosity(LHAPDF::SILENT);
  LHAPDF::initPDFSet(ctrl.pdfset.Data());
  LHAPDF::getDescription();
  LHAPDF::usePDFMember(ctrl.imem);

  //
  // Setup
  //--------------------------------------------------------------------------------------------------------------
  const char * ofname;
  if( strcmp( ctrl.outputfile.c_str(), "") ) ofname = ctrl.outputfile.c_str();
  else ofname = "tmp.root";

  TFile *file = new TFile(ofname, "RECREATE");
  TTree *angletuple = chain->CloneTree();
  int pid_1,pid_2;
  float x_1,x_2,Q,w_pdf,w_pdf_lo,w_pdf_hi;
  angletuple->SetBranchAddress("x_1",		&x_1);
  angletuple->SetBranchAddress("x_2",		&x_2);
  TString qstr(ctrl.shapes); // string that tells us which combination of variables to use for Q in this sample, eg "m4l"
  if(qstr=="91p188") Q = 91.188;
  else angletuple->SetBranchAddress(qstr,	&Q);
  angletuple->SetBranchAddress("pid_1",		&pid_1);
  angletuple->SetBranchAddress("pid_2",		&pid_2);
  TBranch *wpdfBr =  angletuple->Branch("w_pdf",	&w_pdf);

  for(unsigned ientry=0; ientry<angletuple->GetEntries(); ientry++) {
    angletuple->GetEntry(ientry);

    Double_t xf1 = LHAPDF::xfx(x_1,Q,pid_1);
    Double_t xf2 = LHAPDF::xfx(x_2,Q,pid_2);
    w_pdf = (xf1/x_1)*(xf2/x_2);

    wpdfBr->Fill();
  }

  file->Write();
  file->Close();
} 



