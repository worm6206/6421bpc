#include "predictor.h"

#define PHT_CTR_MAX  7
#define PHT_CTR_INIT 3

#define META_CTR_MAX  7
#define META_CTR_INIT 3

#define HIST_LEN   14

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////
/*
Pshare:
  PHT: 14*2^14
  BHT: 8*2^14
Gshare:
  PHT: 8*2^14
  GHR: 14
Meta-predictor: 8*2^14

Total size: 36*2^14 + 14  = 589838 bits  = 58.9 Kbyte
 */
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
    Meta[ii]=META_CTR_INIT; 
  }
  
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool   PREDICTOR::GetPrediction(UINT32 PC){

  UINT32 PCbits = (PC) % (numPhtEntries);
  UINT32 temp = pht_P[PCbits];
  UINT32 PcPhtXor = (PC^temp) % (numPhtEntries);
  UINT32 bhtCounter = bht[PcPhtXor]; // prediction from PShare

  UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
  UINT32 phtCounter = pht_G[phtIndex]; // prediction from GShare
  
  UINT32 MetaCounter = Meta[PCbits]; // choice from Meta-predictor

  if(MetaCounter > META_CTR_MAX/2){  //PShare
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

  UINT32 PCbits = (PC) % (numPhtEntries);
  UINT32 temp = pht_P[PCbits];
  UINT32 PcPhtXor = (PC^temp) % (numPhtEntries);
  UINT32 bhtCounter = bht[PcPhtXor]; // prediction from PShare

  UINT32 phtIndex   = (PC^ghr) % (numPhtEntries);
  UINT32 phtCounter = pht_G[phtIndex]; // prediction from GShare
  
  UINT32 MetaCounter = Meta[PCbits]; // choice from Meta-predictor

  bool PShare, GShare;

  if(bhtCounter > PHT_CTR_MAX/2){
    PShare = TAKEN;
  }else{
    PShare = NOT_TAKEN; 
  }

  if(phtCounter > PHT_CTR_MAX/2){
    GShare = TAKEN; 
  }else{
    GShare = NOT_TAKEN; 
  }

  // Update
  if(resolveDir == TAKEN){
    bht[PcPhtXor] = SatIncrement(bhtCounter, PHT_CTR_MAX);
    pht_G[phtIndex] = SatIncrement(phtCounter, PHT_CTR_MAX);
  }else{
    bht[PcPhtXor] = SatDecrement(bhtCounter);
    pht_G[phtIndex] = SatDecrement(phtCounter);
  }
  pht_P[PcPhtXor] = (pht_P[PcPhtXor] << 1);
  ghr = (ghr << 1);
  if(resolveDir == TAKEN){
    pht_P[PcPhtXor]++; 
    ghr++; 
  }

  //Update Meta-predictor
  if(PShare == resolveDir && GShare != resolveDir){
    Meta[PCbits] = SatIncrement(MetaCounter, META_CTR_MAX);
  }else if(PShare != resolveDir && GShare == resolveDir){
    Meta[PCbits] = SatDecrement(MetaCounter);
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
