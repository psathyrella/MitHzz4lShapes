//
// System headers
//
#include <vector>                   // STL vector class
#include <iostream>                 // standard I/O
#include <iomanip>                  // functions to format standard I/O
#include <fstream>                  // functions for file I/O
#include <string>                   // C++ string class
#include <sstream>                  // class for parsing strings
#include <assert.h>
#include <stdlib.h>   
#include <getopt.h>
using namespace std;

//
// ROOT headers
//
#include <TROOT.h>                  // access to gROOT, entry point to ROOT system
#include <TSystem.h>                // interface to OS
#include <TFile.h>                  // file handle class
#include <TNtuple.h> 
#include <TTree.h>                  // class to access ntuples
#include <TChain.h>                 // 
#include <TBranch.h>                // class to access branches in TTree
#include <TClonesArray.h>           // ROOT array class
#include <TCanvas.h>                // class for drawing
#include <TH1F.h>                   // 1D histograms
#include <TBenchmark.h>             // class to track macro running statistics
#include <TLorentzVector.h>         // 4-vector class
#include <TVector3.h>               // 3D vector class

//
// utility headers
//
#include "ParseArgs.h"
#include "LHAPDF/LHAPDF.h"
