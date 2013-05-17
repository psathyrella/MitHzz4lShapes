#include "Model.h"

//----------------------------------------------------------------------------------------
double Model::smear(double xt)
// Gaussian smearing, systematic translation, and variable inefficiency
{
  // const double cutdummy(-99999);
  // double xeff= 0.3 + (1.0-0.3)/20*(xt+10.0);  // efficiency
  // double x= gRandom->Rndm();
  // if (x>xeff) return cutdummy;
  double xsmear= gRandom->Gaus(-2.5,0.2);     // bias and smear
  return xt+xsmear;
}
//----------------------------------------------------------------------------------------
Model::Model(TString name_, TString channelStr_, TString config, RooRealVar *mass_, RooRealVar *xVar_, RooFormulaVar *N_S_, RooRealVar *totWgt_):
  noData(false),
  unfold(false),
  doCorrel(false),
  absVal(false),
  scaResErr(false),
  debug(false),
  massMin(50),
  massMax(170),
  xMin(0),
  xMinPlot(0),
  xMax(120),
  m4lBinWidth(1), // want this *narrow* since it determines the accuracy of splot's dojiggery
  xBinWidth(3),  // width of this depends on the statistics
  cutVarMin(0),
  cutVarMax(0),
  nSigExp(0),
  nBkgExp(0),
  nBinsM4l(0),
  nBinsX(0),
  nDataMax(-1),
  channel(99),
  name(name_),
  plotdir("."),
  label(""),
  massMinStr(""),
  massMaxStr(""),
  xMinStr(""),
  xMaxStr(""),
  channelStr(channelStr_),
  xVarName(""), // set it using txt file in readConfig
  xVarTitle(""),
  cutVarName(""),
  cutVarMinStr(""),
  cutVarMaxStr(""),
  sigModType("doubleCb"),
  unfMethod("svd"),
  wsOffic(""),
  tBothMc(0),
  tdata(0),
  tmpFileName("/tmp/dkralph/tmpfile-"+name+"-"+to_string(getpid())+".root"),
  tmpFile(0),
  N_S(N_S_),
  mass(mass_),
  xVar(xVar_),
  N_S_Local(0),
  N_BG(0),
  totWgt(totWgt_),
  N_SNuis(0),
  N_BGNuis(0),
  Nuisances(0),
  dsDataM4l(0),
  dsData2d(0),
  dsMcM4l(0),
  dsMc2d(0),
  toyDataM4l(0),
  toyData2d(0),
  hNull(0),
  hEvtsPerBin(0),
  hSigM4l(0),
  hSigX(0),
  hSigGenX(0),
  hBkgM4l(0),
  hBkgX(0),
  hBkgGenX(0),
  hSigChk(0),
  hBkgChk(0),
  hSig2d(0),
  hBkg2d(0),
  dhSigM4l(0),
  dhSig2d(0),
  dhBkgM4l(0),
  dhBkg2d(0),
  pdfSigM4l(0),
  pdfSig2d(0),
  pdfBkg2d(0),
  pdfBkgM4l(0),
  pdfSigM4l_ext(0),
  pdfSigM4l_ext_Local(0),
  pdfSig2d_ext(0),
  pdfBkgM4l_ext(0),
  pdfBkg2d_ext(0),
  totPdfM4l(0),
  totPdfM4l_Local(0),
  totPDF2d(0),
  fitresData(0),
  sData(0),
  sMc(0),
  wonErr(0),
  woffErr(0),
  renfacErr(0),
  pdfErr(0),
  hresErr(0),
  sigUnfolder(0),
  bkgUnfolder(0),
  allUnfolder(0),
  corr(0)
{
  SetStyle();
  if(channelStr!="") {
    assert(channelStr=="0" || channelStr=="1" || channelStr=="2");
    stringstream ss;
    ss << channelStr;
    ss >> channel;
  }

  //----------------------------------------------------------------------------------------
  // get input and create objects
  //----------------------------------------------------------------------------------------
  samples["sig"]  = new vector<sinfo>;
  samples["bkg"]  = new vector<sinfo>;
  samples["data"] = new vector<sinfo>;
  readConfig(config);
  
  N_S_Local = new RooRealVar("N_S_Local_"+name,name+" sig",-100,100);
  N_S_Local->removeRange();
  N_BG = new RooRealVar("N_BG_"+name,name+" bkg",-100,100);
  N_BG->removeRange();

  tmpFile = TFile::Open(tmpFileName,"recreate"); // keep all the temporary trees out of memory
  assert(tmpFile->IsOpen());
  wonErr    = new UpDownHists(name, "won",    xVarName, xMinPlot, xMax, nBinsX);
  woffErr   = new UpDownHists(name, "woff",   xVarName, xMinPlot, xMax, nBinsX);
  renfacErr = new UpDownHists(name, "renfac", xVarName, xMinPlot, xMax, nBinsX);
  pdfErr    = new UpDownHists(name, "pdf",    xVarName, xMinPlot, xMax, nBinsX);
  hresErr   = new UpDownHists(name, "hres",   xVarName, xMinPlot, xMax, nBinsX);
  vector<UpDownHists*> errors;
  errors.push_back(woffErr);
  errors.push_back(wonErr); // official analysis doesn't apply these, so take them out for the moment
  errors.push_back(renfacErr);
  errors.push_back(pdfErr);
  errors.push_back(hresErr);

  hSigM4l  = new TH1D("hSigM4l_"+name, "",  nBinsM4l, massMin, massMax);
  hBkgM4l  = new TH1D("hBkgM4l_"+name, "",  nBinsM4l, massMin, massMax);
  hSigX    = new TH1D("hSigX_"+name, "",    nBinsX,   xMinPlot,   xMax);
  hBkgX    = new TH1D("hBkgX_"+name, "",    nBinsX,   xMinPlot,   xMax);
  hSigGenX = new TH1D("hSigGenX_"+name, "", nBinsX,   xMinPlot,   xMax);
  hBkgGenX = new TH1D("hBkgGenX_"+name, "", nBinsX,   xMinPlot,   xMax);
  hSig2d   = new TH2D("hSig2d_"+name, "",   nBinsM4l, massMin, massMax, nBinsX, xMinPlot, xMax);
  hBkg2d   = new TH2D("hBkg2d_"+name, "",   nBinsM4l, massMin, massMax, nBinsX, xMinPlot, xMax);
  hSigM4l->Sumw2();
  hBkgM4l->Sumw2();
  hSigX->Sumw2();
  hBkgX->Sumw2();
  hSig2d->Sumw2();
  hBkg2d->Sumw2();

  if(unfold) {
    allUnfolder = new Unfolder("all-"+name, "sig + bkg, "+name, plotdir+"/unfold", xVarName, xVarTitle, nBinsX, xMinPlot, xMax);
    sigUnfolder = new Unfolder("sig-"+name, "sig, "+name,       plotdir+"/unfold", xVarName, xVarTitle, nBinsX, xMinPlot, xMax);
    bkgUnfolder = new Unfolder("bkg-"+name, "bkg, "+name,       plotdir+"/unfold", xVarName, xVarTitle, nBinsX, xMinPlot, xMax);
  }

  //----------------------------------------------------------------------------------------
  // create hists, pdfs, datasets, & co
  //----------------------------------------------------------------------------------------
  readMcFiles();
  if(unfold) unfoldEm();
  if(!noData) readDataFiles();

  //----------------------------------------------------------------------------------------
  // make data splot
  //----------------------------------------------------------------------------------------
  RooArgList yieldVars;
  yieldVars.add(*N_S_Local);
  yieldVars.add(*N_BG);
  vector<tuple<TString,TString,TString,TString,TH1D*,TH1D*> > dataComps;
  map<TString,RooUnfoldResponse*> unfResps;
  if(unfold) {
    unfResps["sig_"] = sigUnfolder->unfResp;
    unfResps["bkg_"] = bkgUnfolder->unfResp;
  }
  vector<TString> subChans;
  subChans.push_back("");
  if(!noData) {
    cout << "  making data splot for " << name << endl;
    dataComps.push_back(tuple<TString,TString,TString,TString,TH1D*,TH1D*>(N_S_Local->GetName(), "", "sig", "", hNull, hNull));
    dataComps.push_back(tuple<TString,TString,TString,TString,TH1D*,TH1D*>(N_BG->GetName(),      "", "bkg", "", hNull, hNull));
    vector<UpDownHists*> dummyErrors;
    sData = new Splotholder("sData_"+name, name, plotdir, mass, xVar, nBinsX, xMinPlot, xMax, dsDataM4l, totPdfM4l_Local, dsData2d, yieldVars, dataComps, dummyErrors);
    sData->fillResultHists(subChans,unfResps,unfMethod);
  }

  //----------------------------------------------------------------------------------------
  // make mc splot(s)
  //----------------------------------------------------------------------------------------
  vector<tuple<TString,TString,TString,TString,TH1D*,TH1D*> > mcComps;
  mcComps.push_back(tuple<TString,TString,TString,TString,TH1D*,TH1D*>(N_S_Local->GetName(), "", "sig", "", hSigX, unfold ? hSigGenX : hNull));
  mcComps.push_back(tuple<TString,TString,TString,TString,TH1D*,TH1D*>(N_BG->GetName(),      "", "bkg", "", hBkgX, unfold ? hBkgGenX : hNull));

  RooDataSet *dsLo(0),*dsHi(0); // make splots with up/down shifted mean and width (NOTE: splot screws around with the RooDataSets, so need to make copies before calling the splot constructor
  if(scaResErr) {
    dsLo = new RooDataSet(*dsMcM4l, dsMcM4l->GetName()+TString("Lo"));
    dsHi = new RooDataSet(*dsMcM4l, dsMcM4l->GetName()+TString("Hi"));
  }

  assert(dsMcM4l->get()->getSize() == 1); // if you forget and run the splot constructor before this, this dataset'll have five branches instead of 1
  cout << "  making mc splot for " << name << endl;
  sMc = new Splotholder("sMc_"+name,   name, plotdir, mass, xVar, nBinsX, xMinPlot, xMax, dsMcM4l, totPdfM4l_Local, dsMc2d, yieldVars, mcComps, errors);
  sMc->fillResultHists(subChans,unfResps,unfMethod);

  if(scaResErr && wsOffic=="") { // note: need to unfold these, as well
    pdfSigM4l->shiftParam("mean",  -1);
    pdfSigM4l->shiftParam("sigma", -1);
    Splotholder sMcLo("sMcLo_"+name, name, plotdir, mass, xVar, nBinsX, xMinPlot, xMax, dsLo, totPdfM4l_Local, dsMc2d, yieldVars, mcComps, errors);
    sMcLo.fillResultHists(subChans,unfResps,unfMethod);
    pdfSigM4l->shiftParam("mean",  +1);
    pdfSigM4l->shiftParam("sigma", +1);
    pdfSigM4l->shiftParam("mean",  +1);
    pdfSigM4l->shiftParam("sigma", +1);
    Splotholder sMcHi("sMcHi_"+name, name, plotdir, mass, xVar, nBinsX, xMinPlot, xMax, dsHi, totPdfM4l_Local, dsMc2d, yieldVars, mcComps, errors);
    sMcHi.fillResultHists(subChans,unfResps,unfMethod);
    pdfSigM4l->shiftParam("mean",  -1); // yeah, I *know* this doesn't quite get me back to where I started. But later on we refit to the three channels simultaneously, anyway.
    pdfSigM4l->shiftParam("sigma", -1);
    addScaleResUncertBand(sMc, &sMcLo, &sMcHi);
  }

  sMc->plot(noData ? 0 : sData, unfold);
}
//----------------------------------------------------------------------------------------
Model::~Model()
{
  delete samples["sig"];
  delete samples["bkg"];
  delete samples["data"];
  delete N_S_Local;
  delete N_BG;
  if(wonErr)    delete wonErr;
  if(woffErr)   delete woffErr;
  if(renfacErr) delete renfacErr;
  if(pdfErr)    delete pdfErr;
  if(hresErr)   delete hresErr;
  // should delete tdata here
  // should delete histograms
  if(unfold) {
    delete allUnfolder;
    allUnfolder = 0;
    delete sigUnfolder;
    sigUnfolder = 0;
    delete bkgUnfolder;
    bkgUnfolder = 0;
  }
  // should delete pdfs & co.
  
  samples["sig"]  = 0;
  samples["bkg"]  = 0;
  samples["data"] = 0;
  N_S_Local	  = 0;
  N_BG		  = 0;
  wonErr	  = 0;
  woffErr	  = 0;
  renfacErr	  = 0;
  pdfErr	  = 0;
  hresErr	  = 0;
  
  ifstream ifs(tmpFileName);
  if(ifs.is_open()) remove(tmpFileName);
  else cout << "weird: " << tmpFileName << " DNE" << endl;
}
//----------------------------------------------------------------------------------------
void Model::readConfig(TString config)
{
  cout << "-------------------> Model: " << name << endl;
  ifstream ifs(config);
  assert(ifs.is_open());
  string line;
  TString type("");
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
      if(     dummy=="^label")       { label       = tmpStr; cout << "label      : " << label       << endl; }
      else if(dummy=="^plotdir")     { plotdir     = tmpStr; cout << "plotdir    : " << plotdir     << endl; }
      else if(dummy=="^noData")      { noData      = true;   cout << "noData     : " << noData      << endl; }
      else if(dummy=="^unfold")      { unfold      = true;   cout << "unfold     : " << unfold      << endl; }
      else if(dummy=="^doCorrel")    { doCorrel    = true;   cout << "doCorrel   : " << doCorrel    << endl; }
      else if(dummy=="^absVal")      { absVal      = true;   cout << "absVal     : " << absVal      << endl; }
      else if(dummy=="^absVal")      { absVal      = true;   cout << "absVal     : " << absVal      << endl; }
      else if(dummy=="^scaResErr")   { scaResErr   = true;   cout << "scaResErr  : " << scaResErr   << endl; }
      else if(dummy=="^debug")       { debug       = true;   cout << "debug      : " << debug       << endl; }
      else if(dummy=="^sigModType")  { sigModType  = tmpStr; cout << "sigModType : " << sigModType  << endl; }
      else if(dummy=="^massMin")     { massMin     = tmpVal; cout << "massMin    : " << massMin     << endl; }
      else if(dummy=="^massMax")     { massMax     = tmpVal; cout << "massMax    : " << massMax     << endl; }
      else if(dummy=="^xVarName")    { xVarName    = tmpStr; cout << "xVarName   : " << xVarName    << endl; }
      else if(dummy=="^xVarTitle")   { xVarTitle   = tmpStr; cout << "xVarTitle  : " << xVarTitle   << endl; }
      else if(dummy=="^xMin")        { xMin        = tmpVal; cout << "xMin       : " << xMin        << endl; }
      else if(dummy=="^xMax")        { xMax        = tmpVal; cout << "xMax       : " << xMax        << endl; }
      else if(dummy=="^xBinWidth")   { xBinWidth   = tmpVal; cout << "xBinWidth  : " << xBinWidth   << endl; }
      else if(dummy=="^m4lBinWidth") { m4lBinWidth = tmpVal; cout << "m4lBinWidth: " << m4lBinWidth << endl; }
      else if(dummy=="^cutVarName")  { cutVarName  = tmpStr; cout << "cutVarName : " << cutVarName  << endl; }
      else if(dummy=="^cutVarMin")   { cutVarMin   = tmpVal; cout << "cutVarMin  : " << cutVarMin   << endl; }
      else if(dummy=="^cutVarMax")   { cutVarMax   = tmpVal; cout << "cutVarMax  : " << cutVarMax   << endl; }
      else if(dummy=="^unfMethod")   { unfMethod   = tmpStr; cout << "unfMethod  : " << unfMethod   << endl; }
      else if(dummy=="^wsOffic")     { wsOffic     = tmpStr; cout << "wsOffic    : " << wsOffic     << endl; }
      else if(dummy=="^nDataMax")    { nDataMax    = tmpInt; cout << "nDataMax   : " << nDataMax    << endl; }
      else { cout << "BAD param: " << dummy << endl; assert(0); }
      continue;
    }

    assert(sigModType=="singleCb" || sigModType=="doubleCb" || sigModType=="template");
    assert(unfMethod=="bayes" || unfMethod=="svd");
    xVarTitle.ReplaceAll("~"," ");

    if(line[0] == '$') {
      if(     line.find("sig")  != string::npos) type = "sig";
      else if(line.find("bkg")  != string::npos) type = "bkg";
      else if(line.find("data") != string::npos) type = "data";
      else { cout << "BAD line: " << line << endl; assert(0); }
      continue;
    }
    if(samples.find(type) == samples.end()) {
      cout << "ERROR: couldn't find " << type << " in samples" << endl;
      assert(0);
    }

    stringstream ss(line);
    TString name,lumiStr;
    int era;
    ss >> name >> era >> lumiStr;
    assert(era==2011 || era==2012);
    TString fname("Angles/root/training/"+label+"/"+name+"/BDT."+name+".126.root");
    if(era==2011) fname.ReplaceAll(label,label+"_2011");
    sinfo si(name,fname,era,lumiStr);
    samples[type]->push_back(si);
  }

  if(absVal) xMinPlot = 0;    // if doing absolute value of variable, cuts value min can be negative, but plotting starts at zero
  else       xMinPlot = xMin;

  plotdir += "/"+xVarName;
  gSystem->mkdir(plotdir+"/plots",true);

  nBinsM4l = (int)((massMax-massMin)/m4lBinWidth);
  nBinsX  = (int)((xMax-xMinPlot)/xBinWidth);
  assert(massMin!=massMax);
  assert(xMinPlot!=xMax);

  stringstream ss;
  ss << massMin << " " << massMax << " " << xMin << " " << xMax;
  ss >> massMinStr >> massMaxStr >> xMinStr >> xMaxStr;
  if(cutVarName!="") {
    stringstream ss2;
    ss2 << cutVarMin << " " << cutVarMax;
    ss2 >> cutVarMinStr >> cutVarMaxStr;
  }

}
//----------------------------------------------------------------------------------------
void Model::readMcFiles()
{
  assert(tmpFile->IsOpen());
  tmpFile->cd();
  // Trees of the mc to pass to the SPlot constructor
  double tmpM4l(0),tmpxVar(0),tmpxVarGen(0),tmpWgt(0);
  tBothMc = new TTree("tBothMc_"+name,"both m4l");
  tBothMc->Branch("m4l",             &tmpM4l);
  tBothMc->Branch(xVarName,          &tmpxVar);
  tBothMc->Branch(totWgt->GetName(), &tmpWgt);
  
  vector<TString> types;
  types.push_back("sig");
  types.push_back("bkg");
  for(unsigned itype=0; itype<types.size(); itype++) {
    TString type(types[itype]);
    TH1D *hM4l(  type=="sig" ? hSigM4l  : hBkgM4l);
    TH1D *hX(    type=="sig" ? hSigX    : hBkgX);
    TH1D *hGenX( type=="sig" ? hSigGenX : hBkgGenX);
    TH2D *h2d(   type=="sig" ? hSig2d   : hBkg2d);
    if(wsOffic!="") {
      vector<pair<float,M4lModel*> > pdfList;
      analyticPdfs[type] = pdfList;
    }
    for(unsigned ifile=0; ifile<samples[type]->size(); ifile++) {
      sinfo *si(&(*samples[type])[ifile]);

      TString cutstr("w*(m4l>"+massMinStr+")*(m4l<"+massMaxStr+")*("+xVarName+">"+xMinStr+")*("+xVarName+"<"+xMaxStr+")*"+si->lumiStr);
      if(!si->name.Contains("lljj")) cutstr += "*won*woff*npuw";
      if(channelStr!="")            cutstr += "*(channel=="+channelStr+")";
      if(cutVarName!="")            cutstr += "*("+cutVarName+">"+cutVarMinStr+")*("+cutVarName+"<"+cutVarMaxStr+")";

      varkeepter vk("Angles/data/zzntvars.txt", "r", si->fname, 0, "zznt");

      corr = new Correl;
      
      TString xDrawStr(xVarName);
      if(absVal) xDrawStr = "TMath::Abs("+xDrawStr+")";
      TString xGenDrawStr(xDrawStr);
      xGenDrawStr.ReplaceAll(xVarName,"gen"+xVarName);

      // 1d hists
      TH1D htmpM4l( "htmpM4l","",  nBinsM4l, massMin, massMax);
      TH1D htmpX(   "htmpX","",    nBinsX,   xMinPlot,    xMax);
      TH1D htmpGenX("htmpGenX","", nBinsX,   xMinPlot,    xMax);
      htmpM4l.Sumw2();
      htmpX.Sumw2();
      htmpGenX.Sumw2();
      vk.gettree()->Draw("m4l>>htmpM4l", cutstr, "goff");
      vk.gettree()->Draw(xDrawStr+">>htmpX", cutstr, "goff");
      if(unfold)
	vk.gettree()->Draw(xGenDrawStr+">>htmpGenX", cutstr, "goff");
      si->nExp = htmpM4l.Integral();
      hM4l->Add(&htmpM4l);
      hX->Add(&htmpX);
      hGenX->Add(&htmpGenX);
      // 2d hist
      TH2D htmp2d("htmp2d","", nBinsM4l, massMin, massMax, nBinsX, xMinPlot, xMax);
      htmp2d.Sumw2();
      vk.gettree()->Draw(xDrawStr+":m4l>>htmp2d", cutstr, "goff");
      h2d->Add(&htmp2d);

      if(wsOffic != "") { // if we're using official parametrizations from txt files, push back a pdf and a yield for each component
	M4lModel *componentPdf = getAnalyticPdf(*si);
	// if(si->name.Contains("gfH") || si->name.Contains("vbfH") || si->name.Contains("vttH")) componentPdf->makeNuisPdfs();
	pair<float,M4lModel*> yieldPdf(si->nExp, componentPdf);
	analyticPdfs[type].push_back(yieldPdf);

	// if(si->name=="lljj") {
	//   TCanvas tmpc;
	//   RooPlot *tmpf = mass->frame();
	//   componentPdf->pdf->plotOn(tmpf);
	//   tmpf->Draw();
	//   tmpc.SaveAs("/afs/cern.ch/user/d/dkralph/www/foo.png");
	//   assert(0);
	// }
      }

      cout << "  " << setw(12) << si->name << setw(15) << si->nExp << "    " << cutstr << endl;

      //
      // fill trees
      //
      for(unsigned ientry=0; ientry<vk.getentries(); ientry++) {
	vk.getentry(ientry);

	if(vk.getval("m4l")	 < massMin)	continue;
	if(vk.getval("m4l")	 > massMax)	continue;
	if(vk.getval(xVarName)	 < xMin)	continue;
	if(vk.getval(xVarName)	 > xMax)	continue;
	if(cutVarName!="") {
	  if(vk.getval(cutVarName) < cutVarMin)	continue;
	  if(vk.getval(cutVarName) > cutVarMax)	continue;
	}
	if(channelStr!="" && vk.getval("channel")!=channel)      continue;
	
	tmpWgt     = vk.getval("w")*si->lumi;
	tmpM4l     = vk.getval("m4l");
	tmpxVar    = vk.getval(xVarName);
	if(unfold) tmpxVarGen = vk.getval("gen"+xVarName);
	if(absVal) {
	  tmpxVar    = fabs(tmpxVar);
	  tmpxVarGen = fabs(tmpxVarGen);
	}
	
	// if(ientry<5) cout << "  weight: " << vk.getval("w") << " " << si->lumi;
	if(!si->name.Contains("lljj")) {
	  tmpWgt *= vk.getval("won")*vk.getval("woff")*vk.getval("npuw");
	  // if(ientry<5) cout << "      and: " << vk.getval("won") << " " << vk.getval("woff") << " " << vk.getval("npuw");
	}
	// if(ientry<5) cout << " ---> " << tmpWgt << endl;

	//
	// fill uncertainty hists
	//
	if(si->name.Contains("lljj")) {
	  // wonErr->fill( vk.getval(xVarName),  1, 1, 1);
	  // woffErr->fill(vk.getval(xVarName),  1, 1, 1);
	} else {
	  bool filled(true);
	  filled &= wonErr->fill(    tmpxVar,  vk.getval("won"), vk.getval("won")  -  vk.getval("werron"), vk.getval("won")  + vk.getval("werron"));
	  filled &= woffErr->fill(   tmpxVar, vk.getval("woff"), vk.getval("woff") - vk.getval("werroff"), vk.getval("woff") + vk.getval("werroff"));
	  filled &= renfacErr->fill( tmpxVar,                 1,                vk.getval("pt4lRenfacLo"),                vk.getval("pt4lRenfacHi"));
	  filled &= pdfErr->fill(    tmpxVar,                 1,                   vk.getval("pt4lPdfLo"),                   vk.getval("pt4lPdfHi"));
	  filled &= hresErr->fill(   tmpxVar,                 1,                  vk.getval("pt4lHresLo"),                  vk.getval("pt4lHresHi"));
	  if(!filled)
	    cout
	      << "  failed filling errors: "
	      << setw(12) << vk.getval("m4l")
	      << setw(12) << vk.getval("ZZpt")
	      << setw(12) << vk.getval("ZZy")
	      << endl;
	}

	tBothMc->Fill();
	if(doCorrel) corr->fill(tmpxVar, vk.getval("m4l"));

	if(unfold && !si->name.Contains("lljj")) { // fill a signle response matrix with all sig and bkg components
	  allUnfolder->fill(    (int)(vk.getval("evt")), tmpxVar, tmpxVarGen, tmpWgt);
	  if(type=="sig")
	    sigUnfolder->fill(  (int)(vk.getval("evt")), tmpxVar, tmpxVarGen, tmpWgt);
	  else if(type=="bkg")
	    bkgUnfolder->fill(  (int)(vk.getval("evt")), tmpxVar, tmpxVarGen, tmpWgt);
	  else assert(0);
	}
      }
      if(doCorrel) {
	cout << "calculating corr: " << endl;
	corr->calcSpearman();
	corr->calcPearson();
	cout << "DONE" << endl;
	corr->plot(TString(getenv("HOME"))+"/www/tmp/plots/"+si->name+"-"+name+".png");
	delete corr;
	corr = 0;
      }
    }
  }

  // if(wsOffic!="") {
  //   for(map<TString,vector<pair<float,M4lModel*> > >::iterator it=analyticPdfs.begin(); it!=analyticPdfs.end(); it++) {
  //     TString type((*it).first);
  //     cout << type << endl;
  //     vector<pair<float,M4lModel*> > pdfs((*it).second);
  //     for(unsigned ip=0; ip<pdfs.size(); ip++) {
  // 	cout << setw(15) << pdfs[ip].first << "    ";
  // 	pdfs[ip].second->pdf->Print();
  //     }
  //   }
  // }
  // assert(0);

  // check the yields in the tree are the same as in the histograms
  TH1D hChk("hChk","",100,0,1000);
  tBothMc->Draw("m4l>>hChk",totWgt->GetName());
  assert(fabs(hChk.Integral() - hSigM4l->Integral() - hBkgM4l->Integral()) < 0.005*hChk.Integral());

  wonErr->makeErrorHists();
  woffErr->makeErrorHists();
  renfacErr->makeErrorHists();
  pdfErr->makeErrorHists();
  hresErr->makeErrorHists();

  // cout << "ENTRIES: " << hSigX->GetEntries() << " " << hBkgX->GetEntries() << " " << tBothMc->GetEntries() << endl;
  makeMcShapes();

  if(unfold) {
    TCanvas cvn;
    TH1D hBoth(*hSigX);
    TH1D hSig(*hSigGenX); // NOTE: these are only filled if you're unfolding
    TH1D hBkg(*hBkgGenX);
    hBoth.SetTitle(name+";"+xVarTitle+";"+"norm.");
    hBoth.Add(hBkgX);
    hBoth.Scale(1./hBoth.Integral());
    hSig.SetLineColor(kRed);
    hBkg.SetLineColor(kBlue);
    hSig.Scale(0.5/hSig.Integral());
    hBkg.Scale(0.5/hBkg.Integral());
    hBoth.SetMaximum(1.2*max(hSig.GetMaximum(),hBkg.GetMaximum()));
    hBoth.Draw("e");
    hSig.Draw("histsame");
    hBkg.Draw("histsame");
    TLegend leg(.6,.7,.9,.9);
    leg.SetBorderSize(0);
    leg.SetFillStyle(0);
    leg.AddEntry(&hSig, "gen sig", "l");
    leg.AddEntry(&hBkg, "gen bkg", "l");
    leg.AddEntry(&hBoth, "reco sum", "l");
    leg.Draw();
    cvn.SaveAs(plotdir+"/plots/mc-initial-"+name+".png");
  }
}
//----------------------------------------------------------------------------------------
void Model::makeMcShapes()
{
  assert(mass && xVar && totWgt && N_S && N_BG);
  // make datasets for SPlot constructor
  dsMcM4l = new RooDataSet("dsMcM4l_"+name,"",    tBothMc, RooArgSet(*mass,*totWgt),       "", totWgt->GetName());
  dsMc2d  = new RooDataSet("dsMc2d_"+name,"",     tBothMc, RooArgSet(*mass,*xVar,*totWgt), "", totWgt->GetName());

  nSigExp = hSigM4l->Integral();
  nBkgExp = hBkgM4l->Integral();
  N_S_Local->setVal(nSigExp);
  N_BG->setVal(nBkgExp);
  cout << "    nSigExp: " << setw(15) << nSigExp << "   nBkgExp: " << setw(15) << nBkgExp << endl;

  Nuisances = new RooArgSet("Nuisances_"+name);
  N_BGNuis = new Nuisance("N_BGNuis_"+name, N_BG, false, N_BG->getVal(), 0.1*N_BG->getVal()); // add a nuisance for the bg normalization
  // if(nDataMax<0) 
    Nuisances->add(*N_BGNuis->pdf);
  dhSig2d  = new RooDataHist("dhSig2d_"+name,"sig "+name, RooArgSet(*mass,*xVar), hSig2d);
  dhBkg2d  = new RooDataHist("dhBkg2d_"+name,"bkg "+name, RooArgSet(*mass,*xVar), hBkg2d);
  dhSigM4l = new RooDataHist("dhSigM4l_"+name,"", *mass, hSigM4l);
  hBkgM4l->Smooth(3);
  dhBkgM4l = new RooDataHist("dhBkgM4l_"+name,"", *mass, hBkgM4l, 2);

  if(wsOffic=="") {
    pdfBkgM4l = new M4lModel("pdfBkgM4l_"+name,"", mass, dhBkgM4l);
    if(sigModType!="template") { // fit an analytic model
      pdfSigM4l = new SigModel("pdfSigM4l_"+name, name, mass, sigModType, 0);
      // fit signal model to monte carlo
      cout << "||-------------------> fitting to signal shape in monte carlo:" << endl;
      pdfSigM4l->vars["alpha2"]->setConstant(); // too many DOFs if this is left floating
      pdfSigM4l->fitres = pdfSigM4l->pdf->fitTo(*dhSigM4l,Save(true),SumW2Error(true),PrintLevel(-1),Verbose(false));
      // printRooFitResult(pdfSigM4l->fitres);
      cout << " <------------------- done ||" << endl;
      // later when we fit to data, we only want to vary the nuisances
      pdfSigM4l->vars["m0"]->setConstant(); // i.e. leave dm4l floating
      pdfSigM4l->vars["s0"]->setConstant(); // i.e. leave dsigma floating
      pdfSigM4l->vars["alpha"]->setConstant();
      pdfSigM4l->vars["enn"]->setConstant();
      pdfSigM4l->vars["alpha2"]->setConstant();
      pdfSigM4l->vars["enn2"]->setConstant();
    } else {
      pdfSigM4l = new SigModel("pdfSigM4l_"+name, name, mass, sigModType, dhSigM4l, "");
    }
  } else { // use values from official workspaces
    pdfSigM4l = new SigModel("pdfSigM4l_"+name, name, mass, analyticPdfs["sig"]);
    pdfBkgM4l = new M4lModel("pdfBkgM4l_"+name, name, mass, analyticPdfs["bkg"]);
  }
  pdfSigM4l->makeNuisPdfs(); // make signal shape nuisance pdfs
  Nuisances->add(*pdfSigM4l->nuisancePdfs);

  pdfSigM4l_ext = new RooExtendPdf("pdfSigM4l_ext_"+name,"", *pdfSigM4l->pdf, *N_S); // for combined fit
  pdfBkgM4l_ext = new RooExtendPdf("pdfBkgM4l_ext_"+name,"", *pdfBkgM4l->pdf, *N_BG);
  pdfSigM4l_ext_Local = new RooExtendPdf("pdfSigM4l_ext_Local_"+name,"", *pdfSigM4l->pdf, *N_S_Local); // for fit inside this channel
  totPdfM4l_Local     = new RooAddPdf("totPdfM4l_Local_"+name,"", RooArgList(*pdfSigM4l_ext_Local, *pdfBkgM4l_ext));
  totPdfM4l           = new RooAddPdf("totPdfM4l_"+name,"",       RooArgList(*pdfSigM4l_ext,       *pdfBkgM4l_ext));

  TCanvas cvn;
  RooPlot *frame = mass->frame(Title(name));
  dhSigM4l->plotOn(frame);
  pdfSigM4l->pdf->plotOn(frame);
  // pdfSigM4l->shiftParam("sigma", -1);
  // pdfSigM4l->pdf->plotOn(frame,LineColor(kRed));
  // pdfSigM4l->shiftParam("sigma", +1);
  // pdfSigM4l->shiftParam("sigma", +1);
  // pdfSigM4l->pdf->plotOn(frame,LineColor(kRed));
  frame->Draw();
  cvn.SaveAs(plotdir+"/plots/mc-sig-fit-"+name+".png");
  delete frame;

  RooPlot *frame2 = mass->frame(Title(name));
  dsMcM4l->plotOn(frame2);
  // pdfSigM4l->pdf->plotOn(frame2,LineColor(kRed));
  // pdfBkgM4l->pdf->plotOn(frame2,LineColor(823));
  totPdfM4l_Local->plotOn(frame2);

  TLegend leg(.2,.65,.5,.9);
  leg.SetFillColor(0);
  leg.SetBorderSize(0);
  vector<TH1D*> tmpHists;
  if(wsOffic!="") {
    int icol(0);
    for(unsigned ic=0; ic<pdfSigM4l->compNames.size(); ic++) plotComponent(pdfSigM4l->compNames[ic], pdfSigM4l->pdf, frame2, icol++, &leg, tmpHists);
    for(unsigned ic=0; ic<pdfBkgM4l->compNames.size(); ic++) plotComponent(pdfBkgM4l->compNames[ic], pdfBkgM4l->pdf, frame2, icol++, &leg, tmpHists);
  }  
  frame2->Draw();
  leg.Draw();
  cvn.SaveAs(plotdir+"/plots/mc-fit-"+name+".png");
  for(unsigned ih=0; ih<tmpHists.size(); ih++) delete tmpHists[ih];
  delete frame2;
}
//----------------------------------------------------------------------------------------
void Model::unfoldEm()
{
  allUnfolder->unfold(unfMethod);
  allUnfolder->draw();
  sigUnfolder->unfold(unfMethod);
  sigUnfolder->draw();
  bkgUnfolder->unfold(unfMethod);
  bkgUnfolder->draw();
}
//----------------------------------------------------------------------------------------
void Model::readDataFiles()
{
  tmpFile->cd();
  tdata = new TTree("zznt","zznt");
  double tmpM4l,tmpxVar;
  tdata->Branch("m4l",    &tmpM4l);
  tdata->Branch(xVarName, &tmpxVar);
  hEvtsPerBin = new TH1D(name+"-nEvtsPerBin","", nBinsX, xMin, xMax);
  unsigned nDataMaxEachFile = ((float)nDataMax) / samples["data"]->size();
  for(unsigned ifile=0; ifile<samples["data"]->size(); ifile++) {
    sinfo si((*samples["data"])[ifile]);
    varkeepter vk("Angles/data/zzntvars.txt", "r", si.fname, 0, "zznt");
    cout << "  " << setw(12) << si.name << endl;
    unsigned nPassed(0);
    for(unsigned ientry=0; ientry<vk.getentries(); ientry++) {
      if(nDataMax>0 && nPassed>nDataMaxEachFile) { cout << "    breaking: ientry > " << nDataMaxEachFile << endl; break; }
      vk.getentry(ientry);
      if(vk.getval("m4l")    < massMin)	continue;
      if(vk.getval("m4l")    > massMax)	continue;
      if(vk.getval(xVarName) < xMin)	continue;
      if(vk.getval(xVarName) > xMax)	continue;
      if(cutVarName!="") {
	if(vk.getval(cutVarName) < cutVarMin)	continue;
	if(vk.getval(cutVarName) > cutVarMax)	continue;
      }
      if(channelStr!="" && vk.getval("channel")!=channel) continue;

      tmpM4l  = vk.getval("m4l"); // do it this way so we don't have to worry about whether xVar is float or double in the varkeepter
      tmpxVar = vk.getval(xVarName);
      if(absVal) tmpxVar = fabs(tmpxVar);
      tdata->Fill();
      hEvtsPerBin->Fill(tmpxVar);
      nPassed++;
    }      
  }

  printEvtsPerBin();

  dsDataM4l = new RooDataSet("dsDataM4l_"+name,"", tdata,   RooArgSet(*mass));
  dsData2d  = new RooDataSet("dsData2d_"+name,"",  tdata,   RooArgSet(*mass,*xVar));
    
  cout << "||-------------------> fitting to data:";
  fitresData = totPdfM4l_Local->fitTo(*dsDataM4l,Save(true),SumW2Error(kTRUE),ExternalConstraints(*Nuisances),PrintLevel(debug ? 1 : -1),Verbose(false));
  cout << " <------------------- done ||" << endl;

  pdfSigM4l->setConstant(); // now that we've got the signal model parameters from a data fit, set them constant before passing in to splot

  if(wsOffic=="") {
    cout << "||-------------------> mc signal fit: " << endl;
    printRooFitResult(pdfSigM4l->fitres);
  }

  cout << "||-------------------> data fit: " << endl;
  printRooFitResult(fitresData);
  cout << "data: " << dsDataM4l->numEntries()
       << " (" << N_S_Local->getVal()
       << " + " << N_BG->getVal()
       << " = " << (N_S_Local->getVal() + N_BG->getVal()) << ")" << endl;
  
  TCanvas cvn;
  RooPlot *frame = mass->frame(Title(name));
  dsDataM4l->plotOn(frame);
  // totPdfM4l_Local->plotOn(frame,Components("pdfSigM4l_ext_Local_"+name),LineColor(kRed));
  // totPdfM4l_Local->plotOn(frame,Components("pdfBkgM4l_ext_"+name),LineColor(kRed),LineStyle(kDashed));
  pdfSigM4l->pdf->plotOn(frame,LineColor(kRed));
  pdfBkgM4l->pdf->plotOn(frame,LineColor(823));
  totPdfM4l_Local->plotOn(frame);
  frame->Draw();
  cvn.SaveAs(plotdir+"/plots/data-fit-"+name+".png");
  delete frame;
}
//----------------------------------------------------------------------------------------
void Model::printRooFitResult(RooFitResult *fitres)
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
//----------------------------------------------------------------------------------------
void Model::addScaleResUncertBand(Splotholder *sCe, Splotholder *sLo, Splotholder *sHi)
{
  cout << "adding scale/res uncert: " << endl;
  for(unsigned ih=0; ih<sCe->getResultV(unfold).size(); ih++) {
    TH1D *hCeOrig(sCe->getOrigV(unfold)[ih]);

    TH1D *hCe(sCe->getResultV(unfold)[ih]);
    TH1D *hLo(sLo->getResultV(unfold)[ih]);
    TH1D *hHi(sHi->getResultV(unfold)[ih]);

    for(int ibin=1; ibin<hCe->GetXaxis()->GetNbins()+1; ibin++) {
      double ce(hCe->GetBinContent(ibin));
      double lo(hLo->GetBinContent(ibin));
      double hi(hHi->GetBinContent(ibin));
      double maxDiff = max(fabs(lo-ce), fabs(hi-ce));

      double oldErr(hCe->GetBinError(ibin));
      double newErr = sqrt(oldErr*oldErr + maxDiff*maxDiff);
      // cout
      // 	<< setw(15) << maxDiff
      // 	<< setw(15) << oldErr
      // 	<< " --> "
      // 	<< setw(15) << newErr
      // 	<< endl;
      hCeOrig->SetBinError(ibin, newErr);
    }
  }
}
//----------------------------------------------------------------------------------------
M4lModel *Model::getAnalyticPdf(sinfo &si)
{
  TString proc(si.name);
  M4lModel *pdf;
  TString modelConf(wsOffic+"/"+proc+"-"+name+"-8tev.conf");
  modelConf.ReplaceAll("vttH","VH");
  TString modelName(TString("pdf")+"-"+proc+"-"+(Long_t)si.era+"-"+(Long_t)si.lumi+"-m4l-"+name);
  if(proc == "gfH" || proc == "vbfH" || proc == "vttH") {
    assert(sigModType=="doubleCb");
    pdf = new SigModel(modelName,  name, mass, sigModType, 0, modelConf);
  } else if(proc == "qqZZ") {
    pdf = new QqzzModel(modelName, name, mass, modelConf);
  } else if(proc == "ggZZ") {
    pdf = new GgzzModel(modelName, name, mass, modelConf);
  } else if(proc == "lljj") {
    pdf = new LljjModel(modelName, name, mass, modelConf);
  } else {
    assert(0);
  }
  return pdf;
}
//----------------------------------------------------------------------------------------
void Model::plotComponent(TString name, RooAbsPdf *pdf, RooPlot *fr, int icolor, TLegend *leg, vector<TH1D*> &tmpHists)
{
  vector<int> colors;
  colors.push_back(46);
  colors.push_back(49);
  colors.push_back(30);
  colors.push_back(38);
  colors.push_back(39);
  colors.push_back(41);
  colors.push_back(9);
  colors.push_back(12);
  colors.push_back(8);
  colors.push_back(28);
  colors.push_back(6);
  colors.push_back(823);
  colors.push_back(kBlue-3);
  colors.push_back(kGreen+4);
  colors.push_back(kAzure+4);
  colors.push_back(kOrange+2);
  assert(icolor < (int)colors.size());

  TString hname(name);
  hname.ReplaceAll("m4l","");
  hname.ReplaceAll("4e","");
  hname.ReplaceAll("4m","");
  hname.ReplaceAll("2e2m","");
  hname.ReplaceAll("pdf","");
  hname.ReplaceAll("-"," ");
  TH1D *htmp = new TH1D(hname,"",1,0,1);
  tmpHists.push_back(htmp);
  htmp->SetLineColor(colors[icolor]);
  htmp->SetLineWidth(4);
  pdf->plotOn(fr, Components(name), LineColor(colors[icolor]));
  leg->AddEntry(htmp, htmp->GetName(), "l");
}
//----------------------------------------------------------------------------------------
void Model::printEvtsPerBin(TH1D *hist)
{
  if(!hist) hist = hEvtsPerBin;
  cout << "Events per bin: " << endl;
  for(int ib=0; ib<hist->GetXaxis()->GetNbins()+2; ib++)
    cout << setw(15) << ib << setw(15) << hist->GetBinCenter(ib) << setw(15) << hist->GetBinContent(ib) << endl;
}
