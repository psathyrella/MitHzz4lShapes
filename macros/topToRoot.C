#include <cassert>
#include "string.h"
#include "TSystem.h"
#include "TFile.h"
#include <sstream>
#include "TROOT.h"
#include <fstream>
#include "TLegend.h"
#include <iostream>
#include "TCanvas.h"
#include "TChain.h"
#include "TH1F.h"
#include "TString.h"
#include <vector>
#include "TRegexp.h"

using namespace std;

void topToRoot(TString topfile)
{
  TCanvas c1;
  vector<TH1F*> hists;
  
  ifstream ifs(topfile);
  assert(ifs.is_open());

  TString outdir(topfile);
  outdir = outdir(0,outdir.Last('/'));
  cout << "using outdir: " << outdir << endl;
  TFile outfile(outdir+"/hres-hists.root","recreate");
  
  string line;
  double xmin,xmax;
  vector<double> xvals,yvals;
  TString title,xlabel,ylabel;
  TString dum; // dummy var for stringstream to poo into
  while(getline(ifs,line)) {
    TString tl(line);
    if(tl.Contains(TRegexp("[ ][ ]*("))) continue;
    
    if(tl.Contains("NEW PLOT")) {
      // cout << "xmin: " << xmin << " xmax: " << xmax << " sizes: " << xvals.size() << ", " << yvals.size() << " title: " << title << " xlabel: " << xlabel << " ylabel: " << ylabel << endl;
      assert(xvals.size()==yvals.size());
      assert(xvals.size() > 3);
      double dx1 = xvals[1] - xvals[0];
      double dx2 = xvals[2] - xvals[1];
      double dx3 = xvals[3] - xvals[2];
      double dx;
      if(fabs(dx1-dx2) < 0.00000001 && fabs(dx2-dx3) < 0.000000001)
	dx = dx1;
      else {
	cout << "nonequal bins!" << endl;
	cout << dx1 << " " << dx2 << " " << dx3 << endl;
	assert(0);
      }
      cout << "nbins: " << xvals.size() << " min: " <<  xvals[0] - 0.5*dx << " max: " << xvals[xvals.size()-1] + 0.5*dx << endl;
      TH1F *hist = new TH1F(title, title, xvals.size(), xvals[0] - 0.5*dx, xvals[xvals.size()-1] + 0.5*dx);
      for(unsigned ibin=0; ibin<xvals.size(); ibin++) {
	hist->SetBinContent(ibin+1, yvals[ibin]);
      }
      hist->Draw();
      c1.SaveAs("~/www/"+title+".png");
      hist->Write();

      xmin = xmax = 0;
      xvals.clear();
      yvals.clear();
      title = xlabel = ylabel = "";
    } else if(tl.Contains("SET WINDOW X")) {
      stringstream ss(line);
      ss >> dum >> dum >> dum >> xmin >> dum >> xmax;
    } else if(tl.Contains("TITLE TOP")) {
      stringstream ss(line);
      ss >> dum >> dum >> title;
      title.ReplaceAll("\"","");
    } else if(tl.Contains("TITLE BOTTOM")) {
      stringstream ss(line);
      ss >> dum >> dum >> xlabel;
      xlabel.ReplaceAll("\"","");
    } else if(tl.Contains("TITLE LEFT")) {
      stringstream ss(line);
      ss >> dum >> dum >> ylabel;
      ylabel.ReplaceAll("\"","");
    } else if(tl.Contains("SET ORDER X Y DY")) {
      getline(ifs,line);
      tl = line;
      while(! tl.Contains("PLOT")) {
	stringstream ss(line);
	double x,y,dy;
	ss >> x >> y >> dy;
	xvals.push_back(x);
	yvals.push_back(y);
	getline(ifs,line);
	tl = line;
      }
    } else {
      continue;
    }
  }

  outfile.ls();
  outfile.Close();
}
