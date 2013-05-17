#include "sinfo.h"

//----------------------------------------------------------------------------------------
sinfo::sinfo()
{
}
//----------------------------------------------------------------------------------------
sinfo::sinfo(TString name_, TString fname_, int era_, TString lumiStr_):
  name(name_),
  fname(fname_),
  lumiStr(lumiStr_),
  era(era_),
  nExp(0)
{
  stringstream ss;
  ss << lumiStr;
  ss >> lumi;
}
