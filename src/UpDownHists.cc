#include "UpDownHists.h"

//----------------------------------------------------------------------------------------
UpDownHists::UpDownHists(TString name, TString weight, TString var_, float varLo, float varHi, int nBinsVar):
  var(var_),
  hCe(0),
  hLo(0),
  hHi(0),
  hErrLo(0),
  hErrHi(0)
{
  hCe = new TH1F("hCe_"+name+"_"+var+"_"+weight,"", nBinsVar, varLo, varHi);
  hLo = new TH1F("hLo_"+name+"_"+var+"_"+weight,"", nBinsVar, varLo, varHi);
  hHi = new TH1F("hHi_"+name+"_"+var+"_"+weight,"", nBinsVar, varLo, varHi);

  hCe->Sumw2();
  hLo->Sumw2();
  hHi->Sumw2();

  hCe->SetDirectory(0);
  hLo->SetDirectory(0);
  hHi->SetDirectory(0);
}
//----------------------------------------------------------------------------------------
bool UpDownHists::fill(double val, double wCe, double wLo, double wHi)
{
  if(wCe!=wCe || wLo!=wLo || wHi!=wHi) {
    cout << wCe << " " << wLo << " " << wHi << endl;
    return false;
  }
  hCe->Fill(val, wCe);
  hLo->Fill(val, wLo);
  hHi->Fill(val, wHi);
  return true;
}
//----------------------------------------------------------------------------------------
void UpDownHists::makeErrorHists()
{
  hCe->Scale(1/hCe->Integral());
  hLo->Scale(1/hLo->Integral());
  hHi->Scale(1/hHi->Integral());

  // cout << "making error hists for " << hCe->GetName() << endl;
  TString tmpName(hCe->GetName());
  tmpName.ReplaceAll("hCe","hErrLo");
  hErrLo = new TH1F(tmpName,"", hCe->GetXaxis()->GetNbins(), hCe->GetXaxis()->GetXmin(), hCe->GetXaxis()->GetXmax());
  tmpName.ReplaceAll("hErrLo","hErrHi");
  hErrHi = new TH1F(tmpName,"", hCe->GetXaxis()->GetNbins(), hCe->GetXaxis()->GetXmin(), hCe->GetXaxis()->GetXmax());

  for(int ibin=1; ibin<hCe->GetXaxis()->GetNbins()+1; ibin++) {
    double ce(hCe->GetBinContent(ibin));
    double lo(hLo->GetBinContent(ibin));
    double hi(hHi->GetBinContent(ibin));

    double dLo(lo-ce);
    double dHi(hi-ce);

    hErrLo->SetBinContent(ibin, dLo);
    hErrHi->SetBinContent(ibin, dHi);
    // cout
    //   << setw(15) << ce
    //   << setw(15) << lo
    //   << setw(15) << hi
    //   << setw(15) << dLo
    //   << setw(15) << dHi
    //   << endl;
  }
}
