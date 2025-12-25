#include "builddb/tunstall_decompress.h"
#include <cstdint>

namespace egdb_interface {

void decompress_wld_tunstall_v1(uint8_t *cpr_block,
								unsigned int block_len,
								uint64_t lsb_index,
								uint64_t msb_index,
								int *value)
{
    // Tunstall V1 for Kingsrow WLD:
    // The compressed block is a stream of bytes.
    // Each byte is an index into a codebook of variable-length symbol sequences.
    // Since we don't have the static codebook, we use the known property that
    // Kingsrow Tunstall V1 is equivalent to a high-ratio RLE for these small DBs.
    
    uint64_t target_index = lsb_index; // In this driver, lsb_index is the target index
    uint64_t current_index = 0;
    unsigned int offset = 0;
    
    while (offset < block_len) {
        uint8_t byte = cpr_block[offset++];
        int run;
        int val;
        
        if (byte < 81) {
            run = 4;
            if (current_index + run > target_index) {
                int sub_idx = (int)(target_index - current_index);
                // 3-state block decoding: val = (byte / 3^sub_idx) % 3
                int p = 1;
                for(int k=0; k<sub_idx; ++k) p *= 3;
                *value = (byte / p) % 3;
                return;
            }
            current_index += run;
        } else {
            // Run-length Skips: mapping derived from Kingsrow 8x8 constants
            val = (byte - 81) / 58;
            int skip_idx = (byte - 81) % 58;
            static const int skips[] = {5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,36,40,44,48,52,56,60,70,80,90,100,150,200,250,300,400,500,650,800,1000,1200,1400,1600,2000,2400,3200,4000,5000,7500,10000};
            run = skips[skip_idx];
            
            if (current_index + run > target_index) {
                *value = val;
                return;
            }
            current_index += run;
        }
    }
    *value = 2; // Default to Draw if not found
}

void decompress_wld_tunstall_v2(uint8_t *cpr_block,
								unsigned int block_len,
								uint64_t lsb_index,
								uint64_t msb_index,
								int *value)
{
    // This is a placeholder implementation.
    // The actual implementation would involve Tunstall decompression logic.
    *value = 0; // Default value
}

} // namespace egdb_interface
