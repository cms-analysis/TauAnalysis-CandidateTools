#include "TauAnalysis/CandidateTools/interface/svFitAuxFunctions.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Math/interface/Vector3D.h"

#include <TMath.h>
#include <Math/VectorUtil.h>

#include <limits>

namespace SVfit_namespace 
{
  // Adapted for our vector types from TVector3 class
  reco::Candidate::Vector rotateUz(const math::RThetaPhiVector& toRotate, const reco::Candidate::Vector& newUzVector)
  {
    // NB: newUzVector must be a unit vector !
    Double_t u1 = newUzVector.X();
    Double_t u2 = newUzVector.Y();
    Double_t u3 = newUzVector.Z();
    Double_t up = u1*u1 + u2*u2;

    Double_t fX = toRotate.X();
    Double_t fY = toRotate.Y();
    Double_t fZ = toRotate.Z();

    if ( up ) {
      up = TMath::Sqrt(up);
      Double_t px = fX,  py = fY,  pz = fZ;
      fX = (u1*u3*px - u2*py + u1*up*pz)/up;
      fY = (u2*u3*px + u1*py + u2*up*pz)/up;
      fZ = (u3*u3*px -    px + u3*up*pz)/up;
    } else if ( u3 < 0. ) {
      fX = -fX;
      fZ = -fZ;
    } else {}; // phi = 0, theta = pi

    return reco::Candidate::Vector(fX, fY, fZ);
  }

  reco::Candidate::LorentzVector boostToCOM(
      const reco::Candidate::LorentzVector& comSystem,
      const reco::Candidate::LorentzVector& p4ToBoost) {
    reco::Candidate::Vector boost = comSystem.BoostToCM();
    return ROOT::Math::VectorUtil::boost(p4ToBoost, boost);
  }

  reco::Candidate::LorentzVector boostToLab(
      const reco::Candidate::LorentzVector& rfSystem,
      const reco::Candidate::LorentzVector& p4ToBoost) {
    reco::Candidate::Vector boost = rfSystem.BoostToCM();
    return ROOT::Math::VectorUtil::boost(p4ToBoost, -boost);
  }

  double gjAngleFromX(double x, double visMass, double pVis_rf, double enVis_lab, double motherMass) 
  {
    double enVis_rf = energyFromMomentum(pVis_rf, visMass);
    double beta = TMath::Sqrt(1. - square(motherMass*x/enVis_lab));
    double cosGjAngle = (motherMass*x - enVis_rf)/(pVis_rf*beta);
    double gjAngle = TMath::ACos(cosGjAngle);
    return gjAngle;
  }

  double pVisRestFrame(double visMass, double invisMass, double motherMass)
  {
    double motherMass2 = motherMass*motherMass;
    double pVis = TMath::Sqrt((motherMass2 - square(visMass + invisMass))
                             *(motherMass2 - square(visMass - invisMass)))/(2.*motherMass);
    return pVis;
  }

  double gjAngleToLabFrame(double pVisRestFrame, double gjAngle, double pVisLabFrame)
  {
    // Get the compenent of the rest frame momentum perpindicular to the tau
    // boost direction. This quantity is Lorentz invariant.
    double pVisRestFramePerp = pVisRestFrame*TMath::Sin(gjAngle);

    // Determine the corresponding opening angle in the LAB frame
    double gjAngleLabFrame = TMath::ASin(pVisRestFramePerp/pVisLabFrame);

    return gjAngleLabFrame;
  }

  double motherMomentumLabFrame(double visMass, double pVisRestFrame, double gjAngle, double pVisLabFrame, double motherMass)
  {
    // Determine the corresponding opening angle in the LAB frame
    double angleVisLabFrame = gjAngleToLabFrame(pVisRestFrame, gjAngle, pVisLabFrame);

    // Get the visible momentum perpindicular/parallel to the tau boost direction in the LAB
    double pVisLabFrame_parallel = pVisLabFrame*TMath::Cos(angleVisLabFrame);

    // Now use the Lorentz equation for pVis along the tau direction to solve for
    // the gamma of the tau boost.
    double pVisRestFrame_parallel = pVisRestFrame*TMath::Cos(gjAngle);
    double enVisRestFrame = TMath::Sqrt(square(visMass) + square(pVisRestFrame));

    double gamma = (enVisRestFrame*TMath::Sqrt(square(enVisRestFrame) + square(pVisLabFrame_parallel) - square(pVisRestFrame_parallel)) 
                  - pVisRestFrame_parallel*pVisLabFrame_parallel)/(square(enVisRestFrame) - square(pVisRestFrame_parallel));

    double pMotherLabFrame = TMath::Sqrt(square(gamma) - 1)*motherMass;

    return pMotherLabFrame;
  }

  reco::Candidate::Vector motherDirection(const reco::Candidate::Vector& pVisLabFrame, double angleVisLabFrame, double phiLab) 
  {
    // The direction is defined using polar coordinates in a system where the visible energy
    // defines the Z axis.
    math::RThetaPhiVector motherDirectionVisibleSystem(1.0, angleVisLabFrame, phiLab);

    // Rotate into the LAB coordinate system
    return rotateUz(motherDirectionVisibleSystem, pVisLabFrame.Unit());
  }

  reco::Candidate::LorentzVector motherP4(
    const reco::Candidate::Vector& motherDirection, double motherMomentumLabFrame, double motherMass) 
  {
    // NB: tauDirection must be a unit vector !
    reco::Candidate::LorentzVector motherP4LabFrame =
      reco::Candidate::LorentzVector(
          motherDirection.x()*motherMomentumLabFrame,
          motherDirection.y()*motherMomentumLabFrame,
          motherDirection.z()*motherMomentumLabFrame,
          TMath::Sqrt(motherMomentumLabFrame*motherMomentumLabFrame
            + motherMass*motherMass));

    return motherP4LabFrame;
  }

  double decayAngleFromLabMomenta(const reco::Candidate::LorentzVector& motherP4, const reco::Candidate::LorentzVector& visP4)
  {
    double decayAngle_rf = 0.;
    reco::Candidate::LorentzVector visP4_rf = boostToCOM(motherP4, visP4);
    if ( (motherP4.pt()*visP4_rf.pt()) > 0. ) {
      double scalarProduct = (motherP4.px()*visP4_rf.px() 
                            + motherP4.py()*visP4_rf.py() 
                            + motherP4.pz()*visP4_rf.pz())/(motherP4.P()*visP4_rf.P());
      decayAngle_rf = TMath::ACos(scalarProduct);
    }
    return decayAngle_rf;
  }

  //
  //-------------------------------------------------------------------------------
  //

  double logGaussian(double residual, double sigma)
  {
    if ( sigma > 0. ) {
      return -0.5*TMath::Log(2*TMath::Pi()*square(sigma)) - 0.5*square(residual/sigma);
    } else {
      edm::LogError ("logGaussian")
	<< " Parameter sigma must not be zero !!";
      return std::numeric_limits<float>::min();
    }
  }

  //
  //-------------------------------------------------------------------------------
  //

  void extractHistogramProperties(const TH1* histogram, const TH1* histogram_density,
				  double& xMaximum,  double& xMaximum_interpol, 
				  double& xMean,
				  double& xQuantile016, double& xQuantile050, double& xQuantile084)
  {
//--- compute median, -1 sigma and +1 sigma limits on reconstructed mass
    if ( histogram->Integral() > 0. ) {
      Double_t q[3];
      Double_t probSum[3];
      probSum[0] = 0.16;
      probSum[1] = 0.50;
      probSum[2] = 0.84;
      (const_cast<TH1*>(histogram))->GetQuantiles(3, q, probSum);
      xQuantile016 = q[0];
      xQuantile050 = q[1];
      xQuantile084 = q[2];
    } else {
      xQuantile016 = 0.;
      xQuantile050 = 0.;
      xQuantile084 = 0.;
    }
    
    xMean = histogram->GetMean();
    
    if ( histogram_density->Integral() > 0. ) {    
      int binMaximum = histogram_density->GetMaximumBin();
      xMaximum = histogram_density->GetBinCenter(binMaximum);
      if ( binMaximum > 1 && binMaximum < histogram_density->GetNbinsX() ) {
	double yMaximum   = histogram_density->GetBinContent(binMaximum);
	
	int binLeft       = binMaximum - 1;
	double xLeft      = histogram_density->GetBinCenter(binLeft);
	double yLeft      = histogram_density->GetBinContent(binLeft);    
	
	int binRight      = binMaximum + 1;
	double xRight     = histogram_density->GetBinCenter(binRight);
	double yRight     = histogram_density->GetBinContent(binRight); 
	
	double xMinus     = xLeft - xMaximum;
	double yMinus     = yLeft - yMaximum;
	double xPlus      = xRight - xMaximum;
	double yPlus      = yRight - yMaximum;
	
	xMaximum_interpol = xMaximum + 0.5*(yPlus*square(xMinus) - yMinus*square(xPlus))/(yPlus*xMinus - yMinus*xPlus);
      } else {
	xMaximum_interpol = xMaximum;
      }
    } else {
      xMaximum = 0.;
      xMaximum_interpol = 0.;
    }
  }
}
