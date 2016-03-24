#include "predictor.h"


#define PHT_CTR_MAX  3
#define PHT_CTR_INIT 2

#define HIST_LEN   13

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////
// Total storage budget: 32KB + 17 bits
// Total PHT counters: 2^17 
// Total PHT size = 2^17 * 2 bits/counter = 2^18 bits = 32KB
// GHR size: 17 bits
// Total Size = PHT size + GHR size
/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

PREDICTOR::PREDICTOR(void){

  historyLength    = HIST_LEN;
  ghr              = 0; //GShare
  numPhtEntries    = (1<< HIST_LEN);

  pht_P = new UINT32[numPhtEntries]; //PShare
  bht = new UINT32[numPhtEntries]; //PShare
  pht_G = new UINT32[numPhtEntries]; //GShare
  Meta = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht_P[ii]=0; 
    bht[ii]=PHT_CTR_INIT; 
    pht_G[ii]=PHT_CTR_INIT; 
    Meta[ii]=PHT_CTR_INIT; 
  }
  
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool   PREDICTOR::GetPrediction(UINT32 PC){

  UINT32 pht_history = pht_P[(PC&HIST_LEN)];
  UINT32 PcPhtXor = (PC^pht_history) % (numPhtEntries);
  UINT32 bhtCounter = bht[PcPhtXor]; // prediction from PShare

  UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
  UINT32 phtCounter = pht_G[phtIndex]; // prediction from GShare
  
  UINT32 MetaCounter = Meta[(PC&HIST_LEN)]; // choice from Meta-predictor

  if(MetaCounter > PHT_CTR_MAX/2){  //PShare
    if(bhtCounter > PHT_CTR_MAX/2){
      return TAKEN; 
    }else{
      return NOT_TAKEN; 
    }
  }else{  //GShare
    if(phtCounter > PHT_CTR_MAX/2){
      return TAKEN; 
    }else{
      return NOT_TAKEN; 
    }
  }
  
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void  PREDICTOR::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){

  UINT32 pht_history = pht_P[(PC&HIST_LEN)];
  UINT32 PcPhtXor = (PC^pht_history) % (numPhtEntries);
  UINT32 bhtCounter = bht[PcPhtXor]; // prediction from PShare

  UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
  UINT32 phtCounter = pht_G[phtIndex]; // prediction from GShare
  
  UINT32 MetaCounter = Meta[(PC&HIST_LEN)]; // choice from Meta-predictor


  if(MetaCounter > PHT_CTR_MAX/2){
    // Update PShare
    // update the bht
    if(resolveDir == TAKEN){
      bht[PcPhtXor] = SatIncrement(bhtCounter, PHT_CTR_MAX);
    }else{
      bht[PcPhtXor] = SatDecrement(bhtCounter);
    }
    // update the pht
    pht_P[PcPhtXor] = (pht_P[PcPhtXor] << 1);
    if(resolveDir == TAKEN){
      pht_P[PcPhtXor]++; 
    }
  }else{
    //Update Gshare
    // update the PHT
    if(resolveDir == TAKEN){
      pht_G[phtIndex] = SatIncrement(phtCounter, PHT_CTR_MAX);
    }else{
      pht_G[phtIndex] = SatDecrement(phtCounter);
    }
    // update the GHR
    ghr = (ghr << 1);
    if(resolveDir == TAKEN){
      ghr++; 
    }
  }

  //Update Meta-predictor
  if(resolveDir == predDir){
    Meta[(PC&HIST_LEN)] = SatIncrement(MetaCounter, PHT_CTR_MAX);
  }else{
    Meta[(PC&HIST_LEN)] = SatDecrement(MetaCounter);
  }

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget){

  // This function is called for instructions which are not
  // conditional branches, just in case someone decides to design
  // a predictor that uses information from such instructions.
  // We expect most contestants to leave this function untouched.

  return;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
