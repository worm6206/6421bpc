#include "predictor.h"


#define PHT_CTR_MAX  3
#define PHT_CTR_INIT 2

#define HIST_LEN   14

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
  // ghr              = 0;
  numPhtEntries    = (1<< HIST_LEN);

  pht = new UINT32[numPhtEntries];
  bht = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht[ii]=0; 
  }
  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    bht[ii]=PHT_CTR_INIT; 
  }
  
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool   PREDICTOR::GetPrediction(UINT32 PC){

  // UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
  // UINT32 phtCounter = pht[phtIndex];
  UINT32 pht_history = pht[(PC&HIST_LEN)];
  UINT32 PcPhtXor = (PC^pht_history) % (numPhtEntries);
  UINT32 bhtCounter = bht[PcPhtXor];
  
  if(bhtCounter > PHT_CTR_MAX/2){
    return TAKEN; 
  }else{
    return NOT_TAKEN; 
  }
  
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void  PREDICTOR::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){

  UINT32 pht_history = pht[(PC&HIST_LEN)];
  UINT32 PcPhtXor = (PC^pht_history) % (numPhtEntries);
  UINT32 bhtCounter = bht[PcPhtXor];

  // update the bht

  if(resolveDir == TAKEN){
    bht[PcPhtXor] = SatIncrement(bhtCounter, PHT_CTR_MAX);
  }else{
    bht[PcPhtXor] = SatDecrement(bhtCounter);
  }

  // update the pht

  pht[PcPhtXor] = (pht[PcPhtXor] << 1);

  if(resolveDir == TAKEN){
    pht[PcPhtXor]++; 
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
