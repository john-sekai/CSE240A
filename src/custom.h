//========================================================//
//  custom.h                                              //
//  predictor definitions for customized predictor        //
//  Currently it shares the same data structure with      //
//  the tournament predictor.                             //
//                                                        //
//  Includes function prototypes and global predictor     //
//  variables and defines                                 //
//========================================================//
#ifndef CUSTOM_H
#define CUSTOM_H

#include "predictor.h"
#include "utils.h"
#include "string.h"

#define COUNTER_WIDTH 2
#define THRESHOLD (1 << (COUNTER_WIDTH - 1))
#define CHAR_BIT 8


extern uint32_t* clhistory;  // tournament local history table
extern uint8_t* clpred;  // tournament local predictor (counter)
extern uint8_t* cgpred;  // global history predictor
extern uint8_t* cchooser;  // tournament predictor chooser
extern uint32_t cghistory;  // previous path history



void 
init_custom() {
    if (bpType != CUSTOM) return;

    // initially, same prediction as tournament:9:10:10
    pcIndexBits = 10;
    lhistoryBits = 10;
    ghistoryBits = 9;

    // initialize local history table
    int chistory_entry = 1 << pcIndexBits;
    clhistory = (uint32_t*) malloc(chistory_entry * sizeof(uint32_t));
    memset(clhistory, NOTTAKEN, chistory_entry * sizeof(uint32_t));

    // local prediction 2-bit counter
    int cpred_entry = 1 << lhistoryBits;
    clpred = (uint8_t*) malloc(cpred_entry);
    memset(clpred, WN, cpred_entry);
    
    // global history 2-bit counter
    int ghistory_entry = 1 << ghistoryBits;
    cgpred = (uint8_t*) malloc(ghistory_entry);
    memset(cgpred, WN, ghistory_entry);

    // local-global 2-bit chooser
    int chooser_entry = 1 << ghistoryBits;
    cchooser = (uint8_t*) malloc(chooser_entry);
    memset(cchooser, WG, chooser_entry);

    // the global history
    cghistory = 0;
}

uint32_t gshare_index(uint32_t pc, uint32_t hist) {
    uint32_t mask = (1 << ghistoryBits) - 1;
    return (hist & mask) ^ (pc & mask);
}

uint32_t
get_cchooser_count(uint32_t pc) {
    return *(cchooser + gshare_index(pc, cghistory));
}

uint8_t
custom_global_predict(uint32_t pc) {
    size_t take = cgpred[gshare_index(pc, cghistory)];
    return thresholding(take);
}

uint8_t 
custom_local_predict(uint32_t pc) {
    uint32_t pc_index = get_lower_pc(pc, pcIndexBits);
    uint32_t index = clhistory[pc_index];
    uint8_t take = clpred[index];
    return thresholding(take);
}

uint8_t 
custom_make_prediction(uint32_t pc) {
    uint32_t choose_g = get_cchooser_count(pc);
    // count = g_correct - l_correct
    if (choose_g >= THRESHOLD) {
        // choose g
        return custom_global_predict(pc);
    } else {
        // choose l
        return custom_local_predict(pc);
    }
}

void
custom_train(uint32_t pc, uint8_t outcome) {
    // train global predictor
    uint8_t gpred = custom_global_predict(pc);
    // ===== gshare mod =====
    uint32_t gindex = gshare_index(pc, cghistory);
    update_counter(cgpred, gindex, gpred, outcome);
    // ======================

    // train local predictor
    uint32_t pc_index = get_lower_pc(pc, pcIndexBits);
    uint32_t index = clhistory[pc_index];
    uint8_t lpred = thresholding(clpred[index]);
    update_counter(clpred, index, lpred, outcome);
    set_history(clhistory, pc_index, outcome, lhistoryBits);

    // update chooser
    if ((gpred == outcome && lpred != outcome) &&
        (get_cchooser_count(pc) != ((1 << COUNTER_WIDTH) - 1))) {
        ++cchooser[gindex];
    } else if (gpred != outcome && (lpred == outcome) &&
        (get_cchooser_count(pc) != 0)) {
        --cchooser[gindex];
    }

    // finally, update path history
    set_history(&cghistory, 0, outcome, ghistoryBits);
}

#endif