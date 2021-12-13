//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include "tournament.h"
#include "custom.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

// ==========================
// tournament data structures
// ==========================
// tournament tables
uint32_t* tlhistory;  // tournament local history table
uint8_t* tlpred;  // tournament local predictor (counter)
uint8_t* tgpred;  // counter for global branch pattern
uint8_t* tchooser;
uint32_t tghistory;   // history stored in big-endian


// ==========================
// tournament data structures
// ==========================
// tournament tables
uint32_t* clhistory;  // tournament local history table
uint8_t* clpred;  // tournament local predictor (counter)
uint8_t* cgpred;  // counter for global branch pattern
uint8_t* cchooser;
uint32_t cghistory;   // history stored in big-endian

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  // tournament initialization
  init_tournament();
  
  init_custom();

}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
    case TOURNAMENT:
      return tournament_make_prediction(pc);
    case CUSTOM:
      return custom_make_prediction(pc); 
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType)
  {
  case TOURNAMENT:
    tournament_train(pc, outcome);
    break;
  case CUSTOM:
    custom_train(pc, outcome);
    break;
  default:
    break;
  }
}
