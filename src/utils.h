#ifndef UTILS_H
#define UTILS_H

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
update_counter(uint8_t* counter, size_t index, uint8_t pred, uint8_t outcome) {
    if (outcome == TAKEN && counter[index] != ST) {
        counter[index] += 1;
    } else if (outcome == NOTTAKEN && counter[index] != SN) {
        counter[index] -= 1;
    }
}

#endif