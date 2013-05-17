{  
  if(gSystem->Getenv("CMSSW_VERSION")) {
    // TString rfitpath("/afs/cern.ch/cms/sw/$SCRAM_ARCH/lcg/roofit/5.26.00-cms5/include");
    TString rfitpath("/server/02a/cmsprod/cmssoft/slc5_amd64_gcc434/lcg/roofit/5.28.00a-cms3/include/");
    TString path = gSystem->GetIncludePath();
    path += "-I. -I$ROOTSYS/src -I";
    path += rfitpath;
    path += " -I/home/dkralph/cms/cmssw/025/CMSSW_4_4_1/src";
    gSystem->SetIncludePath(path.Data());
    TString str = gSystem->GetMakeSharedLib();
    if (str.Contains("-m32")==0 && str.Contains("-m64")==0) {
      str.ReplaceAll("g++", "g++ -m32");
      gSystem->SetMakeSharedLib(str);
    }      
    
    // gROOT->Macro("$CMSSW_BASE/src/MitAna/macros/setRootEnv.C+");
    gROOT->Macro("$CMSSW_BASE/src/Common/CPlot.cc+");
    gROOT->Macro("$CMSSW_BASE/src/Common/MitStyleRemix.cc+");
    // gSystem->Load("$CMSSW_BASE/lib/slc5_amd64_gcc434/libMitHttNtupleDefs.so");
  }

  // Show which process needs debugging
  // gInterpreter->ProcessLine(".! ps |grep root.exe");
}
