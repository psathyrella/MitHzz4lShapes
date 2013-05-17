#include "Splotholder.h"

Splotholder::Splotholder(TString name_, TString channelName, TString plotdir_, RooRealVar *discr, RooRealVar *xvar_, int nBinsX_, float xMin_, float xMax_,
			 // NOTE: xMin is going to be xMinPlot, i.e. 0 if absVal is true
			 RooDataSet *dsDiscr, RooAbsPdf *pdf, RooDataSet *ds2d_,
			 RooArgList &yieldVars_,
			 // yield var name, channel name, comboStr, mc reco hist, mc gen hist, response matrix
			 vector<tuple<TString,TString,TString,TString,TH1D*,TH1D*> > &components_,
			 vector<UpDownHists*> &errors_):                           // pdf, renfac, etc
  
  name(name_),
  plotdir(plotdir_),
  channel(channelName),
  nBinsX(nBinsX_),
  xMin(xMin_),
  xMax(xMax_),
  xvar(xvar_),
  ds2d(ds2d_),
  components(&components_),
  yieldVars(&yieldVars_),
  errors(&errors_)
{
  assert(dsDiscr->numEntries() == ds2d->numEntries());
  for(unsigned icomp=0; icomp<components->size(); icomp++) {
    TString yieldVarName = get<0>((*components)[icomp]);
    RooRealVar *yieldVar = (RooRealVar*)yieldVars_.find(yieldVarName);
    assert(yieldVar);
    yieldVarV.push_back(yieldVar);
    TString chan(get<1>((*components)[icomp]));
    chans.push_back(chan);
    plotComboStrs.push_back(get<2>((*components)[icomp]));
    fillComboStrs.push_back(get<3>((*components)[icomp]));
    hResultV.push_back(   new TH1D(TString("hResult_")+yieldVarName+"_"+chan+"_"+name, yieldVar->GetTitle(), nBinsX, xMin, xMax));
    hUnfResultV.push_back(new TH1D(TString("hUnf_")+yieldVarName+"_"+chan+"_"+name, yieldVar->GetTitle(), nBinsX, xMin, xMax));
    if(hResultV.back()->GetSumw2N()    == 0) hResultV.back()->Sumw2();
    if(hUnfResultV.back()->GetSumw2N() == 0) hUnfResultV.back()->Sumw2();
    histsToDelete.push_back(hResultV.back());
    histsToDelete.push_back(hUnfResultV.back());
    TH1D *hist = get<4>((*components)[icomp]); // mc reco hist
    if(hist) {
      hOrigV.push_back(hist);
      hOrigV.back()->SetTitle(yieldVar->GetTitle());
      if(hOrigV.back()->GetSumw2N() == 0) hOrigV.back()->Sumw2();
    }
    TH1D *genHist = get<5>((*components)[icomp]); // mc gen hist
    if(genHist) {
      hGenOrigV.push_back(genHist);
      hGenOrigV.back()->SetTitle(yieldVar->GetTitle());
      if(hGenOrigV.back()->GetSumw2N() == 0) hGenOrigV.back()->Sumw2();
    }
  }

  // cout << "Splotholder::Splotholder" << endl;
  // for(unsigned ic=0; ic<chans.size(); ic++) {
  //   cout
  //     << setw(15) << chans[ic]
  //     << setw(15) << plotComboStrs[ic]
  //     << setw(35) << hResultV[ic]->GetName();
  //   if(hOrigV.size() > 0)    cout << setw(15) << hOrigV[ic]->GetName();
  //   if(hGenOrigV.size() > 0) cout << setw(15) << hGenOrigV[ic]->GetName();
  //   cout << endl;
  // }
  cout << "    starting SPlot constructor" << endl;
  splot = new RooStats::SPlot("splot_"+name,"splot of m4l: "+name, *dsDiscr, pdf, yieldVars_); // NOTE: splot *modifies* the dataset, in particular you can't pass a dataset through splot twice because all hell breaks loose
}
//----------------------------------------------------------------------------------------
Splotholder::~Splotholder()
{
  delete splot;
  for(unsigned ih=0; ih<histsToDelete.size(); ih++) delete histsToDelete[ih];
}
// //----------------------------------------------------------------------------------------
// void Splotholder::fillResultHists(RooUnfoldResponse *unfResp)
// {
//   vector<TString> subChans;
//   subChans.push_back("");
//   map<TString,RooUnfoldResponse*> unfResps;
//   if(unfResp) unfResps[""] = unfResp;
//   fillResultHists(subChans, unfResps);
// }
//----------------------------------------------------------------------------------------
void Splotholder::fillResultHists(vector<TString> &subChans, map<TString,RooUnfoldResponse*> &unfResps, TString unfMethod)
{
  vector<map<TString,pair<TH1D*,RooUnfoldResponse*> > > fillHists; // for each input hist, we need a map to one or more hists (e.g. for each channel)
  for(unsigned icomp=0; icomp<components->size(); icomp++) {
    TString howToFill(fillComboStrs[icomp]);
    TString howToPlot(plotComboStrs[icomp]);
    TH1D *hResult(hResultV[icomp]);
    map<TString,pair<TH1D*,RooUnfoldResponse*> > hMap;
    if(howToFill=="all") { // make a hist for each channel to fill separately
      for(unsigned ich=0; ich<subChans.size(); ich++) {
	TH1D *hTmp = new TH1D(*hResult);
	histsToDelete.push_back(hTmp);
	assert(hTmp->GetSumw2N() != 0);
	hTmp->SetName(TString(hResult->GetName())+"_"+subChans[ich]);
	RooUnfoldResponse *unfResp(0);
	if(unfResps.size() > 0) {
	  assert(unfResps.find(howToPlot+"_"+subChans[ich]) != unfResps.end());
	  unfResp = unfResps[howToPlot+"_"+subChans[ich]];
	}
	pair<TH1D*,RooUnfoldResponse*> pr(hTmp,unfResp);
	hMap[subChans[ich]] = pr;
      }
    } else { // only filling for a single channel
      TH1D *hTmp = new TH1D(*hResult);
      histsToDelete.push_back(hTmp);
      hTmp->SetName(TString(hResult->GetName())+"_"+howToFill);
      RooUnfoldResponse *unfResp(0);
      if(unfResps.size() > 0) {
	assert(unfResps.find(howToPlot+"_"+howToFill) != unfResps.end());
	unfResp = unfResps[howToPlot+"_"+howToFill];
      }
      pair<TH1D*,RooUnfoldResponse*> pr(hTmp,unfResp);
      hMap[howToFill] = pr;
    }
    fillHists.push_back(hMap);
  }
  
  for(int ientry=0; ientry<ds2d->numEntries(); ientry++) {
    for(unsigned icomp=0; icomp<components->size(); icomp++) {
      TString yieldVarName(yieldVarV[icomp]->GetName());
      RooRealVar *yieldVar = (RooRealVar*)yieldVars->find(yieldVarName);
      assert(yieldVar);

      TString histChan(""); // if not split into channels, it's the empty string
      if(((RooRealVar*)ds2d->get(ientry)->find("categs"))) { // if dataset has a category var (otherwise we just fill with everything)
      	TString evtChan = ((RooCategory*)ds2d->get(ientry)->find("categs"))->getLabel();
	// if(yieldVarName.Contains("_S_") && evtChan!=histChan) continue; // only fill histograms with the proper channel
	if(fillHists[icomp].find(evtChan) == fillHists[icomp].end()) // not filling a hist for this component for this channel
	  continue;
	histChan = evtChan;
      }

      // hResultV[icomp]->Fill(
      assert(fillHists[icomp].find(histChan) != fillHists[icomp].end());
      float xVal(((RooRealVar*)ds2d->get(ientry)->find(xvar->GetName()))->getVal());
      float sWgt(splot->GetSWeight(ientry,yieldVar->GetName()));
      fillHists[icomp][histChan].first->Fill(xVal,sWgt);
      
      // if(ientry<40) cout
      // 		      << "    filled: "
      // 		      // << setw(40) << yieldVar->GetName()
      // 		      // << setw(40) << hResultV[icomp]->GetName()
      // 		      << setw(15) << histChan
      // 		      << setw(15) << ((RooRealVar*)ds2d->get(ientry)->find(xvar->GetName()))->getVal()
      // 		      << setw(15) << ((RooRealVar*)ds2d->get(ientry)->find("m4l"))->getVal()
      // 		      << setw(15) << splot->GetSWeight(ientry,yieldVar->GetName())
      // 		      << endl;
    }
  }

  assert(fillHists.size() == hResultV.size());

  // UNFOLD HERE
  if(unfResps.size() > 0) {
    vector<map<TString,TH1D*> > unfFillHists; // unfolded histogram for each filled histogram
    for(unsigned icomp=0; icomp<fillHists.size(); icomp++) {
      map<TString,TH1D*> hUnfMap;
      map<TString,pair<TH1D*,RooUnfoldResponse*> > *hMap(&fillHists[icomp]);
      // cout << "unfolding: " << icomp << endl;
      for(map<TString,pair<TH1D*,RooUnfoldResponse*> >::iterator it=hMap->begin(); it!=hMap->end(); it++) {
	TString chan((*it).first);
	TH1D *hist((*it).second.first);
	RooUnfoldResponse *unfResp((*it).second.second);
	// cout << "    " << setw(15) << chan << " " << hist->GetName() << endl;
	TH1D *hUnf(unfoldHist(hist,unfResp,unfMethod));
	hUnfMap[chan] = hUnf;
	histsToDelete.push_back(hUnf);
      }
      unfFillHists.push_back(hUnfMap);
    }
    // COMBINE hists into hResultV
    assert(hUnfResultV.size() == unfFillHists.size());
    // cout << "combining unfolded hists:" << endl;
    for(unsigned icomp=0; icomp<unfFillHists.size(); icomp++) {
      TH1D *hUnfResult(hUnfResultV[icomp]);
      map<TString,TH1D*> *hMap(&unfFillHists[icomp]);
      // cout << "  " << icomp << endl;
      for(map<TString,TH1D*>::iterator it=hMap->begin(); it!=hMap->end(); it++) {
	// cout << "    " << setw(15) << (*it).first << " " << (*it).second->GetName() << endl;
	hUnfResult->Add((*it).second);
      }
    }
  }
  
  // COMBINE hists into hResultV
  // cout << "combining result hists:" << endl;
  for(unsigned icomp=0; icomp<fillHists.size(); icomp++) {
    TH1D *hResult(hResultV[icomp]);
    map<TString,pair<TH1D*,RooUnfoldResponse*> > *hMap(&fillHists[icomp]);
    // cout << "  " << icomp << endl;
    for(map<TString,pair<TH1D*,RooUnfoldResponse*> >::iterator it=hMap->begin(); it!=hMap->end(); it++) {
      // cout << "    " << setw(15) << (*it).first << " " << (*it).second->GetName() << endl;
      hResult->Add((*it).second.first);
    }
  }

  setMcErrors();
}
//----------------------------------------------------------------------------------------
TH1D *Splotholder::unfoldHist(TH1D *hist, RooUnfoldResponse* unfResp, TString unfMethod)
{
  Unfolder unfolder(hist->GetName(), hist->GetName(), "", "", "", unfResp, hist, 0, unfMethod); // don't need the strings if we're not plotting it
  TH1D *hTmpResult = new TH1D(*unfolder.hUnfX);
  TString responseName(hist->GetName());
  hTmpResult->SetName(responseName.ReplaceAll("Result", "Unf"));
  return hTmpResult;
}
//----------------------------------------------------------------------------------------
vector<TH1D*>& Splotholder::getResultV(bool unfolded)
{
  if(hCombResultV.size() > 0) {
    if(unfolded) return hCombUnfResultV;
    else         return hCombResultV;
  } else {
    if(unfolded) return hUnfResultV;
    else         return hResultV;
  }
}
//----------------------------------------------------------------------------------------
vector<TH1D*>& Splotholder::getOrigV(bool unfolded)
{
  if(hCombOrigV.size() > 0) {
    if(unfolded) return hCombGenOrigV;
    else         return hCombOrigV;
  } else {
    if(unfolded) return hGenOrigV;
    else         return hOrigV;
  }
}
//----------------------------------------------------------------------------------------
void Splotholder::prepOrigHist(TH1D *hist, int color, double &ymax)
{
  assert(hist);
  // cout << "WARN: not rescaling histograms" << endl;
  if(hist->Integral() != 0)
    hist->Scale(1./hist->Integral());
  else
    cout << "\n\nHEY! " << hist->GetName() << " has integral of zero\n" << endl;
  hist->SetTitle(hist->GetTitle()+TString(";")+xvar->GetTitle()+";norm.");
  hist->SetLineWidth(4);
  hist->SetLineColor(color);
  hist->SetFillColor(color);
  hist->SetFillStyle(1);
  hist->SetMarkerSize(0);
  ymax = max(ymax,hist->GetMaximum());
}
//----------------------------------------------------------------------------------------
void Splotholder::prepResultHist(TH1D *hist, int color, double &ymax)
{
  // cout << "WARN: not rescaling histograms" << endl;
  if(hist->Integral() != 0)
    hist->Scale(1./hist->Integral());
  hist->SetLineColor(color);
  hist->SetLineWidth(4);
  hist->SetMarkerSize(0);
  ymax = max(ymax,hist->GetMaximum());
}
//----------------------------------------------------------------------------------------
void Splotholder::combineEach(vector<TH1D*> &hists, vector<TH1D*> &combHists)
{
  if(hists.size() == 0) return;

  // cout << "Splotholder::combineEach;" << endl;
  map<TString,TH1D*> newComboHists;
  // cout << "  making hists" << endl;
  for(unsigned icomb=0; icomb<components->size(); icomb++) {
    TString comboName(plotComboStrs[icomb]);
    if(newComboHists.find(comboName) != newComboHists.end()) continue; // already made this hist
    TH1D *hCombined = new TH1D(*hists[0]);
    hCombined->SetTitle(channel);
    // cout << "  " << setw(15) << comboName << " " << hCombined->GetName() << endl;
    histsToDelete.push_back(hCombined);
    hCombined->SetName(comboName+hists[0]->GetName());
    hCombined->SetTitle(comboName);
    if(hCombined->GetSumw2N() == 0) hCombined->Sumw2();
    hCombined->Reset();
    combHists.push_back(hCombined);
    newComboHists[comboName] = hCombined;
  }

  assert(hists.size() == plotComboStrs.size());
  // cout << "  combining" << endl;
  for(unsigned ich=0; ich<hists.size(); ich++) {
    // cout << "    " << setw(35) << hists[ich]->GetName() << " " << plotComboStrs[ich] << endl;
    if(plotComboStrs[ich] != "") newComboHists[plotComboStrs[ich]]->Add(hists[ich]);
    else                         combHists.push_back(hists[ich]);
  }
}
//----------------------------------------------------------------------------------------
void Splotholder::combineChannels()
{
  combineEach(hResultV,    hCombResultV);
  combineEach(hUnfResultV, hCombUnfResultV); // make sure this happens after you've unfolded and thus filled it
  combineEach(hOrigV,      hCombOrigV);
  combineEach(hGenOrigV,   hCombGenOrigV);   // make sure this happens after you've unfolded and thus filled it
}
//----------------------------------------------------------------------------------------
void Splotholder::plot(Splotholder *sp2, bool unfolded)
// In order to combine channels, the hists corresponding the the indices in chsToCombine are added together
// in a new hist that's pushed to the back of the vectors
{
  bool drawFolded(unfolded);
  double ymin(0),ymax(0),yminChi2(0),ymaxChi2(0);

  double maxFac(1.2);
  // if(TString(xvar->GetName()).Contains("y")) maxFac = 1.4;

  TCanvas cvn;
  if(hOrigV.size()>0) {
    cvn.Divide(1,2);
    cvn.cd(1);
    gPad->SetPad(0,.2,1,1);
    cvn.cd(2);
    gPad->SetPad(0,0,1,.2);
    cvn.cd(1);
  }

  vector<int> colorOrigV,colorResultV;
  colorOrigV.push_back(kOrange+10); colorResultV.push_back(kRed+2);
  colorOrigV.push_back(429);        colorResultV.push_back(kBlue);
  colorOrigV.push_back(kGreen-6);   colorResultV.push_back(kGreen+3);
  colorOrigV.push_back(kViolet-5);  colorResultV.push_back(kMagenta+3);

  TLegend leg(.6,.7,1,.9);
  leg.SetBorderSize(0);
  leg.SetFillStyle(0);
  vector<TH1D> hChi2v;
  for(unsigned ich=0; ich<getResultV(unfolded).size(); ich++) {
    
    TH1D *hResult(getResultV(unfolded)[ich]);
    assert(hResult);
    int colOrig   = (colorOrigV.size()>ich)   ? colorOrigV[ich]   : 1;
    int colResult = (colorResultV.size()>ich) ? colorResultV[ich] : 1;
    prepResultHist(hResult, colResult, ymax);
    TH1D *hOrig = (getOrigV(unfolded).size()>ich) ? getOrigV(unfolded)[ich] : 0;
    if(hOrig) {
      prepOrigHist(hOrig, colOrig, ymax);
      double chi2;
      hChi2v.push_back(getChi2(TString(hOrig->GetName())+"_"+name, hOrig, hResult, chi2, yminChi2, ymaxChi2));
      TString chi2str;
      stringstream ss;
      ss << chi2;
      ss >> chi2str;
      leg.AddEntry(hOrig, TString(hOrig->GetTitle()).ReplaceAll(channel+" ","")/*+": "+chi2str*/, "l");

      if(drawFolded) {
	TH1D *hOrigFolded = (getOrigV(false).size()>ich) ? getOrigV(false)[ich] : 0;
	prepOrigHist(hOrigFolded, colOrig, ymax);
	hOrigFolded->SetFillColor(0);
      }
    }
    leg.AddEntry(hResult, TString("inferred ")+TString(hOrig->GetTitle()).ReplaceAll(channel+" ",""), "l");
  }

  assert(getOrigV(unfolded).size() > 0);
  TH1D *hStart(getOrigV(unfolded)[0]);
  // TH1D hframe("hframe",channel+";"+hStart->GetXaxis()->GetTitle()+";"+hStart->GetYaxis()->GetTitle(),hStart->GetXaxis()->GetNbins(),hStart->GetXaxis()->GetXmin(),hStart->GetXaxis()->GetXmax());
  TH1D hframe("hframe",channel+";"+hStart->GetXaxis()->GetTitle()+";"+hStart->GetYaxis()->GetTitle(),nBinsX,xMin,xMax);
  hframe.SetMaximum(maxFac*ymax);
  hframe.SetMinimum(0.75*hStart->GetMinimum());
  hframe.Reset();
  hframe.Draw();
  for(unsigned ich=0; ich<getResultV(unfolded).size(); ich++) {
    TH1D *hResult(getResultV(unfolded)[ich]);
    TH1D *hOrig = (getOrigV(unfolded).size()>ich) ? getOrigV(unfolded)[ich] : 0;
    TString origDrawStr("e3same");
    if(hOrig) hOrig->Draw(origDrawStr);
    hResult->Draw("esame");
  }

  leg.Draw();

  cvn.cd(2);
  if(getOrigV(unfolded).size()>0) {
    for(unsigned ich=0; ich<hChi2v.size(); ich++) {
      TString drawStr("e");
      if(ich!=0) drawStr += "same";
      hChi2v[ich].SetMinimum(floor(yminChi2));
      hChi2v[ich].SetMaximum(ceil(ymaxChi2));
      hChi2v[ich].Draw(drawStr);
    }
  }
  TLine zLine(xMin,0,xMax,0);
  zLine.SetLineStyle(kDashed);
  zLine.Draw();
  cvn.SaveAs(plotdir+"/plots/result-mc-"+name+".png");

  if(drawFolded) {
    for(unsigned ich=0; ich<hResultV.size(); ich++) {
      TCanvas cvn2;
      ymin = ymax = 0;
      int colOrig   = (colorOrigV.size()>ich)   ? colorOrigV[ich]   : 1;
      int colResult = (colorResultV.size()>ich) ? colorResultV[ich] : 1;
      TH1D *hResult(hResultV[ich]);
      TH1D *hUnfResult(hUnfResultV[ich]);
      TH1D *hOrig(hOrigV[ich]);
      TH1D *hGenOrig(hGenOrigV[ich]);
      prepResultHist(hResult,    colResult, ymax);
      prepResultHist(hUnfResult, colResult, ymax);
      prepOrigHist(hOrig,        colOrig,   ymax);
      prepOrigHist(hGenOrig,     colOrig,   ymax);
      hResult->SetLineColor(1);
      hOrig->SetFillColor(0);
      hOrig->SetLineColor(1);
      hOrig->SetLineWidth(4);
      hOrig->SetLineStyle(kDashed);
      hGenOrig->SetFillColor(0);

      hResult->SetMaximum(maxFac*ymax);
      hResult->SetMinimum(0);
      hResult->Draw("e");
      hUnfResult->Draw("esame");
      hOrig->Draw("histsame");
      hGenOrig->Draw("histsame");

      TLegend leg(.6,.7,.95,.9);
      leg.SetBorderSize(0);
      leg.SetFillStyle(0);
      leg.AddEntry(hResult,    "reco splot", "l");
      leg.AddEntry(hUnfResult, "unf splot",  "l");
      leg.AddEntry(hOrig,      "reco orig", "l");
      leg.AddEntry(hGenOrig,   "gen orig", "l");
      leg.Draw();
      
      cvn2.SaveAs(plotdir+"/plots/result-mc-"+plotComboStrs[ich]+"_"+name+".png");
      // need to reset things for data draw below
      prepOrigHist(hOrig,        colOrig,   ymax);
      prepOrigHist(hGenOrig,     colOrig,   ymax);
    }
  }

  // plot data
  if(sp2) {
    TCanvas cvn2;
    for(unsigned ich=0; ich<getResultV(unfolded).size(); ich++) {
      ymin = ymax = 0;
      assert(sp2->getResultV(unfolded).size() > ich);
      TH1D *hData = sp2->getResultV(unfolded)[ich];
      // cout << "WARN: not rescaling histograms" << endl;
      if(hData->Integral() != 0)
      	hData->Scale(1./hData->Integral());
      ymin = min(ymin, hData->GetMinimum());
      ymax = max(ymax, hData->GetMaximum());
      TH1D *hOrig = (getOrigV(unfolded).size()>ich) ? getOrigV(unfolded)[ich] : 0;
      assert(hOrig);
      ymin = min(ymin, hOrig->GetMinimum());
      ymax = max(ymax, hOrig->GetMaximum());
      hOrig->SetMinimum(1.3*maxFac*ymin);
      hOrig->SetMaximum(1.3*maxFac*ymax);
      hOrig->Draw("e3");
      hData->Draw("e0same");
      TLegend dataLeg(.7,.75,.95,.9);    
      dataLeg.SetBorderSize(0);
      dataLeg.SetFillStyle(0);
      dataLeg.AddEntry(hOrig, "expected", "l");
      dataLeg.AddEntry(hData, "data", "p");
      dataLeg.Draw();
      cvn2.SaveAs(plotdir+"/plots/result-data-"+name+"-"+TString(yieldVarV[ich]->GetName())+".png");
    }
  }
}
//----------------------------------------------------------------------------------------
TH1D Splotholder::getChi2(TString name, TH1D *h1, TH1D *h2, double &chi, double &yminChi2, double &ymaxChi2)
{
  TH1D hChi2(*h1);
  hChi2.Reset();
  hChi2.SetNameTitle("hChi2_"+name,";;#chi");

  assert(h1->GetXaxis()->GetNbins() == h2->GetXaxis()->GetNbins());
  double sum2(0);
  for(int ibin=1; ibin<h1->GetXaxis()->GetNbins()+1; ibin++) {
    double val1(h1->GetBinContent(ibin));
    double val2(h2->GetBinContent(ibin));
    double err1(h1->GetBinError(ibin));
    double err2(h2->GetBinError(ibin));

    if(val1==0) err1 = 1.15;
    if(val2==0) err2 = 1.15;

    double chi = (val2-val1) / sqrt(err1*err1 + err2*err2);
    hChi2.SetBinContent(ibin, chi);
    sum2 += chi*chi;
  }
  chi = sum2 / h1->GetXaxis()->GetNbins();

  hChi2.GetXaxis()->SetNdivisions(0);
  hChi2.GetYaxis()->SetNdivisions(2);
  hChi2.GetXaxis()->SetLabelSize(0);
  hChi2.GetYaxis()->SetLabelSize(.2);
  hChi2.GetYaxis()->SetTitleSize(.3);
  hChi2.GetYaxis()->SetTitleOffset(.2);
  hChi2.GetYaxis()->RotateTitle();
  hChi2.GetYaxis()->SetNoExponent(true);
  hChi2.SetFillStyle(0);
  
  yminChi2 = min(yminChi2, hChi2.GetMinimum());
  ymaxChi2 = max(ymaxChi2, hChi2.GetMaximum());

  return hChi2;
}
//----------------------------------------------------------------------------------------
void Splotholder::setMcErrors()
{
  if(errors->size() == 0) return;
  for(unsigned ih=0; ih<hOrigV.size(); ih++) {
    TH1D *hOrig(hOrigV[ih]);
    for(int ibin=0; ibin<hOrig->GetXaxis()->GetNbins(); ibin++) {
      double sum2(0);
      for(unsigned ierr=0; ierr<errors->size(); ierr++) { // add up the sum of squares for each uncertainty we include
	UpDownHists *udh((*errors)[ierr]);
	double err = max( fabs(udh->hErrLo->GetBinContent(ibin)), fabs(udh->hErrHi->GetBinContent(ibin)) );
	sum2 += err*err;
      }
      double stat(hOrig->GetBinError(ibin));
      double error = sqrt(sum2 + stat*stat);
      hOrig->SetBinError(ibin,error);
    }
  }
}
