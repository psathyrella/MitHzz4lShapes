#include "splotter.h"
#include <iostream>
#include <cassert>
#include "TRandom.h"
#include "TH1D.h"

#include "RooUnfold/src/RooUnfoldResponse.h"
#include "RooUnfold/src/RooUnfoldBayes.h"
#include "RooUnfold/src/RooUnfoldSvd.h"

#include "MitStyleRemix.h"
//#include "RooUnfoldTUnfold.h"

using namespace std;

int main(int argc, char ** argv)  
{
  SetStyle();
  assert(argc>1);
  TString config(argv[1]);
  splotter spx("foo",config);
}
