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
-   **Workflow (STRICT):**
    1.  **Log:** Document the attempt and hypothesis.
    2.  **Run Test:** Execute `headless_checkers` with the relevant test file.
    3.  **Analyze:** Examine the output log in detail.
    4.  **Investigate:** Feed the analysis to `codebase_investigator` to determine the solution.
    5.  **Plan:** Create a `write_todos` list based on the investigator's solution.
    6.  **Execute:** Fix items one at a time, checking results after each.
    7.  **Document:** Log the result in `corrections_history.txt`.

## Project Status

The project is currently debugging the **Endgame Database (EGDB) WLD Driver**.

*   **Build Status**: The application builds successfully.
*   **Logging System**: Operational.
*   **EGDB Status (VERIFIED)**:
    *   **Initialization**: WLD and MTC drivers load successfully.
    *   **WLD**: Functional. Correctly identifies Wins and Losses (verified with 3-king test and 4-piece late game). The "Draw" result for the 4-piece start position is consistent with database content.
    *   **MTC**: Functional. Correctly returns distance-to-win/loss (verified 107/108 plies for 3-king test).
    *   **Orientation**: Confirmed Bit 0 = Square 1 (Top). Standard Kingsrow indexing.
    *   **Logic**: Reference `position_to_index_slice` is used. Manual bitboard reversals removed.

### Current Objectives

*   **Finalize GUI**: Connect EGDB stats to the UI.
*   **Testing**: Continue regression testing with diverse positions.
*   **Code Quality Optimization**: 
    *   Standardize constants using `constexpr` and `enum class` for type safety.
    *   Optimize AI evaluation by flattening PSTs and singletonizing Zobrist keys.
    *   Enhance application lifecycle by implementing proper singleton cleanup for the Logger.
    *   Improve UI responsiveness with dynamic board resizing.

### Recent Progress:
*   **2025-12-21**: Removed manual bitboard reversal. Confirmed orientation. Switched to reference indexing function.
