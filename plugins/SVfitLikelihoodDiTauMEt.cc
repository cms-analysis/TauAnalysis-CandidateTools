#include "TauAnalysis/CandidateTools/plugins/SVfitLikelihoodDiTauMEt.h"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "TauAnalysis/CandidateTools/interface/SVfitAlgorithm.h"
#include "TauAnalysis/CandidateTools/interface/svFitAuxFunctions.h"
#include "TauAnalysis/CandidateTools/interface/candidateAuxFunctions.h"

#include <TMath.h>
#include <TVector2.h>

#include <string>

using namespace SVfit_namespace;

const double parSigmaMin = 5.0;
const double perpSigmaMin = 5.0;

template <typename T1, typename T2>
SVfitLikelihoodDiTauMEt<T1,T2>::SVfitLikelihoodDiTauMEt(const edm::ParameterSet& cfg)
  : SVfitDiTauLikelihoodBase<T1,T2>(cfg),
    parSigma_(0),
    parBias_(0),
    perpSigma_(0),
    perpBias_(0)
{
  edm::ParameterSet cfgResolution = cfg.getParameter<edm::ParameterSet>("resolution");

  parSigma_ = new TFormula("parSigma", cfgResolution.getParameter<std::string>("parSigma").data());
  parBias_ = new TFormula("parBias", cfgResolution.getParameter<std::string>("parBias").data());

  perpSigma_ = new TFormula("perpSigma", cfgResolution.getParameter<std::string>("perpSigma").data());
  perpBias_ = new TFormula("perpBias", cfgResolution.getParameter<std::string>("perpBias").data());

  srcPFCandidates_ = cfg.getParameter<edm::InputTag>("srcPFCandidates");

  varyPhi_ = cfg.exists("varyPhi") ? cfg.getParameter<bool>("varyPhi") : true;
}

template <typename T1, typename T2>
SVfitLikelihoodDiTauMEt<T1,T2>::~SVfitLikelihoodDiTauMEt()
{
  delete parSigma_;
  delete parBias_;

  delete perpSigma_;
  delete perpBias_;
}

template <typename T1, typename T2>
void SVfitLikelihoodDiTauMEt<T1,T2>::beginEvent(const edm::Event& evt, const edm::EventSetup& es)
{
  //std::cout << "<SVfitLikelihoodDiTauMEt::beginEvent>:" << std::endl;

  evt.getByLabel(srcPFCandidates_, pfCandidates_);
}

template <typename T1, typename T2>
void SVfitLikelihoodDiTauMEt<T1,T2>::beginCandidate(const CompositePtrCandidateT1T2MEt<T1,T2>& diTau)
{
  qX_ = diTau.leg1()->px() + diTau.leg2()->px() + diTau.met()->px();
  qY_ = diTau.leg1()->py() + diTau.leg2()->py() + diTau.met()->py();
  qT_ = TMath::Sqrt(qX_*qX_ + qY_*qY_);
}

template <typename T1, typename T2>
bool SVfitLikelihoodDiTauMEt<T1,T2>::isFittedParameter(int index) const
{
  if      (             index == SVfit_namespace::kLeg1thetaRest ) return true;
  else if ( varyPhi_ && index == SVfit_namespace::kLeg1phiLab    ) return true;
  else if (             index == SVfit_namespace::kLeg2thetaRest ) return true;
  else if ( varyPhi_ && index == SVfit_namespace::kLeg2phiLab    ) return true;
  else return false;
}

template <typename T1, typename T2>
double SVfitLikelihoodDiTauMEt<T1,T2>::operator()(const CompositePtrCandidateT1T2MEt<T1,T2>& diTau,
						  const SVfitDiTauSolution& solution) const
{
//--- compute negative log-likelihood for neutrinos produced in tau lepton decays
//    to match missing transverse momentum reconstructed in the event
//
//    NB: MET likelihood is split into perp/par components along (leptonic) leg1 of the diTau object
//
  if ( this->verbosity_ ) {
    std::cout << "SVfitLikelihoodDiTauMEt::operator()>:" << std::endl;
    std::cout << " sumEt = " << diTau.met()->sumEt() << std::endl;
  }

  double parSigma = parSigma_->Eval(qT_);
  if ( parSigma < parSigmaMin ) parSigma = parSigmaMin;
  double parBias = parBias_->Eval(qT_);
  if ( this->verbosity_ ) std::cout << " parSigma = " << parSigma << ", parBias = " << parBias << std::endl;

  double perpSigma = perpSigma_->Eval(qT_);
  if ( perpSigma < perpSigmaMin ) perpSigma = perpSigmaMin;
  double perpBias = perpBias_->Eval(qT_);
  if ( this->verbosity_ ) std::cout << " perpSigma = " << perpSigma << ", perpBias = " << perpBias << std::endl;

  double projCosPhi,  projSinPhi;
  if ( qT_ > 0. ) {
    projCosPhi = (qX_/qT_);
    projSinPhi = (qY_/qT_);
  } else {
//--- make unit vector bisecting tau lepton "legs"
//    and project difference between "true" generated and reconstructed MET
//    in direction parallel and perpendicular to that vector
    TVector2 diTauDirection = getDiTauBisectorDirection(diTau.leg1()->p4(), diTau.leg2()->p4());

    projCosPhi = diTauDirection.X();
    projSinPhi = diTauDirection.Y();
  }

  double metPx = diTau.met()->px();
  double metPy = diTau.met()->py();

  double recoMET_par = (metPx*projCosPhi + metPy*projSinPhi);
  double recoMET_perp = (metPx*projSinPhi - metPy*projCosPhi);
  if ( this->verbosity_ ) {
    std::cout << " recoMET_par = " << recoMET_par << std::endl;
    std::cout << " recoMET_perp = " << recoMET_perp << std::endl;
  }

  reco::Candidate::LorentzVector nuP4 = solution.leg1().p4Invis() + solution.leg2().p4Invis();
  double nuPx = nuP4.px();
  double nuPy = nuP4.py();

  double fittedMET_par = (nuPx*projCosPhi + nuPy*projSinPhi);
  double fittedMET_perp = (nuPx*projSinPhi - nuPy*projCosPhi);
  if ( this->verbosity_ ) {
    std::cout << " fittedMET_par = " << fittedMET_par << std::endl;
    std::cout << " fittedMET_perp = " << fittedMET_perp << std::endl;
  }

  double parResidual = (recoMET_par - fittedMET_par) - parBias;
  double perpResidual = (recoMET_perp - fittedMET_perp) - perpBias;
  if ( this->verbosity_ ) {
    std::cout << " parResidual = " << parResidual << std::endl;
    std::cout << " perpResidual = " << perpResidual << std::endl;
  }

  double negLogLikelihood = -(logGaussian(parResidual, parSigma) + logGaussian(perpResidual, perpSigma));
  if ( this->verbosity_ ) std::cout << "--> negLogLikelihood = " << negLogLikelihood << std::endl;

  return negLogLikelihood;
}

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/Candidate/interface/Candidate.h"

typedef SVfitLikelihoodDiTauMEt<pat::Electron, pat::Tau> SVfitLikelihoodElecTauPairMEt;
typedef SVfitLikelihoodDiTauMEt<pat::Muon, pat::Tau> SVfitLikelihoodMuTauPairMEt;
typedef SVfitLikelihoodDiTauMEt<pat::Tau, pat::Tau> SVfitLikelihoodDiTauPairMEt;
typedef SVfitLikelihoodDiTauMEt<pat::Electron, pat::Muon> SVfitLikelihoodElecMuPairMEt;
typedef SVfitLikelihoodDiTauMEt<pat::Electron, pat::Electron> SVfitLikelihoodDiElecPairMEt;
typedef SVfitLikelihoodDiTauMEt<pat::Muon, pat::Muon> SVfitLikelihoodDiMuPairMEt;
typedef SVfitLikelihoodDiTauMEt<reco::Candidate, reco::Candidate> SVfitLikelihoodDiCandidatePairMEt;

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_EDM_PLUGIN(SVfitElecTauPairLikelihoodBasePluginFactory, SVfitLikelihoodElecTauPairMEt, "SVfitLikelihoodElecTauPairMEt");
DEFINE_EDM_PLUGIN(SVfitMuTauPairLikelihoodBasePluginFactory, SVfitLikelihoodMuTauPairMEt, "SVfitLikelihoodMuTauPairMEt");
DEFINE_EDM_PLUGIN(SVfitDiTauPairLikelihoodBasePluginFactory, SVfitLikelihoodDiTauPairMEt, "SVfitLikelihoodDiTauPairMEt");
DEFINE_EDM_PLUGIN(SVfitElecMuPairLikelihoodBasePluginFactory, SVfitLikelihoodElecMuPairMEt, "SVfitLikelihoodElecMuPairMEt");
DEFINE_EDM_PLUGIN(SVfitDiElecPairLikelihoodBasePluginFactory, SVfitLikelihoodDiElecPairMEt, "SVfitLikelihoodDiElecPairMEt");
DEFINE_EDM_PLUGIN(SVfitDiMuPairLikelihoodBasePluginFactory, SVfitLikelihoodDiMuPairMEt, "SVfitLikelihoodDiMuPairMEt");
DEFINE_EDM_PLUGIN(SVfitDiCandidatePairLikelihoodBasePluginFactory, SVfitLikelihoodDiCandidatePairMEt, "SVfitLikelihoodDiCandidatePairMEt");