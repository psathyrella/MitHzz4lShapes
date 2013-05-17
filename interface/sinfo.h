#ifndef SINFO
#define SINFO

#include <sstream>

#include "TString.h"

using namespace std;

// lightweight sample information
class sinfo
{
public:
  sinfo();
  sinfo(TString name, TString fname, int era, TString lumi);
  TString name,fname,lumiStr;
  int lumi,era;
  float nExp; // expected events
};

#endif
