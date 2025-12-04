# GEMINI Project Analysis: Checkers Engine

## Project Overview

This project is focused on creating a standalone, cross-platform C/C++ GUI application that replicates the look, feel, and behavior of the original Windows Checkerboard. The goal is to refactor the existing C/C++ codebase within `ResourceFiles/` to remove Windows-specific dependencies and replace them with POSIX-compliant or standard C/C++ alternatives, making the entire application, including its GUI and integrated endgame database (EGDB) functionality, cross-platform and Buildable on Linux.

### Architecture

The architecture is a unified C/C++ application, handling both the low-level, performance-critical engine logic and the higher-level game management and graphical user interface. The `ResourceFiles` directory is the effective project root.

A key ongoing task is the porting of the C engine. This involves:
- Replacing Windows API calls with POSIX equivalents.
- Consolidating scattered and conflicting C header files into a single, unified `checkers_types.h`.
- Removing all Windows-specific GUI code from the C sources.

## Building and Running

The C/C++ application must be compiled first. The build process is managed by a shell script.

-   **Build Command:**

    ```bash
    ./build_checkerboard.sh
    ```

-   **Output:** The script compiles all the C/C++ source files located in `ResourceFiles/` and links them into a single executable named `checkerboard_app` in the same directory.

## Development Conventions

-   **Unified Header:** All C/C++ code uses a single header file, `ResourceFiles/checkers_types.h`, for all common types and definitions to resolve compilation issues.
-   **Build Script:** `build_checkerboard.sh` is the single source of truth for building the application.
-   **File Archiving:** Obsolete files are moved to the `/OLD/` directory.
-   **History:** After every significant action, `ResourceFiles/corrections_history.txt` is updated with a description of the fix, and `ResourceFiles/compilation_status.txt` is updated with the current compilation status.

## Project Status

The project has achieved a major milestone by successfully integrating the Endgame Database (EGDB) and resolving critical bugs.

*   **Ported Core Application**: The application has been successfully ported from its Windows-specific origins to a cross-platform C++/Qt framework.
*   **Working Build System**: A reliable build system (`build_checkerboard.sh`) is in place.
*   **Functional GUI and AI**: The application features a functional GUI, a working checkerboard, and an integrated AI that can complete a game.
*   **Comprehensive Status Word**: A 32-bit Program Status Word (PSW) effectively tracks program execution and critical events, aiding in diagnostics.
*   **EGDB is ONLINE and STABLE**: All critical EGDB issues have been resolved. This includes:
    *   The primary initialization and parsing bug (a one-character string comparison error in the index file parser).
    *   The "decode_value forward search failed - blocknumber out of bounds" error. This was resolved by meticulously debugging the combinatorial indexing logic in `calculate_index` and `calculate_lsb_index`. The fix involved correcting the `occupied_mask` used during black king indexing to prevent overcounting of occupied squares, and refining the combinatorial bounds check from `x < 0` to `x < i - 1` within `calculate_lsb_index`.
    *   A "double free or corruption" crash on exit, which was fixed by implementing a shutdown guard in `db_exit()`.

The EGDB is now fully functional and stable, performing lookups and returning correct win/loss/draw values without errors.

### Current Focus:
With the EGDB fully integrated and stable, the immediate focus shifts to other tasks to enhance the application.

### Recent Progress:
*   **Database Refactoring (Templatization):**
    *   Successfully refactored all duplicated WLD (Win/Loss/Draw) and MTC (Mate-to-Capture) database functions (`parse_single_base_entry`, `parseindexfile`, `processEgdbFiles`, `get_disk_block`, `decode_value`, `dblookup`) into generic, templated functions (`*_generic<T>`). This significantly reduces code duplication and improves maintainability.
    *   Updated the build system (`build_checkerboard.sh`) to use C++17 standard (`-std=c++17`) to support `if constexpr` and `std::is_same::value` for robust templated programming.
*   **Program Status Word (PSW) Integration:**
    *   Integrated `updateProgramStatusWord()` and `clearProgramStatusWordFlags()` into the new generic database functions and `db_init`, replacing direct bitwise manipulation of `g_programStatusWord`. This provides a more encapsulated and safer way to manage critical application status.
*   **Decompression Scheme Verification:**
    *   Investigated and confirmed that the two decompression schemes used in the `DBManager` (Tunstall-like run-length encoding for values > 80 and direct table lookup for values <= 80) are correctly implemented and consistent with the original logic found in historical code (`OLD/dblookup.cpp`).
    *   Confirmed that the available EGDB files (`.idx` files in the `db/` directory) do not utilize a "haspartials" feature, meaning the current decompression logic is sufficient and no additional schemes are required at this time.