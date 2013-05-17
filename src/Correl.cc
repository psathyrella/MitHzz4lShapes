#include "Correl.h"

//----------------------------------------------------------------------------------------
Correl::Correl():
  nEvts(0),
  xSum(0),
  ySum(0)
{
}
// //----------------------------------------------------------------------------------------
// Correl::~Correl()
// {
  
// }
//----------------------------------------------------------------------------------------
void Correl::fill(double x, double y)
{
  nEvts++;
  xv.push_back(x);
  yv.push_back(y);
  xSum += x;
  ySum += y;
}
//----------------------------------------------------------------------------------------
double Correl::calcPearson(bool print)
{
  assert(nEvts>0);
  double xMean = xSum/((double)nEvts);
  double yMean = ySum/((double)nEvts);

  double numer(0),denomX(0),denomY(0);
  for(unsigned iev=0; iev<xv.size(); iev++) {
    double dX(xv[iev] - xMean);
    double dY(yv[iev] - yMean);
    numer  += (dX)*(dY);
    denomX += (dX)*(dX);
    denomY += (dY)*(dY);
  }

  float correlation = numer / sqrt(denomX*denomY);
  if(print) {
    cout
      << "  pearson:"
      << setw(15) << correlation
      << endl;
  }

  return correlation;
}
//----------------------------------------------------------------------------------------
vector<unsigned> Correl::makeRankVec(vector<float> &vec, vector<unsigned> &ranks)
// return vector of ranks of values in vec
{
  bool debug(false);
  
  assert(ranks.size()==0);
  if(debug) cout << "vec: " << endl;
  for(int iev=0; iev<nEvts; iev++) {
    if(debug) cout << setw(15) << vec[iev] << endl;
    ranks.push_back(0);
  }

  // find the smallest x
  double smallest;
  int iSmall(-1);
  for(int iev=0; iev<nEvts; iev++) {
    if(iev==0 || vec[iev]<smallest) {
      smallest = vec[iev];
      iSmall   = iev;
    }
  }
  if(debug) cout << "smallest: " << smallest << " iSmall: " << iSmall << endl;
  assert(iSmall>=0);
  ranks[iSmall] = 1;
  unsigned lastRank(1);

  double lastSmallest(smallest);
  // unsigned iLastSmallest(iSmall);
  for(int iev=1; iev<nEvts; iev++) { // start at one since we already found the smallest one
    // find the next smallest
    double nextSmallest(0); // initial values are kind of nonsense
    unsigned iNextSmallest(0);
    bool firstOne(true); // keep track of whether we've set initial values for nextSmallest and iNextSmallest
    for(int jev=0; jev<nEvts; jev++) { // find the next smallest
      if(ranks[jev]!=0) continue; // already did this one
      if(vec[jev]==lastSmallest) { // found a new one that's got the same value
	// cout << "  equal: " << setw(15) << vec[jev] << setw(15) << jev << endl;
	nextSmallest  = vec[jev];
	iNextSmallest = jev;
	firstOne = false;
	break;
      }
      // if it's greater than the last smallest, and it's the closest so far
      if(firstOne || (vec[jev] > lastSmallest && fabs(vec[jev]-lastSmallest) < fabs(nextSmallest-lastSmallest))) {
	// cout << "  setting: " << setw(15) << vec[jev] << setw(15) << jev << endl;
	nextSmallest  = vec[jev];
	iNextSmallest = jev;
	firstOne = false;
      } else {
	// cout << "  not greater & co: "
	//      << setw(15) << vec[jev]
	//      << setw(15) << lastSmallest
	//      << setw(15) << fabs(vec[jev]-lastSmallest)
	//      << setw(15) << fabs(nextSmallest-lastSmallest)
	//      << endl;
      }
    }
    ranks[iNextSmallest] = ++lastRank;
    lastSmallest  = nextSmallest;
    // iLastSmallest = iNextSmallest;
    if(debug) cout << "next smallest: " << setw(15) << nextSmallest << endl;
  }

  if(debug) cout << "xranks: " << endl;
  for(int iev=0; iev<nEvts; iev++) { // start at one since we already found the smallest one
    if(debug) cout << setw(15) << vec[iev] << setw(15) << ranks[iev] << endl;
    assert(ranks[iev]!=0);
  }

  return ranks;
}
//----------------------------------------------------------------------------------------
double Correl::calcSpearman(bool print)
// see wikipedia entry on Spearman's rank correlation coefficient
{
  makeRankVec(xv,xRanks);
  makeRankVec(yv,yRanks);
  unsigned long sumDiff2(0);
  for(int iev=0; iev<nEvts; iev++) {
    unsigned long diff = (int)xRanks[iev] - (int)yRanks[iev];
    sumDiff2 += diff*diff;
    // cout << setw(15) << xv[iev] << setw(15) << yv[iev] << setw(15) << xRanks[iev] << setw(15) << yRanks[iev] << setw(15) << diff*diff << endl;
  }

  double rho = 1 - ((double)6*(double)sumDiff2) / ((double)nEvts*( nEvts*nEvts-1 ));
  if(print) {
    cout
      << "  spearman:"
      << setw(15) << rho
      << endl;
  }

  return rho;
}
//----------------------------------------------------------------------------------------
void Correl::plot(TString png)
{
  TGraph gr(nEvts);
  for(int iev=0; iev<nEvts; iev++) {
    gr.SetPoint(iev, xv[iev], yv[iev]);
  }

  gr.SetMarkerSize(.5);

  TCanvas cvn;
  gr.Draw("ap");
  cvn.SaveAs((png=="") ? "~/www/foo.png" : png);
}
