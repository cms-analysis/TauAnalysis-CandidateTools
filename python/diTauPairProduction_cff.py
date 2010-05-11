import FWCore.ParameterSet.Config as cms
import copy

#--------------------------------------------------------------------------------
# produce combinations of tau-jet + tau-jet pairs
#--------------------------------------------------------------------------------

allDiTauPairs = cms.EDProducer("PATDiTauPairProducer",
    useLeadingTausOnly = cms.bool(False),
    srcLeg1 = cms.InputTag('cleanPatTaus'),
    srcLeg2 = cms.InputTag('cleanPatTaus'),
    dRmin12 = cms.double(0.3),
    srcMET = cms.InputTag('patMETs'),
    srcGenParticles = cms.InputTag('genParticles'),                                 
    recoMode = cms.string(""),
    scaleFuncImprovedCollinearApprox = cms.string('1'),                           
    verbosity = cms.untracked.int32(0)
)

#---------------------------------------------------------------------------------------
# produce tau-jet + tau-jet pairs with tau-jet selection criteria applied on first leg
#---------------------------------------------------------------------------------------

# require 1st tau candidate to be within geometric acceptance of Pixel + SiTracker detectors
selectedDiTauPairs1stTauEta21Individual = copy.deepcopy(allDiTauPairs)
selectedDiTauPairs1stTauEta21Individual.srcLeg1 = cms.InputTag('selectedLayer1TausEta21Individual')

selectedDiTauPairs1stTauEta21Cumulative = copy.deepcopy(selectedDiTauPairs1stTauEta21Individual)
selectedDiTauPairs1stTauEta21Cumulative.srcLeg1 = cms.InputTag('selectedLayer1TausForDiTauEta21Cumulative')

# require 1st tau candidate to have transverse energy above threshold
selectedDiTauPairs1stTauPt20Individual = copy.deepcopy(allDiTauPairs)
selectedDiTauPairs1stTauPt20Individual.srcLeg1 = cms.InputTag('selectedLayer1TausPt20Individual')

selectedDiTauPairs1stTauPt20Cumulative = copy.deepcopy(selectedDiTauPairs1stTauPt20Individual)
selectedDiTauPairs1stTauPt20Cumulative.srcLeg1 = cms.InputTag('selectedLayer1TausForDiTauPt20Cumulative')

# require 1st tau candidate to have a leading track
# (track of Pt > 1. GeV within matching cone of size dR = 0.2 around jet-axis)
selectedDiTauPairs1stTauLeadTrkIndividual = copy.deepcopy(allDiTauPairs)
selectedDiTauPairs1stTauLeadTrkIndividual.srcLeg1 = cms.InputTag('selectedLayer1TausLeadTrkIndividual')

selectedDiTauPairs1stTauLeadTrkCumulative = copy.deepcopy(selectedDiTauPairs1stTauLeadTrkIndividual)
selectedDiTauPairs1stTauLeadTrkCumulative.srcLeg1 = cms.InputTag('selectedLayer1TausForDiTauLeadTrkCumulative')

# require leading track of 1st tau candidate to have Pt > 5. GeV
selectedDiTauPairs1stTauLeadTrkPtIndividual = copy.deepcopy(allDiTauPairs)
selectedDiTauPairs1stTauLeadTrkPtIndividual.srcLeg1 = cms.InputTag('selectedLayer1TausLeadTrkPtIndividual')

selectedDiTauPairs1stTauLeadTrkPtCumulative = copy.deepcopy(selectedDiTauPairs1stTauLeadTrkPtIndividual)
selectedDiTauPairs1stTauLeadTrkPtCumulative.srcLeg1 = cms.InputTag('selectedLayer1TausForDiTauLeadTrkPtCumulative')

# require 1st tau candidate to have no tracks of Pt > 1. GeV
# in isolation cone of size dR = 0.8, surrounding signal cone of size dR = 5./Et
selectedDiTauPairs1stTauTrkIsoIndividual = copy.deepcopy(allDiTauPairs)
selectedDiTauPairs1stTauTrkIsoIndividual.srcLeg1 = cms.InputTag('selectedLayer1TausTrkIsoIndividual')

selectedDiTauPairs1stTauTrkIsoCumulative = copy.deepcopy(selectedDiTauPairs1stTauTrkIsoIndividual)
selectedDiTauPairs1stTauTrkIsoCumulative.srcLeg1 = cms.InputTag('selectedLayer1TausForDiTauTrkIsoCumulative')

# require 1st tau candidate to be isolated
# with respect to energy deposits in ECAL
selectedDiTauPairs1stTauEcalIsoIndividual = copy.deepcopy(allDiTauPairs)
selectedDiTauPairs1stTauEcalIsoIndividual.srcLeg1 = cms.InputTag('selectedLayer1TausEcalIsoIndividual')

selectedDiTauPairs1stTauEcalIsoCumulative = copy.deepcopy(selectedDiTauPairs1stTauEcalIsoIndividual)
selectedDiTauPairs1stTauEcalIsoCumulative.srcLeg1 = cms.InputTag('selectedLayer1TausForDiTauEcalIsoCumulative')

# require 1st tau candidate to have either one or three tracks within signal cone
selectedDiTauPairs1stTauProngIndividual = copy.deepcopy(allDiTauPairs)
selectedDiTauPairs1stTauProngIndividual.srcLeg1 = cms.InputTag('selectedLayer1TausProngIndividual')

selectedDiTauPairs1stTauProngCumulative = copy.deepcopy(selectedDiTauPairs1stTauProngIndividual)
selectedDiTauPairs1stTauProngCumulative.srcLeg1 = cms.InputTag('selectedLayer1TausForDiTauProngCumulative')

# require 1st tau candidate to have charge either +1 or -1
# (computed as sum of charges of tracks within signal cone)
selectedDiTauPairs1stTauChargeIndividual = copy.deepcopy(allDiTauPairs)
selectedDiTauPairs1stTauChargeIndividual.srcLeg1 = cms.InputTag('selectedLayer1TausChargeIndividual')

selectedDiTauPairs1stTauChargeCumulative = copy.deepcopy(selectedDiTauPairs1stTauChargeIndividual)
selectedDiTauPairs1stTauChargeCumulative.srcLeg1 = cms.InputTag('selectedLayer1TausForDiTauChargeCumulative')

#----------------------------------------------------------------------------------------
# produce tau-jet + tau-jet pairs with tau-jet selection criteria applied on second leg
#----------------------------------------------------------------------------------------

# require 2nd tau candidate to be within geometric acceptance of Pixel + SiTracker detectors
selectedDiTauPairs2ndTauEta21Individual = copy.deepcopy(selectedDiTauPairs1stTauChargeCumulative)
selectedDiTauPairs2ndTauEta21Individual.srcLeg2 = cms.InputTag('selectedLayer1TausEta21Individual')

selectedDiTauPairs2ndTauEta21Cumulative = copy.deepcopy(selectedDiTauPairs2ndTauEta21Individual)
selectedDiTauPairs2ndTauEta21Cumulative.srcLeg2 = cms.InputTag('selectedLayer1TausForDiTauEta21Cumulative')

# require 2nd tau candidate to have transverse energy above threshold
selectedDiTauPairs2ndTauPt20Individual = copy.deepcopy(selectedDiTauPairs1stTauChargeCumulative)
selectedDiTauPairs2ndTauPt20Individual.srcLeg2 = cms.InputTag('selectedLayer1TausPt20Individual')

selectedDiTauPairs2ndTauPt20Cumulative = copy.deepcopy(selectedDiTauPairs1stTauPt20Individual)
selectedDiTauPairs2ndTauPt20Cumulative.srcLeg2 = cms.InputTag('selectedLayer1TausForDiTauPt20Cumulative')

# require 2nd tau candidate to have a leading track
# (track of Pt > 1. GeV within matching cone of size dR = 0.2 around jet-axis)
selectedDiTauPairs2ndTauLeadTrkIndividual = copy.deepcopy(selectedDiTauPairs1stTauChargeCumulative)
selectedDiTauPairs2ndTauLeadTrkIndividual.srcLeg2 = cms.InputTag('selectedLayer1TausLeadTrkIndividual')

selectedDiTauPairs2ndTauLeadTrkCumulative = copy.deepcopy(selectedDiTauPairs1stTauLeadTrkIndividual)
selectedDiTauPairs2ndTauLeadTrkCumulative.srcLeg2 = cms.InputTag('selectedLayer1TausForDiTauLeadTrkCumulative')

# require leading track of 2nd tau candidate to have Pt > 5. GeV
selectedDiTauPairs2ndTauLeadTrkPtIndividual = copy.deepcopy(selectedDiTauPairs1stTauChargeCumulative)
selectedDiTauPairs2ndTauLeadTrkPtIndividual.srcLeg2 = cms.InputTag('selectedLayer1TausLeadTrkPtIndividual')

selectedDiTauPairs2ndTauLeadTrkPtCumulative = copy.deepcopy(selectedDiTauPairs1stTauLeadTrkPtIndividual)
selectedDiTauPairs2ndTauLeadTrkPtCumulative.srcLeg2 = cms.InputTag('selectedLayer1TausForDiTauLeadTrkPtCumulative')

# require 2nd tau candidate to have no tracks of Pt > 1. GeV
# in isolation cone of size dR = 0.8, surrounding signal cone of size dR = 5./Et
selectedDiTauPairs2ndTauTrkIsoIndividual = copy.deepcopy(selectedDiTauPairs1stTauChargeCumulative)
selectedDiTauPairs2ndTauTrkIsoIndividual.srcLeg2 = cms.InputTag('selectedLayer1TausTrkIsoIndividual')

selectedDiTauPairs2ndTauTrkIsoCumulative = copy.deepcopy(selectedDiTauPairs1stTauTrkIsoIndividual)
selectedDiTauPairs2ndTauTrkIsoCumulative.srcLeg2 = cms.InputTag('selectedLayer1TausForDiTauTrkIsoCumulative')

# require 2nd tau candidate to be isolated
# with respect to energy deposits in ECAL
selectedDiTauPairs2ndTauEcalIsoIndividual = copy.deepcopy(selectedDiTauPairs1stTauChargeCumulative)
selectedDiTauPairs2ndTauEcalIsoIndividual.srcLeg2 = cms.InputTag('selectedLayer1TausEcalIsoIndividual')

selectedDiTauPairs2ndTauEcalIsoCumulative = copy.deepcopy(selectedDiTauPairs1stTauEcalIsoIndividual)
selectedDiTauPairs2ndTauEcalIsoCumulative.srcLeg2 = cms.InputTag('selectedLayer1TausForDiTauEcalIsoCumulative')

# require 2nd tau candidate to have either one or three tracks within signal cone
selectedDiTauPairs2ndTauProngIndividual = copy.deepcopy(selectedDiTauPairs1stTauChargeCumulative)
selectedDiTauPairs2ndTauProngIndividual.srcLeg2 = cms.InputTag('selectedLayer1TausProngIndividual')

selectedDiTauPairs2ndTauProngCumulative = copy.deepcopy(selectedDiTauPairs1stTauProngIndividual)
selectedDiTauPairs2ndTauProngCumulative.srcLeg2 = cms.InputTag('selectedLayer1TausForDiTauProngCumulative')

# require 1st tau candidate to have charge either +1 or -1
# (computed as sum of charges of tracks within signal cone)
selectedDiTauPairs2ndTauChargeIndividual = copy.deepcopy(selectedDiTauPairs1stTauChargeCumulative)
selectedDiTauPairs2ndTauChargeIndividual.srcLeg2 = cms.InputTag('selectedLayer1TausChargeIndividual')

selectedDiTauPairs2ndTauChargeCumulative = copy.deepcopy(selectedDiTauPairs1stTauChargeIndividual)
selectedDiTauPairs2ndTauChargeCumulative.srcLeg2 = cms.InputTag('selectedLayer1TausForDiTauChargeCumulative')

produceDiTauPairs = cms.Sequence( allDiTauPairs
                                 *selectedDiTauPairs1stTauEta21Individual * selectedDiTauPairs1stTauEta21Cumulative
                                 *selectedDiTauPairs1stTauPt20Individual * selectedDiTauPairs1stTauPt20Cumulative
                                 *selectedDiTauPairs1stTauLeadTrkIndividual * selectedDiTauPairs1stTauLeadTrkCumulative
                                 *selectedDiTauPairs1stTauLeadTrkPtIndividual * selectedDiTauPairs1stTauLeadTrkPtCumulative
                                 *selectedDiTauPairs1stTauTrkIsoIndividual * selectedDiTauPairs1stTauTrkIsoCumulative
                                 *selectedDiTauPairs1stTauEcalIsoIndividual * selectedDiTauPairs1stTauEcalIsoCumulative
                                 *selectedDiTauPairs1stTauProngIndividual * selectedDiTauPairs1stTauProngCumulative
                                 *selectedDiTauPairs1stTauChargeIndividual * selectedDiTauPairs1stTauChargeCumulative 
                                 *selectedDiTauPairs2ndTauEta21Individual * selectedDiTauPairs2ndTauEta21Cumulative
                                 *selectedDiTauPairs2ndTauPt20Individual * selectedDiTauPairs2ndTauPt20Cumulative
                                 *selectedDiTauPairs2ndTauLeadTrkIndividual * selectedDiTauPairs2ndTauLeadTrkCumulative
                                 *selectedDiTauPairs2ndTauLeadTrkPtIndividual * selectedDiTauPairs2ndTauLeadTrkPtCumulative
                                 *selectedDiTauPairs2ndTauTrkIsoIndividual * selectedDiTauPairs2ndTauTrkIsoCumulative
                                 *selectedDiTauPairs2ndTauEcalIsoIndividual * selectedDiTauPairs2ndTauEcalIsoCumulative
                                 *selectedDiTauPairs2ndTauProngIndividual * selectedDiTauPairs2ndTauProngCumulative
                                 *selectedDiTauPairs2ndTauChargeIndividual * selectedDiTauPairs2ndTauChargeCumulative )
