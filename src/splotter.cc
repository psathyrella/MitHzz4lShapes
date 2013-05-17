#include "splotter.h"

//----------------------------------------------------------------------------------------
splotter::splotter(TString name_, TString config):
  noData(false),
  absVal(false),
  unfold(false),
  massMin(0),
  massMax(0),
  xMin(0),
  xMinPlot(0),
  xMax(0),
  m4lBinWidth(0), // want this *narrow* since it determines the accuracy of splot's dojiggery
  xBinWidth(0),  // width of this depends on the statistics
  cutVarMin(0),
  cutVarMax(0),
  nBinsM4l(0),
  nBinsX(0),
  nDataMax(-1),
  name(name_),
  plotdir("."),
  xVarName(""),
  xVarTitle(""),
  cutVarName(""),
  cutVarMinStr(""),
  cutVarMaxStr(""),
  unfMethod("svd"),
  tmpFileName("/tmp/dkralph/tmpfile-"+name+"-"+to_string(getpid())+".root"),
  mass(0),
  xVar(0),
  N_S_Tot(0),
  totWgt(0),
  nSigExp_4e(0),
  nSigExp_4m(0),
  nSigExp_2e2m(0)
{
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);
  TH1::SetDefaultSumw2();
  readConfig(config);

  // roofit infrastructure
  mass = new RooRealVar("m4l","m_{4l}",massMin,massMax);
  mass->setBins(nBinsM4l);
  xVar = new RooRealVar(xVarName,xVarTitle,xMinPlot,xMax);
  xVar->setBins(nBinsX);
  N_S_Tot = new RooRealVar("N_S_Tot","total sig",-100,100);
  N_S_Tot->removeRange();
  totWgt = new RooRealVar("totWgt","",-100,100);
  totWgt->removeRange();

  vector<TString> subChans,cutStrs;
  subChans.push_back("4e");    cutStrs.push_back("0");
  subChans.push_back("4m");    cutStrs.push_back("1");
  subChans.push_back("2e2m");  cutStrs.push_back("2");

  map<TString,RooRealVar*> nSigExpV;
  for(unsigned ich=0; ich<subChans.size(); ich++) {
    TString channel(subChans[ich]);
    nSigExpV[channel] = new RooRealVar("nSigExp_"+channel,"",   -100,100);
    nSigExpV[channel]->removeRange();
  }
  
  map<TString,RooFormulaVar*> N_Sv;
  N_Sv["2e2m"] = new RooFormulaVar("N_S_2e2m","", "N_S_Tot / (1 + nSigExp_4e/nSigExp_2e2m + nSigExp_4m/nSigExp_2e2m)", RooArgList(*N_S_Tot, *nSigExpV["4e"], *nSigExpV["4m"], *nSigExpV["2e2m"]));
  N_Sv["4e"]   = new RooFormulaVar("N_S_4e","", "(nSigExp_4e/nSigExp_2e2m)*N_S_2e2m",                                  RooArgList(*nSigExpV["4e"], *nSigExpV["2e2m"], *N_Sv["2e2m"]));
  N_Sv["4m"]   = new RooFormulaVar("N_S_4m","", "(nSigExp_4m/nSigExp_2e2m)*N_S_2e2m",                                  RooArgList(*nSigExpV["4m"], *nSigExpV["2e2m"], *N_Sv["2e2m"]));

  map<TString,Model*> models;
  double nExpTot(0);
  RooCategory categs("categs","");
  TH1D hEvtsPerBin(name+"-hEvtsPerBin", "", nBinsX, xMin, xMax);
  for(unsigned ich=0; ich<subChans.size(); ich++) {
    TString channel(subChans[ich]);
    categs.defineType(channel);
    models[channel] = new Model(channel, cutStrs[ich], config, mass, xVar, N_Sv[channel], totWgt);
    // assert(0);
    nSigExpV[channel]->setVal(models[channel]->nSigExp);
    nSigExpV[channel]->setConstant();
    nExpTot += models[channel]->nSigExp;
    hEvtsPerBin.Add(models[channel]->hEvtsPerBin);
  }
  N_S_Tot->setVal(nExpTot);
  // assert(0);

  models["4e"]->printEvtsPerBin(&hEvtsPerBin);
  
  cout << "\nCombining channels ------------------->" << endl;
  //----------------------------------------------------------------------------------------
  // Do a simultaneous fit to the three channels in order to get a constant mean for the signal model,
  // and to get good initial values for the signal and background yields
  //
  TFile *tmpFile = TFile::Open(tmpFileName,"recreate"); // keep all the temporary trees out of memory
  tmpFile->cd(); // use it to keep -Wall from complaining
  RooArgSet *Nuisances = new RooArgSet("Nuisances_"+name);
  RooSimultaneous simPdf("simPdf","", categs);
  vector<tuple<TString,TString,TString,TString,TH1D*,TH1D*> > mcComponents,dataComponents; // name of associated yield var, reco hist, gen hist, and response matrix fro this channel
  map<TString,RooUnfoldResponse*> unfResps;
  RooArgList yieldVars;
  yieldVars.add(*N_S_Tot);
  TH1D *hNull(0); // type safe null pointer for passing to tuple constructor
  for(unsigned ich=0; ich<subChans.size(); ich++) {
    TString chan(subChans[ich]);
    Model *mod(models[chan]);

    if(unfold) {
      unfResps["sig_"+chan] = mod->sigUnfolder->unfResp;
      unfResps["bkg_"+chan] = mod->bkgUnfolder->unfResp;
    }

    simPdf.addPdf(*mod->totPdfM4l, chan);

    // mod->pdfSigM4l->vars["dm4l"]->setConstant(false); // refloat them prior to fitting for common nuisances on the mean and width
    // mod->pdfSigM4l->vars["dsigma"]->setConstant(false);
    mod->pdfSigM4l->fixNuisances(false);

    // if(nDataMax<0) {
      Nuisances->add(*mod->N_BGNuis->pdf);
      Nuisances->add(*mod->pdfSigM4l->nuisancePdfs);
    // }

    yieldVars.add(*mod->N_BG);
    mcComponents.push_back(  tuple<TString,TString,TString,TString,TH1D*,TH1D*>(N_S_Tot->GetName(),   chan, "sig", chan,  mod->hSigX, mod->hSigGenX));
    dataComponents.push_back(tuple<TString,TString,TString,TString,TH1D*,TH1D*>(N_S_Tot->GetName(),   chan, "sig", chan,  hNull,      hNull));
    mcComponents.push_back(  tuple<TString,TString,TString,TString,TH1D*,TH1D*>(mod->N_BG->GetName(), chan, "bkg", "all", mod->hBkgX, mod->hBkgGenX));
    dataComponents.push_back(tuple<TString,TString,TString,TString,TH1D*,TH1D*>(mod->N_BG->GetName(), chan, "bkg", "all", hNull,      hNull));
  }

  RooDataSet *dsCombDataM4l(0);
  RooDataSet *dsCombMcM4l(0),*dsCombMc2d(0),*dsCombData2d(0);
  RooArgSet allVars("allVars");
  allVars.add(*totWgt);
  allVars.add(*mass);
  allVars.add(*xVar);
  
  dsCombMcM4l = new RooDataSet("dsCombMcM4l","", RooArgSet(*mass,totWgt), Index(categs),
			       Import("4e",   *models["4e"]->dsMcM4l),
			       Import("4m",   *models["4m"]->dsMcM4l),
			       Import("2e2m", *models["2e2m"]->dsMcM4l),
			       WeightVar("totWgt")
			       );
  
  dsCombMc2d = new RooDataSet("dsCombMc2d","", allVars, Index(categs),
			      Import("4e",   *models["4e"]->dsMc2d),
			      Import("4m",   *models["4m"]->dsMc2d),
			      Import("2e2m", *models["2e2m"]->dsMc2d),
			      WeightVar("totWgt")
			      );

  RooFitResult *fitres(0);
  if(!noData) {
    dsCombDataM4l = new RooDataSet("dsCombDataM4l","", *mass, Index(categs),
				   Import("4e",   *models["4e"]->dsDataM4l),
				   Import("4m",   *models["4m"]->dsDataM4l),
				   Import("2e2m", *models["2e2m"]->dsDataM4l)
				   );
    
    
    dsCombData2d = new RooDataSet("dsCombData2d","", allVars, Index(categs),
				  Import("4e",   *models["4e"]->dsData2d),
				  Import("4m",   *models["4m"]->dsData2d),
				  Import("2e2m", *models["2e2m"]->dsData2d)
				  );
    
    // simultaneous fit to data
    cout << "||-------------------> simultaneous fit to data:";
    fitres = simPdf.fitTo(*dsCombDataM4l,Save(true),SumW2Error(kTRUE),ExternalConstraints(*Nuisances),PrintLevel(-1),Verbose(false));
    printRooFitResult(fitres);
    cout << " <------------------- done ||" << endl;

    TCanvas cvn;
    for(unsigned ich=0; ich<subChans.size(); ich++) {
      TString channel(subChans[ich]);
      RooPlot *m4lframe = mass->frame(Title(name));
      dsCombDataM4l->plotOn(m4lframe,Cut(TString("categs==categs::")+channel));
      simPdf.plotOn(m4lframe,Slice(categs,channel),ProjWData(categs,*dsCombDataM4l));
      m4lframe->Draw();
      cvn.SaveAs(plotdir+"/plots/combined-data-fit-"+channel+".png");
      delete m4lframe;
    }	  
  } else {
    cout << "||-------------------> simultaneous fit to mc:";
    fitres = simPdf.fitTo(*dsCombMcM4l,Save(true),SumW2Error(kTRUE),ExternalConstraints(*Nuisances),PrintLevel(-1),Verbose(false));
    printRooFitResult(fitres);
    cout << " <------------------- done ||" << endl;
  }

  // get the nuisances on the mean and width from the simultaneous fit
  float dm4lVal = ((RooRealVar*)fitres->floatParsFinal().find("dm4l"))->getVal();
  for(unsigned ich=0; ich<subChans.size(); ich++) {
    TString channel(subChans[ich]);
    Model *mod(models[channel]);
    mod->pdfSigM4l->setVal("dm4l",dm4lVal);  // set them all to the common value
    RooRealVar *dsigmaVar = (RooRealVar*)fitres->floatParsFinal().find("dsigma_"+channel); // individual width for each channel
    assert(dsigmaVar);
    float dsigmaVal = dsigmaVar->getVal();
    mod->pdfSigM4l->setVal("dsigma",dsigmaVal);
    mod->pdfSigM4l->fixNuisances();
  }

  vector<UpDownHists*> errors; // don't need to remake these -- the uncerts get picked up from the component Splotholder histograms
  if(!unfold) assert(unfResps.size() == 0);
  cout << "making mc splot for " << name << endl;
  Splotholder *sMc = new Splotholder(    "sMc_"+name, "", plotdir, mass, xVar, nBinsX, xMinPlot, xMax, dsCombMcM4l,   &simPdf, dsCombMc2d,   yieldVars,   mcComponents, errors);
  sMc->fillResultHists(  subChans, unfResps, unfMethod);
  sMc->combineChannels();
  Splotholder *sData(0);
  if(!noData) {
    cout << "making data splot for " << name << endl;
    sData = new Splotholder("sData_"+name, "", plotdir, mass, xVar, nBinsX, xMinPlot, xMax, dsCombDataM4l, &simPdf, dsCombData2d, yieldVars, dataComponents, errors);
    sData->fillResultHists(subChans, unfResps, unfMethod);
    sData->combineChannels();
  }

  sMc->plot(noData ? 0 : sData, unfold);

  // cout << "check values are still shifted: " << endl;
  // for(unsigned ich=0; ich<subChans.size(); ich++) {
  //   TString channel(subChans[ich]);
  //   Model *mod(models[channel]);
  //   mod->pdfSigM4l->mean->Print();
  //   mod->pdfSigM4l->sigma->Print();
  // }

  cout
    << setw(15) << "4e: "
    << setw(15) << N_Sv["4e"]->getVal()
    << setw(15) << nSigExpV["4e"]->getVal()
    << endl
    << setw(15) << "4m: "
    << setw(15) << N_Sv["4m"]->getVal()
    << setw(15) << nSigExpV["4m"]->getVal()
    << endl
    << setw(15) << "2e2m: "
    << setw(15) << N_Sv["2e2m"]->getVal()
    << setw(15) << nSigExpV["2e2m"]->getVal()
    << endl;

  delete models["4e"];
  delete models["4m"];
  delete models["2e2m"];
  assert(TString("ROOFITSUCKS")==TString("FALSE"));
}
//----------------------------------------------------------------------------------------
void splotter::readConfig(TString config)
{
  cout << "-------------------> splotter: " << name << endl;
  ifstream ifs(config);
  assert(ifs.is_open());
  string line;
  while(getline(ifs,line)) {
    if(line[0] == '#') continue;
    if(line[0] == '^') {
      stringstream ss1(line),ss2(line),ss3(line);
      TString dummy,tmpStr;
      double tmpVal;
      int tmpInt;
      ss1 >> dummy >> tmpStr;
      ss2 >> dummy >> tmpVal;
      ss3 >> dummy >> tmpInt;
      if     (dummy=="^label")       { }
      else if(dummy=="^plotdir")     { plotdir     = tmpStr; cout << "plotdir    : " << plotdir     << endl; }
      else if(dummy=="^noData")      { noData      = true;   cout << "noData     : " << noData      << endl; }
      else if(dummy=="^absVal")      { absVal      = true;   cout << "absVal     : " << absVal      << endl; }
      else if(dummy=="^unfold")      { unfold      = true;   cout << "unfold     : " << unfold      << endl; }
      else if(dummy=="^massMin")     { massMin     = tmpVal; cout << "massMin    : " <<  massMin    << endl; }
      else if(dummy=="^massMax")     { massMax     = tmpVal; cout << "massMax    : " <<  massMax    << endl; }
      else if(dummy=="^xVarName")    { xVarName    = tmpStr; cout << "xVarName   : " <<  xVarName   << endl; }
      else if(dummy=="^xVarTitle")   { xVarTitle   = tmpStr; cout << "xVarTitle  : " <<  xVarTitle  << endl; }
      else if(dummy=="^xMin")        { xMin        = tmpVal; cout << "xMin       : " <<  xMin       << endl; }
      else if(dummy=="^xMax")        { xMax        = tmpVal; cout << "xMax       : " <<  xMax       << endl; }
      else if(dummy=="^xBinWidth")   { xBinWidth   = tmpVal; cout << "xBinWidth  : " <<  xBinWidth  << endl; }
      else if(dummy=="^m4lBinWidth") { m4lBinWidth = tmpVal; cout << "m4lBinWidth: " << m4lBinWidth << endl; }
      else if(dummy=="^cutVarName")  { cutVarName  = tmpStr; cout << "cutVarName : " << cutVarName  << endl; }
      else if(dummy=="^cutVarMin")   { cutVarMin   = tmpVal; cout << "cutVarMin  : " << cutVarMin   << endl; }
      else if(dummy=="^cutVarMax")   { cutVarMax   = tmpVal; cout << "cutVarMax  : " << cutVarMax   << endl; }
      else if(dummy=="^unfMethod")   { unfMethod   = tmpStr; cout << "unfMethod  : " << unfMethod   << endl; }
      else if(dummy=="^nDataMax")    { nDataMax    = tmpInt; cout << "nDataMax   : " << nDataMax    << endl; }
      // else { cout << "BAD param: " << dummy << endl; assert(0); }
      continue;
    }
  }
  
  assert(unfMethod=="bayes" || unfMethod=="svd");
  xVarTitle.ReplaceAll("~"," ");

  if(absVal) xMinPlot = 0;    // if doing absolute value of variable, cuts value min can be negative, but plotting starts at zero
  else       xMinPlot = xMin;

  plotdir += "/"+xVarName;
  gSystem->mkdir(plotdir+"/plots",true);
  assert(m4lBinWidth!=0 && xBinWidth!=0);
  nBinsM4l = (int)((massMax-massMin)/m4lBinWidth);
  nBinsX  = (int)((xMax-xMinPlot)/xBinWidth);
  assert(massMin!=massMax);
  assert(xMinPlot!=xMax);
}
//----------------------------------------------------------------------------------------
splotter::~splotter()
{
  ifstream ifs(tmpFileName);
  if(ifs.is_open()) remove(tmpFileName);
  else cout << "weird: " << tmpFileName << " DNE" << endl;
}
//----------------------------------------------------------------------------------------
void splotter::printRooFitResult(RooFitResult *fitres)
{

  map<TString, pair<float,float> > vals,errors;
  
  RooArgList initpars(fitres->floatParsInit()),finalPars(fitres->floatParsFinal());
  RooRealVar *tmpVar(0);

  // set initial values
  TIterator *iter(initpars.createIterator());
  while((tmpVar = (RooRealVar*)iter->Next())) {
    assert(tmpVar);
    vals[tmpVar->GetName()].first = tmpVar->getVal();
  }

  // set final values
  iter = finalPars.createIterator();
  while((tmpVar = (RooRealVar*)iter->Next())) {
    assert(tmpVar);
    vals[tmpVar->GetName()].second  = tmpVar->getVal();
    // errors[tmpVar->GetName()].first  = tmpVar->getErrorLo();
    // errors[tmpVar->GetName()].second = tmpVar->getErrorHi();
    errors[tmpVar->GetName()].first  = tmpVar->getError();
  }

  cout << fitres->GetName() << ":" << endl;
  for(map<TString, pair<float,float> >::iterator it=vals.begin(); it!=vals.end(); it++) {
    TString name((*it).first);
    cout
      << setw(20) << name
      << setw(12) << vals[name].first
      << "  ----> "
      << setw(12) << vals[name].second
      << "  +/- "
      << setw(12) << errors[name].first
      << endl;
  }
}
