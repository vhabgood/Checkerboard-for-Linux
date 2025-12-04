#include "core_types.h"

// Definition of the global program status word
#ifdef __cplusplus
extern "C" {
#endif
uint32_t g_programStatusWord = 0;
#ifdef __cplusplus
}
#endif

// Getter for g_programStatusWord
uint32_t getProgramStatusWord() {
    return g_programStatusWord;
}

// Setter for g_programStatusWord
void setProgramStatusWord(uint32_t value) {
    g_programStatusWord = value;
}

// Updater for g_programStatusWord (bitwise OR)
void updateProgramStatusWord(uint32_t flags_to_set) {
    g_programStatusWord |= flags_to_set;
}

// Clearer for g_programStatusWord (bitwise AND NOT)
void clearProgramStatusWordFlags(uint32_t flags_to_clear) {
    g_programStatusWord &= ~flags_to_clear;
}
