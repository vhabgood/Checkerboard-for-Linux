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

The project has achieved significant stability improvements, with a robust build system and thread-safe logging now in place. We have systematically addressed several critical crashes and memory corruption issues.

*   **Build Status**: The application builds successfully.
*   **Logging System**: A robust, thread-safe logging system is fully implemented and confirmed to be working correctly. **Log levels are now correctly filtered, and excessive debug messages no longer lead to massive log files.** Logs are concise and informative, now providing detailed AI move information for both players (score, depth, EGDB result). **The issue of literal `\n` characters appearing in the log output has been resolved by correctly handling newline characters in the `Logger` class, ensuring proper formatting.**
*   **AIWorker Stability**:
    *   The stack-based data corruption bug in `find_captures_recursive` has been addressed, resolving the `malloc(): smallbin double linked list corrupted` crash during autoplay.
    *   Proper `AIWorker` thread cleanup logic has been implemented in `MainWindow::~MainWindow()`, addressing previous hangs and heap corruption issues that occurred on application exit.
    *   `AppState`, `AI_State`, and `CBmove` custom types are now correctly registered with Qt's meta-type system, eliminating "Cannot queue arguments" errors and a major source of undefined behavior and potential memory corruption in inter-thread communication.
    *   **AI Search Efficiency & "Stuck" Behavior:** The AI search algorithm (`minimax` and `quiescenceSearch`) no longer gets "stuck" in infinite loops or takes excessively long. This was resolved by:
        *   **Fixing a critical typo** in `AIWorker::isSquareAttacked` that led to incorrect board evaluation.
        *   **Adding `m_abortRequested` checks** within `quiescenceSearch` to ensure it respects time limits and abort requests.
        *   **Correcting the transposition table usage** by ensuring it is cleared only once at the beginning of `AIWorker::searchBestMove`, rather than at each iterative deepening step.
        *   **Adding transposition table lookups and storage to `quiescenceSearch`** to prevent redundant computations during capture sequences.
    *   **Headless AI Execution & Threading:** The headless application (`headless_checkers`) now correctly executes AI moves for both White and Black players. The issue of the main `QEventLoop` blocking due to `AIWorker` running in the same thread has been resolved by:
        *   **Implementing a dedicated `QThread` for `AIWorker`** in `headless_main.cpp`, mirroring the GUI application's successful threading model.
        *   **Using a `MainController` `QObject`** to emit signals to the `AIWorker`'s thread for move requests.
        *   **Ensuring proper thread cleanup** for the `AIWorker`'s `QThread` before `main()` exits, preventing "Destroyed while thread is still running" errors.
*   **EGDB Status**: The EGDB initialization is now successfully enabled and functioning correctly. **The previous issue of "Parsed 0 WLD/MTC blocks" has been resolved, and EGDB lookups are being performed successfully, as confirmed by log analysis.** Furthermore, redundant `DBManager::db_init` calls have been eliminated in `headless_main.cpp`, ensuring EGDB is initialized only once.
*   **Program Status Word (PSW):**
    *   Integrated and refactored. `g_programStatusWord` is now managed via getter/setter/updater functions in `core_types.cpp` for better global variable management and stability. Enhanced PSW flags provide granular status for EGDB lookups.

### Current Problem: None

All previously identified major problems (AI getting stuck, excessive logging, EGDB not loading blocks, incorrect headless AI execution/threading, and log formatting) have been successfully resolved. The application is now stable, produces concise and informative logs, and the AI correctly utilizes the EGDB for lookups in both GUI and headless modes.

### Next Steps:
*   **Further EGDB Integration/Testing:** Continue with more extensive EGDB testing, potentially including different piece counts and scenarios, to ensure full coverage and correct behavior.
*   **AI Refinement:** Explore further AI improvements beyond basic search and evaluation.

### Recent Progress:
*   **Database Refactoring (Templatization):**
    *   Successfully refactored all duplicated WLD (Win/Loss/Draw) and MTC (Mate-to-Capture) database functions (`parse_single_base_entry`, `parseindexfile`, `processEgdbFiles`, `get_disk_block`, `decode_value`, `dblookup`) into generic, templated functions (`*_generic<T>`). This significantly reduces code duplication and improves maintainability.
    *   Updated the build system (`build_checkerboard.sh`) to use C++17 standard (`-std=c++17`) to support `if constexpr` and `std::is_same::value` for robust templated programming.
*   **Program Status Word (PSW) Integration:**
    *   Integrated `updateProgramStatusWord()` and `clearProgramStatusWordFlags()` into the new generic database functions and `db_init`, replacing direct bitwise manipulation of `g_programStatusWord`. This provides a more encapsulated and safer way to manage critical application status.
    *   **Enhanced PSW Flags**: Added granular PSW flags for various EGDB lookup states, including attempts, specific error conditions (out-of-bounds, not present, invalid index, disk read, decode errors), single-value hits, and explicit flags for Win, Loss, Draw, and Unknown results.
*   **Decompression Scheme Verification:**
    *   Investigated and confirmed that the two decompression schemes used in the `DBManager` (Tunstall-like run-length encoding for values > 80 and direct table lookup for values <= 80) are correctly implemented and consistent with the original logic found in historical code (`OLD/dblookup.cpp`).
    *   Confirmed that the available EGDB files (`.idx` files in the `db/` directory) do not utilize a "haspartials" feature, meaning the current decompression logic is sufficient and no additional schemes are required at this time.