# EGDB Lookup Trace for FEN: W:W19,20:B4,5

This document traces the logic of the `dblookup` function for the FEN string `W:W19,20:B4,5`.

## 1. FEN Parsing and Initial State

- **FEN:** `W:W19,20:B4,5`
- **Color to move:** White (`cl = CB_WHITE`, which is `1`)
- **White pieces:** Men on squares 19 and 20.
- **Black pieces:** Men on squares 4 and 5.
- **Piece counts:** 2 white men, 0 white kings, 2 black men, 0 black kings. Total = 4 pieces.

The `FENtobitboard_pos` function converts this to a `bitboard_pos` structure:
- `p.wm = (1 << 18) | (1 << 19)`
- `p.bm = (1 << 3) | (1 << 4)`
- `p.wk = 0`
- `p.bk = 0`
- `p.color = 1` (White)

## 2. `dblookup` initial analysis

The function `dblookup` starts by analyzing the position.

- `bm = 2`, `bk = 0`, `wm = 2`, `wk = 0`
- `bmrank` (rank of the most advanced black man) = `MSB(p.bm) / 4 = MSB((1<<3)|(1<<4)) / 4 = 4 / 4 = 1`.
- `wmrank` (rank of the most advanced white man) = `(31 - LSB(p.wm)) / 4 = (31 - LSB((1<<18)|(1<<19))) / 4 = (31 - 18) / 4 = 13 / 4 = 3`.

## 3. Position Reversal

The code checks if the position should be reversed for canonical representation. The condition is:
`(((wm+wk-bm-bk)<<16) + ((wk-bk)<<8) + ((wmrank-bmrank)<<4) + p.color) > 0`

- `wm+wk-bm-bk = 2 - 2 = 0`
- `wk-bk = 0 - 0 = 0`
- `wmrank-bmrank = 3 - 1 = 2`
- `p.color = 1` (White)

Calculation: `(0 << 16) + (0 << 8) + (2 << 4) + 1 = 0 + 0 + 32 + 1 = 33`.
Since `33 > 0`, the position is **reversed**.

### Reversed Position State:
- Original pieces are swapped and bit-reversed.
  - `revbitboard_pos.bm = revert(p.wm) = revert((1<<18)|(1<<19)) = (1<<13)|(1<<12)`
  - `revbitboard_pos.wm = revert(p.bm) = revert((1<<3)|(1<<4)) = (1<<28)|(1<<27)`
  - `revbitboard_pos.bk = 0`, `revbitboard_pos.wk = 0`
- Color is flipped: `p.color` becomes `CB_BLACK` (0).
- Piece counts are swapped: `bm` becomes 2, `wm` becomes 2.
- Ranks are swapped: `bmrank` becomes 3, `wmrank` becomes 1.

The lookup now proceeds with this reversed, canonical position.

## 4. `cprsubdatabase` Lookup

The code looks for a database entry corresponding to the canonical position:
- `dbpointer = &cprsubdatabase[bm=2][bk=0][wm=2][wk=0][bmrank=3][wmrank=1][color=0]`

Assuming this entry exists and points to a valid database file, the next step is calculating the index.

## 5. `calculate_index`

The `calculate_index` function is called with the **reversed** position `p` and the **swapped** piece counts and ranks.

- `p.bm = (1<<13)|(1<<12)`
- `p.wm = (1<<28)|(1<<27)`
- `bm=2`, `bk=0`, `wm=2`, `wk=0`
- `bmrank=3`, `wmrank=1`

### Index Calculation Steps:

1.  **`bmindex = calculate_lsb_index(p.bm, 0)`**
    - `pieces = (1<<13)|(1<<12)`
    - Loop 1: `x=12`, `index += bicoef[12][1] = 12`.
    - Loop 2: `x=13`, `index += bicoef[13][2] = 78`.
    - **`bmindex` (initial) = 12 + 78 = 90.**

2.  **`wmindex = calculate_lsb_index(revert(p.wm), revert(p.bm))`**
    - `revert(p.wm)` = `revert((1<<28)|(1<<27)) = (1<<3)|(1<<4)`.
    - `revert(p.bm)` = `revert((1<<13)|(1<<12)) = (1<<18)|(1<<19)`. This is the `occupied_mask`.
    - `pieces = (1<<3)|(1<<4)`.
    - Loop 1: `x=3`. `bit_count` before 3 in mask is 0. `x` remains 3. `index += bicoef[3][1] = 3`.
    - Loop 2: `x=4`. `bit_count` before 4 in mask is 0. `x` remains 4. `index += bicoef[4][2] = 6`.
    - **`wmindex` (initial) = 3 + 6 = 9.**

3.  **`bkindex` and `wkindex`** are both 0 as there are no kings.

4.  **Calculate Ranges:**
    - `bmrange = bicoef[4*(3+1)][2] - bicoef[4*3][2] = bicoef[16][2] - bicoef[12][2] = 120 - 66 = 54`.
    - `wmrange = bicoef[4*(1+1)][2] - bicoef[4*1][2] = bicoef[8][2] - bicoef[4][2] = 28 - 6 = 22`.
    - `bkrange = bicoef[32 - 2 - 2][0] = 1`.

5.  **Adjust Indices by Rank:**
    - `bmindex -= bicoef[4 * 3][2] = 90 - 66 = 24`.
    - `wmindex -= bicoef[4 * 1][2] = 9 - 6 = 3`.

6.  **Calculate Final Index:**
    `final_index = bmindex + wmindex * bmrange + bkindex * bmrange * wmrange + ...`
    `final_index = 24 + 3 * 54 + 0 + 0`
    `final_index = 24 + 162`
    **`final_index = 186`**

## 6. `decode_value`

The `dblookup` function will now use this final index of `186` to find the block containing this index, load the block from the `.cpr` file, and decompress the value.

- It performs a search over the `dbpointer->idx` array to find `blocknumber`.
- It calls `get_disk_block` to read the data from the file into the cache.
- It calls `decode_value` with the `diskblock` and the index `186` to extract the win/loss/draw value.

This trace confirms the index calculation for your FEN is **186**. If the EGDB is not being hit, the issue likely lies in one of three areas:
1.  The `cprsubdatabase` entry for this piece configuration is not being correctly initialized during `db_init`.
2.  The file I/O (`get_disk_block`) is failing to read the correct block from the database file.
3.  The `decode_value` function has a logic error that prevents it from correctly decompressing the value from the block data.

Please review this trace. We can add logging at these specific stages to pinpoint the failure.
