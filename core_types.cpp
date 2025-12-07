#include "core_types.h"
#include <iostream> // For debugging, if needed

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

// Definitions for BitboardConstants
namespace BitboardConstants {
    char bitsinword[65536];
    uint32_t revword[65536];

    // Function to initialize the lookup tables
    static void initBitboardConstants() {
        static bool initialized = false;
        if (initialized) return;

        // Initialize bitsinword (population count for 16-bit values)
        for (int i = 0; i < 65536; i++) {
            bitsinword[i] = (char)recbitcount(i);
        }

        // Initialize revword (bit reversal for 16-bit values)
        for (int i = 0; i < 65536; i++) {
            uint32_t n = i;
            uint32_t reversed_n = 0;
            for (int j = 0; j < 16; j++) {
                if ((n >> j) & 1) {
                    reversed_n |= (1 << (15 - j));
                }
            }
            revword[i] = reversed_n;
        }
        initialized = true;
    }

    // Call initializer at startup
    struct InitHelper {
        InitHelper() {
            initBitboardConstants();
        }
    };
    static InitHelper helper; // This will construct the helper and call initBitboardConstants
} // namespace BitboardConstants
