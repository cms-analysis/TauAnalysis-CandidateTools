#ifndef TauAnalysis_CandidateTools_NSVfitTauToLepLikelihoodPolarization_h
#define TauAnalysis_CandidateTools_NSVfitTauToLepLikelihoodPolarization_h

/** \class NSVfitTauToLepLikelihoodPolarization
 *
 * Plugin for computing likelihood for tau lepton decay 
 *  tau- --> e- nu nu (tau- --> mu- nu nu)
 * to be compatible with matrix element of V-A electroweak decay
 *
 * \author Christian Veelken, UC Davis
 *
 * \version $Revision: 1.1 $
 *
 * $Id: NSVfitTauToLepLikelihoodPolarization.h,v 1.1 2011/03/23 17:46:39 veelken Exp $
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "TauAnalysis/CandidateTools/interface/NSVfitSingleParticleLikelihood.h"

#include "AnalysisDataFormats/TauAnalysis/interface/NSVfitSingleParticleHypothesisBase.h"

template <typename T>
class NSVfitTauToLepLikelihoodPolarization : public NSVfitSingleParticleLikelihood
{
 public:
  NSVfitTauToLepLikelihoodPolarization(const edm::ParameterSet&);
  ~NSVfitTauToLepLikelihoodPolarization();
  
  void beginJob(NSVfitAlgorithmBase*);
  
  double operator()(const NSVfitSingleParticleHypothesisBase*) const;
};

#endif