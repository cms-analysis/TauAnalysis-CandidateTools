#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Math/GSLMCIntegrator.h"

#include "TauAnalysis/CandidateTools/interface/svFitAuxFunctions.h"
#include "TauAnalysis/CandidateTools/interface/NSVfitStandaloneAlgorithm.h"

NSVfitStandaloneAlgorithm::NSVfitStandaloneAlgorithm(std::vector<NSVfitStandalone::MeasuredTauLepton> measuredTauLeptons, NSVfitStandalone::Vector measuredMET , const TMatrixD& covMET, unsigned int verbosity) 
  : fitStatus_(-1), 
    verbosity_(verbosity), 
    maxObjFunctionCalls_(5000),
    mcObjectiveFunctionAdapter_(0),
    mcPtEtaPhiMassAdapter_(0),
    integrator2_(0),
    isInitialized2_(false),
    maxObjFunctionCalls2_(100000)
{ 
  // instantiate minuit, the arguments might turn into configurables once
  minimizer_ = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");
  // instantiate the combined likelihood
  nll_ = new NSVfitStandalone::NSVfitStandaloneLikelihood(measuredTauLeptons, measuredMET, covMET, (verbosity_ > 2));
}

NSVfitStandaloneAlgorithm::~NSVfitStandaloneAlgorithm() 
{
  delete nll_;
  delete minimizer_;
  delete mcObjectiveFunctionAdapter_;
  delete mcPtEtaPhiMassAdapter_;
  delete integrator2_;
}

void
NSVfitStandaloneAlgorithm::setup()
{
  using namespace NSVfitStandalone;

  if(verbosity_>0){
    std::cout << "<NSVfitStandaloneAlgorithm::setup()>" << std::endl;
  }
  for(unsigned int idx=0; idx<nll_->measuredTauLeptons().size(); ++idx){
    if(verbosity_>0){
      std::cout << " --> upper limit of leg1::mNuNu will be set to "; 
      if(nll_->measuredTauLeptons()[idx].decayType() == kHadDecay){ 
	std::cout << "0";
      } 
      else{
	std::cout << (SVfit_namespace::tauLeptonMass - TMath::Min(nll_->measuredTauLeptons()[idx].mass(), 1.5));
      } 
      std::cout << std::endl;
    }
    // start values for xFrac
    minimizer_->SetLimitedVariable(idx*kMaxFitParams + kXFrac, std::string(TString::Format("leg%d::xFrac", idx + 1)).c_str(), 0.5, 0.1, 0., 1.);
    // start values for nunuMass
    if(nll_->measuredTauLeptons()[idx].decayType() == kHadDecay){ 
      minimizer_->SetFixedVariable(idx*kMaxFitParams + kMNuNu, std::string(TString::Format("leg%d::mNuNu", idx + 1)).c_str(), 0.); 
    } 
    else{ 
      minimizer_->SetLimitedVariable(idx*kMaxFitParams + kMNuNu, std::string(TString::Format("leg%d::mNuNu", idx + 1)).c_str(), 0.8, 0.10, 0., SVfit_namespace::tauLeptonMass - TMath::Min(nll_->measuredTauLeptons()[idx].mass(), 1.5)); 
    }
    // start values for phi
    minimizer_->SetVariable(idx*kMaxFitParams + kPhi, std::string(TString::Format("leg%d::phi", idx + 1)).c_str(), 0.0, 0.25);
  }
}

void
NSVfitStandaloneAlgorithm::fit()
{
  if(verbosity_>0){
    std::cout << "<NSVfitStandaloneAlgorithm::fit()>" << std::endl
	      << "--> dimension of fit    : " << nll_->measuredTauLeptons().size()*NSVfitStandalone::kMaxFitParams << std::endl
	      << "--> maxObjFunctionCalls : " << maxObjFunctionCalls_ << std::endl; 
  }
  // clear minimizer
  minimizer_->Clear();
  // set verbosity level of minimizer
  minimizer_->SetPrintLevel(-1);
  // setup the function to be called and the dimension of the fit
  ROOT::Math::Functor toMinimize(standaloneObjectiveFunctionAdapter_, nll_->measuredTauLeptons().size()*NSVfitStandalone::kMaxFitParams);
  minimizer_->SetFunction(toMinimize); 
  setup();
  minimizer_->SetMaxFunctionCalls(maxObjFunctionCalls_);
  // set Minuit strategy = 2, in order to get reliable error estimates:
  // http://www-cdf.fnal.gov/physics/statistics/recommendations/minuit.html
  minimizer_->SetStrategy(2);
  // compute uncertainties for increase of objective function by 0.5 wrt. 
  // minimum (objective function is log-likelihood function)
  minimizer_->SetErrorDef(0.5);
  if(verbosity_>0){
    std::cout << "--> starting ROOT::Math::Minimizer::Minimize..." << std::endl;
    std::cout << "--> #freeParameters = " << minimizer_->NFree() << ","
  	      << "--> #constrainedParameters = " << (minimizer_->NDim() - minimizer_->NFree()) << std::endl;
  }
  // do the minimization
  nll_->addDelta(false);
  nll_->addSinTheta(true);
  minimizer_->Minimize();
  if(verbosity_>1){ 
    minimizer_->PrintResults(); 
  };
  /* get Minimizer status code, check if solution is valid:
    
     0: Valid solution
     1: Covariance matrix was made positive definite
     2: Hesse matrix is invalid
     3: Estimated distance to minimum (EDM) is above maximum
     4: Reached maximum number of function calls before reaching convergence
     5: Any other failure
  */
  fitStatus_ = minimizer_->Status();
  if(verbosity_>0){ 
    std::cout << "--> fitStatus = " << fitStatus_ << std::endl; 
  }
  
  // and write out the result
  using NSVfitStandalone::kXFrac;
  using NSVfitStandalone::kMNuNu;
  using NSVfitStandalone::kPhi;
  using NSVfitStandalone::kMaxFitParams;
  // update di-tau system with final fit results
  nll_->results(fittedTauLeptons_, minimizer_->X());
  // determine uncertainty of the fitted di-tau mass
  double x1RelErr = minimizer_->Errors()[kXFrac]/minimizer_->X()[kXFrac];
  double x2RelErr = minimizer_->Errors()[kMaxFitParams + kXFrac]/minimizer_->X()[kMaxFitParams + kXFrac];
  // this gives a unified treatment for retrieving the result for integration mode and fit mode
  mass_ = fittedDiTauSystem().mass();
  massUncert_ = TMath::Sqrt(0.25*x1RelErr*x1RelErr + 0.25*x2RelErr*x2RelErr)*fittedDiTauSystem().mass();
  if(verbosity_>1){
    std::cout << ">> -------------------------------------------------------------" << std::endl;
    std::cout << ">> Resonance Record: " << std::endl;
    std::cout << ">> -------------------------------------------------------------" << std::endl;
    std::cout << ">> pt  (di-tau)    = " << fittedDiTauSystem().pt  () << std::endl;
    std::cout << ">> eta (di-tau)    = " << fittedDiTauSystem().eta () << std::endl;
    std::cout << ">> phi (di-tau)    = " << fittedDiTauSystem().phi () << std::endl;
    std::cout << ">> mass(di-tau)    = " << fittedDiTauSystem().mass() << std::endl;  
    std::cout << ">> massUncert      = " << massUncert_ << std::endl
	      << "   error[xFrac1]   = " << minimizer_->Errors()[kXFrac] << std::endl
	      << "   value[xFrac1]   = " << minimizer_->X()[kXFrac]      << std::endl
	      << "   error[xFrac2]   = " << minimizer_->Errors()[kMaxFitParams+kXFrac] << std::endl
	      << "   value[xFrac2]   = " << minimizer_->X()[kMaxFitParams+kXFrac]      << std::endl;
    for(unsigned int leg=0; leg<2; ++leg){
      std::cout << ">> -------------------------------------------------------------" << std::endl;
      std::cout << ">> Leg " << leg+1 << " Record: " << std::endl;
      std::cout << ">> -------------------------------------------------------------" << std::endl;
      std::cout << ">> pt  (meas)      = " << nll_->measuredTauLeptons()[leg].p4().pt () << std::endl;
      std::cout << ">> eta (meas)      = " << nll_->measuredTauLeptons()[leg].p4().eta() << std::endl;
      std::cout << ">> phi (meas)      = " << nll_->measuredTauLeptons()[leg].p4().phi() << std::endl; 
      std::cout << ">> pt  (fit )      = " << fittedTauLeptons()[leg].pt () << std::endl;
      std::cout << ">> eta (fit )      = " << fittedTauLeptons()[leg].eta() << std::endl;
      std::cout << ">> phi (fit )      = " << fittedTauLeptons()[leg].phi() << std::endl; 
    }
  }
}

void
NSVfitStandaloneAlgorithm::integrate()
{
  using namespace NSVfitStandalone;
  
  if(verbosity_>0){
    std::cout << "<NSVfitStandaloneAlgorithm::integrate()>:" << std::endl;
  }

  double pi = 3.14159265;
  // number of hadrponic decays
  int khad = 0;
  for(unsigned int idx=0; idx<nll_->measuredTauLeptons().size(); ++idx){
    if(nll_->measuredTauLeptons()[idx].decayType() == kHadDecay){ 
      khad++; 
    }
  }
  // number of parameters for fit
  int par = nll_->measuredTauLeptons().size()*NSVfitStandalone::kMaxFitParams - (khad + 1);
  /* --------------------------------------------------------------------------------------
     lower and upper bounds for integration. Boundaries are deefined for each decay channel
     separately. The order is: 
     
     - 3dim : fully hadronic {xFrax, phihad1, phihad2}
     - 4dim : semi  leptonic {xFrac, nunuMass, philep, phihad}
     - 5dim : fully leptonic {xFrac, nunuMass1, philep1, nunuMass2, philep2}
     
     xl* defineds the lower integation bound, xu* defines the upper integration bound. 
     ATTENTION: order matters here! In the semi-leptonic decay the lepton must go first in 
     the parametrization, as it is first in the definition of integral boundaries. This is
     the reason why the measuredLeptons are eventually re-ordered in the constructor of 
     this class before passing them on to NSVfitStandaloneLikelihood.
  */
  double xl3[3] = { 0.0, -pi, -pi };
  double xu3[3] = { 1.0,  pi,  pi };
  double xl4[4] = { 0.0, 0.0, -pi, -pi };
  double xu4[4] = { 1.0, SVfit_namespace::tauLeptonMass, pi, pi };
  double xl5[5] = { 0.0, 0.0, -pi, 0.0, -pi };
  double xu5[5] = { 1.0, SVfit_namespace::tauLeptonMass, pi, SVfit_namespace::tauLeptonMass, pi };

  // integrator instance
  //ROOT::Math::IntegratorMultiDim ig2(ROOT::Math::IntegrationMultiDim::kVEGAS, 1.e-12, 1.e-5);
  ROOT::Math::GSLMCIntegrator ig2("vegas", 1.e-12, 1.e-5, 2000);
  ROOT::Math::Functor toIntegrate(&standaloneObjectiveFunctionAdapter_, &ObjectiveFunctionAdapter::Eval, par); 
  standaloneObjectiveFunctionAdapter_.SetPar(par);
  ig2.SetFunction(toIntegrate);
  nll_->addDelta(true);
  nll_->addSinTheta(false);
  int count = 0;
  double pMax = 0.;
  double mtest = measuredDiTauSystem().mass();
  bool skiphighmasstail = false;
  for(int i=0; i<100 && (!skiphighmasstail); ++i){
    standaloneObjectiveFunctionAdapter_.SetM(mtest);
    double p = -1.;
    if(par == 4){
      p = ig2.Integral(xl4, xu4);
    } else if(par == 5){
      p = ig2.Integral(xl5, xu5);
    } else if(par == 3){
      p = ig2.Integral(xl3, xu3);
    } else{
      std::cout << " >> ERROR : the number of measured leptons must be 2" << std::endl;
      assert(0);
    }
    if(verbosity_>1){
      std::cout << "--> scan idx = " << i << "  mtest = " << mtest << "  p = " << p << "  pmax = " << pMax << std::endl;
    }
    if(p>pMax){
      mass_ = mtest;
      pMax  = p;
      count = 0;
    } 
    else{
      if(p<(1.e-3*pMax)){
	++count;
	if(count>= 5){
	  skiphighmasstail=true;
	}
      } 
      else {
	count=0;
      }
    }
    mtest += TMath::Max(2.5, 0.025*mtest);
  }
  if ( verbosity_ > 0 ) {
    std::cout << "--> mass  = " << mass_  << std::endl;
    std::cout << "--> pmax  = " << pMax   << std::endl;
    std::cout << "--> count = " << count  << std::endl;
  }
}

void
NSVfitStandaloneAlgorithm::integrate2()
{
  using namespace NSVfitStandalone;
  
  if(verbosity_>0){
    std::cout << "<NSVfitStandaloneAlgorithm::integrate2()>:" << std::endl;
  }

  if ( isInitialized2_ ) {
    mcPtEtaPhiMassAdapter_->Reset();
  } else {    
    edm::ParameterSet cfg;
    cfg.addParameter<std::string>("mode", "Metropolis");
    cfg.addParameter<std::string>("initMode", "none");
    cfg.addParameter<unsigned>("numIterBurnin", TMath::Nint(0.10*maxObjFunctionCalls2_));
    cfg.addParameter<unsigned>("numIterSampling", maxObjFunctionCalls2_);
    cfg.addParameter<int>("numIterSimAnnealingPhase1", TMath::Nint(0.02*maxObjFunctionCalls2_));    
    cfg.addParameter<int>("numIterSimAnnealingPhase2", TMath::Nint(0.06*maxObjFunctionCalls2_));
    cfg.addParameter<double>("T0", 15.);
    cfg.addParameter<double>("alpha", 1.0 - 1.e+2/maxObjFunctionCalls2_);
    cfg.addParameter<unsigned>("numChains", 1);
    cfg.addParameter<unsigned>("numBatches", 1);
    cfg.addParameter<unsigned>("L", 1);
    cfg.addParameter<double>("epsilon0", 1.e-2);
    cfg.addParameter<double>("nu", 0.71);
    cfg.addParameter<std::string>("name", "NSVfitStandaloneAlgorithm");
    cfg.addParameter<int>("verbosity", 0);
    integrator2_ = new MarkovChainIntegrator(cfg);
    mcObjectiveFunctionAdapter_ = new MCObjectiveFunctionAdapter();
    integrator2_->setIntegrand(*mcObjectiveFunctionAdapter_);
    mcPtEtaPhiMassAdapter_ = new MCPtEtaPhiMassAdapter();
    integrator2_->registerCallBackFunction(*mcPtEtaPhiMassAdapter_);
    isInitialized2_= true;    
  }
  double pi = 3.14159265;
  // number of hadronic decays
  int khad = 0;
  for(unsigned int idx=0; idx<nll_->measuredTauLeptons().size(); ++idx){
    if(nll_->measuredTauLeptons()[idx].decayType() == kHadDecay){ 
      khad++; 
    }
  }
  // number of parameters for fit
  int nDim = nll_->measuredTauLeptons().size()*NSVfitStandalone::kMaxFitParams - (khad + 1);  
  mcObjectiveFunctionAdapter_->SetNDim(nDim);
  /* --------------------------------------------------------------------------------------
     lower and upper bounds for integration. Boundaries are deefined for each decay channel
     separately. The order is: 
     
     - 3dim : fully hadronic {xFrax, phihad1, phihad2}
     - 4dim : semi  leptonic {xFrac, nunuMass, philep, phihad}
     - 5dim : fully leptonic {xFrac, nunuMass1, philep1, nunuMass2, philep2}
     
     xl* defineds the lower integation bound, xu* defines the upper integration bound. 
     ATTENTION: order matters here! In the semi-leptonic decay the lepton must go first in 
     the parametrization, as it is first in the definition of integral boundaries. This is
     the reason why the measuredLeptons are eventually re-ordered in the constructor of 
     this class before passing them on to NSVfitStandaloneLikelihood.
  */
  double x03[3] = { 0.5, 0.0, 0.0 };
  double xl3[3] = { 0.0, -pi, -pi };
  double xu3[3] = { 1.0,  pi,  pi };
  double x04[4] = { 0.5, 0.8, 0.0, 0.0 };
  double xl4[4] = { 0.0, 0.0, -pi, -pi };
  double xu4[4] = { 1.0, SVfit_namespace::tauLeptonMass, pi, pi };
  double x05[5] = { 0.5, 0.8, 0.0, 0.8, 0.0 };
  double xl5[5] = { 0.0, 0.0, -pi, 0.0, -pi };
  double xu5[5] = { 1.0, SVfit_namespace::tauLeptonMass, pi, SVfit_namespace::tauLeptonMass, pi };
  std::vector<double> x0;
  std::vector<double> xl;
  std::vector<double> xu;
  for ( int i = 0; i < nDim; ++i ) {
    if      ( nDim == 3 ) {
      x0[i] = x03[i];
      xl[i] = xl3[i];
      xu[i] = xu3[i];
    } else if ( nDim == 4 ) {
      x0[i] = x04[i];
      xl[i] = xl4[i];
      xu[i] = xu4[i];
    } else if ( nDim == 5 ) {
      x0[i] = x05[i];
      xl[i] = xl5[i];
      xu[i] = xu5[i];
    } else {
      std::cout << " >> ERROR : the number of measured leptons must be 2" << std::endl;
      assert(0);
    }
  }
  integrator2_->initializeStartPosition_and_Momentum(x0);
  nll_->addDelta(false);
  nll_->addSinTheta(false);
  double integral = 0.;
  double integralErr = 0.;
  int errorFlag = 0;
  integrator2_->integrate(xl, xu, integral, integralErr, errorFlag);
  pt_ = mcPtEtaPhiMassAdapter_->getPt();
  eta_ = mcPtEtaPhiMassAdapter_->getEta();
  phi_ = mcPtEtaPhiMassAdapter_->getPhi();
  mass_ = mcPtEtaPhiMassAdapter_->getMass();
  if ( verbosity_ > 0 ) {
    std::cout << "--> Pt = " << pt_ << ", eta = " << eta_ << ", phi = " << phi_ << ", mass  = " << mass_  << std::endl;
  }
}
