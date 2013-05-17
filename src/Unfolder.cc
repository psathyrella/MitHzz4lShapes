#include "Unfolder.h"

//----------------------------------------------------------------------------------------
// constructor for training the unfolding
Unfolder::Unfolder(TString name_, TString title_, TString plotdir_, TString xVarName_, TString xVarTitle_, int nBinsX, float xMin, float xMax):
  name(name_),
  title(title_),
  plotdir(plotdir_),
  xVarName(xVarName_),
  xVarTitle(xVarTitle_),
  hGenX(0),
  hRespX(0),
  hUnfX(0),
  unfResp(0),
  bayes(0),
  svd(0)
{
  unfResp = new RooUnfoldResponse(nBinsX, xMin, xMax);
  hGenX   = new TH1D("hGenX_"+name,"",  nBinsX, xMin, xMax);
  hGenX->Sumw2();
  hRespX  = new TH1D("hRespX_"+name,"", nBinsX, xMin, xMax);
  hRespX->Sumw2();
}
//----------------------------------------------------------------------------------------
// constructor if you've trained elsewhere and are just reconstructing the gen distribution
Unfolder::Unfolder(TString name_, TString title_, TString plotdir_, TString xVarName_, TString xVarTitle_,
		   RooUnfoldResponse *unfResp_, TH1D *hRespX_, TH1D *hGenX_, TString method):
  name(name_),
  title(title_),
  plotdir(plotdir_),
  xVarName(xVarName_),
  xVarTitle(xVarTitle_),
  hGenX(hGenX_),
  hRespX(hRespX_),
  hUnfX(0),
  unfResp(unfResp_),
  bayes(0),
  svd(0)
{
  unfold(method);
  // print();
  draw();
}
//----------------------------------------------------------------------------------------
Unfolder::~Unfolder()
{
  // delete unfResp; // don't delete these -- we may not own it
  // delete hGenX;
  // hGenX = 0;
  // delete hRespX;
  // hRespX = 0;
  if(bayes) {
    delete bayes;
    bayes = 0;
  }
  if(svd)   {
    delete svd;
    svd = 0;
  }
  delete hUnfX;
  hUnfX = 0;
}
//----------------------------------------------------------------------------------------
void Unfolder::fill(int ievt, double xVar, double genXVar, double wgt)
{
  if((ievt % 2)==0) { // train on even events
    unfResp->Fill(xVar, genXVar, wgt);
  } else {
    hRespX->Fill(xVar, wgt);
    hGenX->Fill(genXVar, wgt);
  }
  // test on a smeared breit-wigner
  // double xt(gRandom->BreitWigner(42, 17));
  // double x(smear(xt));
  // unfResp->Fill(x, xt, wgt);
  // hRespX->Fill(x, wgt);
  // hGenX->Fill(xt, wgt);
}
//----------------------------------------------------------------------------------------
RooUnfold *Unfolder::getUnf(TString method)
{
  if(method=="bayes")    return bayes;
  else if(method=="svd") return svd;
  else assert(0);
}
//----------------------------------------------------------------------------------------
void Unfolder::unfold(TString method)
{
  assert(unfResp && hRespX);
  if(method=="bayes")     bayes = new RooUnfoldBayes(unfResp, hRespX, 4);
  else if(method=="svd")  svd   = new   RooUnfoldSvd(unfResp, hRespX, 10);
  else assert(0);
  hUnfX = (TH1D*)getUnf(method)->Hreco(RooUnfold::kCovToy);
  hUnfX->Sumw2();
}
//----------------------------------------------------------------------------------------
void Unfolder::print()
{
  if(hGenX) {
    if(bayes) bayes->PrintTable(cout,hGenX);
    if(svd)   svd->PrintTable(cout,hGenX);
  } else {
    cout << "Unfolder::print --> no gen histogram" << endl;
  }
}
//----------------------------------------------------------------------------------------
void Unfolder::draw()
{
  if(!hGenX) {
    // cout << "Unfolder:: draw --> no gen histogram, returning" << endl;
    return;
  }
  if(plotdir=="") {
    // cout << "Unfolder:: draw --> no plot dir, returning" << endl;
    return;
  }

  assert(hGenX && hRespX && hUnfX);
  TCanvas cvn;
  hGenX->SetTitle(";"+xVarTitle+";norm.");
  hGenX->SetMaximum(1.3*max(hGenX->GetMaximum(), hUnfX->GetMaximum()));

  hGenX->Scale(1./hGenX->Integral());
  hRespX->Scale(1./hRespX->Integral());
  hUnfX->Scale(1./hUnfX->Integral());
  double ymax = max(hGenX->GetMaximum(), hRespX->GetMaximum());
  ymax = max(ymax, hUnfX->GetMaximum());

  hGenX->SetLineColor(823);
  hRespX->SetLineColor(kRed);
  hGenX->SetLineWidth(4);
  hRespX->SetLineWidth(4);
  hRespX->SetLineStyle(kDashed);

  hGenX->SetTitle(title);
  hGenX->SetMaximum(1.2*ymax);
  hGenX->Draw("hist");
  hRespX->Draw("histsame");
  hUnfX->Draw("esame");

  TLegend legish(.7,.7,.9,.9);
  legish.SetFillColor(0);
  legish.SetBorderSize(0);
  legish.AddEntry(hGenX, "gen truth", "l");
  legish.AddEntry(hRespX,"reco", "l");
  legish.AddEntry(hUnfX,"unfolded", "p");
  legish.Draw();
  gSystem->mkdir(plotdir+"/plots",kTRUE);
  cvn.SaveAs(plotdir+"/plots/"+name+"-"+xVarName+".png");
}
