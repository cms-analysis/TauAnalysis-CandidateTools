import FWCore.ParameterSet.Config as cms

#--------------------------------------------------------------------------------
# define sources of systematic uncertainties
# considered in Z --> tau+ tau- and W --> tau nu cross-section measurements
# (Note that the same values for systematic uncertainties are used for all channels)
#--------------------------------------------------------------------------------

electronSystematics = {
    "sysElectronEnUp"       : cms.InputTag('smearedElectronsEnScaleUp'),
    "sysElectronEnDown"     : cms.InputTag('smearedElectronsEnScaleDown')
}

muonSystematics = {
    "sysMuonPtUp"           : cms.InputTag('patMuonsMuScleFitCorrectedMomentumShiftUp'),
    "sysMuonPtDown"         : cms.InputTag('patMuonsMuScleFitCorrectedMomentumShiftDown')
}

tauSystematics = {   
    "sysTauJetEnUp"         : cms.InputTag('patTausJECshiftUp'),
    "sysTauJetEnDown"       : cms.InputTag('patTausJECshiftDown')
}

muTauPairSystematics = {
    "sysMuonPtUp"           : cms.InputTag('allMuTauPairsSysMuonPtUp'),
    "sysMuonPtDown"         : cms.InputTag('allMuTauPairsSysMuonPtDown'),
    "sysTauJetEnUp"         : cms.InputTag('allMuTauPairsSysTauJetEnUp'),
    "sysTauJetEnDown"       : cms.InputTag('allMuTauPairsSysTauJetEnDown'),
    "sysJetEnUp"            : cms.InputTag('allMuTauPairsSysJetEnUp'),
    "sysJetEnDown"          : cms.InputTag('allMuTauPairsSysJetEnDown')
}

metSystematicsForZtoMuTau = {
    "sysMuonPtUp"           : cms.InputTag('patMETsSysMuonPtUp'),
    "sysMuonPtDown"         : cms.InputTag('patMETsSysMuonPtDown'),
    "sysTauJetEnUp"         : cms.InputTag('patMETsSysTauJetEnUp'),
    "sysTauJetEnDown"       : cms.InputTag('patMETsSysTauJetEnDown')
}

metSystematicsForAHtoMuTau = {
    "sysMuonPtUp"           : cms.InputTag('patMETsSysMuonPtUp'),
    "sysMuonPtDown"         : cms.InputTag('patMETsSysMuonPtDown'),
    "sysTauJetEnUp"         : cms.InputTag('patMETsSysTauJetEnUp'),
    "sysTauJetEnDown"       : cms.InputTag('patMETsSysTauJetEnDown'),
    "sysJetEnUp"            : cms.InputTag('patMETsSysJetEnUp'),
    "sysJetEnDown"          : cms.InputTag('patMETsSysJetEnDown')
}

jetSystematics = {   
    "sysJetEnUp"            : cms.InputTag('patJetsJECshiftUp'),
    "sysJetEnDown"          : cms.InputTag('patJetsJECshiftDown')
}

theorySystematics = {
    # CV: do not run theory systematics for now,
    #     as in particular the module for estimating PDF uncertainties
    #     significantly increases the overall run-time/memory consumption of the cmsRun job !!
    #"sysPdfWeights(41)"     : cms.InputTag('pdfWeights:cteq65'),
    #"sysIsrWeight"          : cms.InputTag('isrWeight'),
    #"sysFsrWeight"          : cms.InputTag('fsrWeight')
}

def getSysUncertaintyNames(sysUncertaintyMaps):
    
    sysUncertaintyNames = []
    
    for sysUncertaintyMap in sysUncertaintyMaps:
        for sysUncertaintyName in sysUncertaintyMap.keys():
            sysUncertaintyNames.append(sysUncertaintyName)

    return sysUncertaintyNames

def getSysUncertaintyParameterSets(sysUncertaintyMaps):
    
    sysUncertaintyParameterSets = []
    
    for sysUncertaintyMap in sysUncertaintyMaps:
        for sysUncertaintyName, sysUncertaintyInputTag in sysUncertaintyMap.items():
            sysUncertaintyParameterSet = cms.PSet()

            setattr(sysUncertaintyParameterSet, "name", cms.string(sysUncertaintyName))
            setattr(sysUncertaintyParameterSet, "src", sysUncertaintyInputTag)

            sysUncertaintyParameterSets.append(sysUncertaintyParameterSet)

    return cms.VPSet(sysUncertaintyParameterSets)
