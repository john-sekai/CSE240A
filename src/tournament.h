//========================================================//
//  tournament.h                                          //
//  predictor definitions for tournament predictor        //
//                                                        //
//  Includes function prototypes and global predictor     //
//  variables and defines                                 //
//========================================================//
#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "predictor.h"
#include "string.h"

#define COUNTER_WIDTH 2
#define THRESHOLD (1 << (COUNTER_WIDTH - 1))
#define CHAR_BIT 8

extern uint32_t* tlhistory;  // tournament local history table
extern uint8_t* tlpred;  // tournament local predictor (counter)
extern uint8_t* tgpred;  // global history predictor
extern uint8_t* tchooser;  // tournament predictor chooser
extern uint32_t tghistory;  // previous path history


void 
set_history(uint32_t* history, size_t index, uint8_t outcome, size_t w) {
    uint32_t new_hist = history[index] << 1;
    if (outcome == 1) {
        new_hist += 1;
    }
    history[index] = (new_hist & ((1 << w) - 1));
}

uint32_t 
get_lower_pc(uint32_t pc, uint32_t bits) {
    // given a program counter, return the lowest bits bits of the address
    return pc & ((1 << bits) - 1);
}

uint8_t 
thresholding(uint8_t take) {
    if (take >= WT) {
        return TAKEN;
    }
    return NOTTAKEN;
}

void 
update_counter(char* counter, size_t index, uint8_t pred, uint8_t outcome) {
    if (outcome == TAKEN && counter[index] != ST) {
        counter[index] += 1;
    } else if (outcome == NOTTAKEN && counter[index] != SN) {
        counter[index] -= 1;
    }
}

void 
init_tournament() {
    if (bpType != TOURNAMENT) return;

    // initialize local history table
    int lhistory_entry = 1 << pcIndexBits;
    tlhistory = (uint32_t*) malloc(lhistory_entry * sizeof(uint32_t));
    memset(tlhistory, NOTTAKEN, lhistory_entry * sizeof(uint32_t));

    // local prediction 2-bit counter
    int lpred_entry = 1 << lhistoryBits;
    tlpred = (uint8_t*) malloc(lpred_entry);
    memset(tlpred, WN, lpred_entry);
    
    // global history 2-bit counter
    int ghistory_entry = 1 << ghistoryBits;
    tgpred = (uint8_t*) malloc(ghistory_entry);
    memset(tgpred, WN, ghistory_entry);

    // local-global 2-bit chooser
    int chooser_entry = 1 << ghistoryBits;
    tchooser = (uint8_t*) malloc(chooser_entry);
    memset(tchooser, WG, chooser_entry);

    // the global history
    tghistory = 0;
}

uint32_t
get_tchooser_count() {
    return *(tchooser + tghistory);
}

uint8_t
tournament_global_predict() {
    size_t take = tgpred[tghistory];
    return thresholding(take);
}

uint8_t 
tournament_local_predict(uint32_t pc) {
    uint32_t pc_index = get_lower_pc(pc, pcIndexBits);
    uint32_t index = tlhistory[pc_index];
    uint8_t take = tlpred[index];
    return thresholding(take);
}

uint8_t 
tournament_make_prediction(uint32_t pc) {
    uint32_t choose_g = get_tchooser_count();
    // count = g_correct - l_correct
    if (choose_g >= THRESHOLD) {
        // choose g
        return tournament_global_predict(pc);
    } else {
        // choose l
        return tournament_local_predict(pc);
    }
}

void
tournament_train(uint32_t pc, uint8_t outcome) {
    // train global predictor
    uint8_t gpred = tournament_global_predict(pc);
    update_counter(tgpred, tghistory, gpred, outcome);

    // train local predictor
    uint32_t pc_index = get_lower_pc(pc, pcIndexBits);
    uint32_t index = tlhistory[pc_index];
    uint8_t lpred = thresholding(tlpred[index]);
    update_counter(tlpred, index, lpred, outcome);
    set_history(tlhistory, pc_index, outcome, lhistoryBits);

    // update chooser
    if ((gpred == outcome && lpred != outcome) &&
        (get_tchooser_count() != ((1 << COUNTER_WIDTH) - 1))) {
        ++tchooser[tghistory];
    } else if (gpred != outcome && (lpred == outcome) &&
        (get_tchooser_count() != 0)) {
        --tchooser[tghistory];
    }

    // finally, update path history
    set_history(&tghistory, 0, outcome, ghistoryBits);
}

#endif