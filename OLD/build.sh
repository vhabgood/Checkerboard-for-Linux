#!/bin/bash
set -x

# Define paths
PROJECT_ROOT="/home/victor/Desktop/checkers/Programs/checkers_project"
RESOURCE_FILES_DIR="${PROJECT_ROOT}/ResourceFiles"

# Set PKG_CONFIG_PATH for Qt5
export PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH

# Qt specific flags
QT_CFLAGS=$(pkg-config --cflags Qt5Widgets Qt5Gui Qt5Core Qt5Multimedia)
QT_LIBS=$(pkg-config --libs Qt5Widgets Qt5Gui Qt5Core Qt5Multimedia)

# C++ source files
CPP_SRCS=(
    "${RESOURCE_FILES_DIR}/bitcount.c"
    "${RESOURCE_FILES_DIR}/bitboard.c"
    "${RESOURCE_FILES_DIR}/coordinates.c"
    "${RESOURCE_FILES_DIR}/crc.c"
    "${RESOURCE_FILES_DIR}/fen.c"
    "${RESOURCE_FILES_DIR}/game_logic.c"
    "${RESOURCE_FILES_DIR}/simplech.c"
    "${RESOURCE_FILES_DIR}/dblookup.c"
    "${RESOURCE_FILES_DIR}/app_instance.cpp"
    "${RESOURCE_FILES_DIR}/AutoThreadWorker.cpp"
    "${RESOURCE_FILES_DIR}/CB_movegen.cpp"
    "${RESOURCE_FILES_DIR}/CheckerBoard.cpp"
    "${RESOURCE_FILES_DIR}/CheckerBoardWidget.cpp"
    "${RESOURCE_FILES_DIR}/checkers_db_api.cpp"
    "${RESOURCE_FILES_DIR}/CommentMoveDialog.cpp"
    "${RESOURCE_FILES_DIR}/EngineCommandDialog.cpp"
    "${RESOURCE_FILES_DIR}/EngineOptionsDialog.cpp"
    "${RESOURCE_FILES_DIR}/EngineSelectDialog.cpp"
    "${RESOURCE_FILES_DIR}/FindCRDialog.cpp"
    "${RESOURCE_FILES_DIR}/FindPositionDialog.cpp"
    "${RESOURCE_FILES_DIR}/GameDatabaseDialog.cpp"
    "${RESOURCE_FILES_DIR}/GameInfoDialog.cpp"
    "${RESOURCE_FILES_DIR}/LoadGameDialog.cpp"
    "${RESOURCE_FILES_DIR}/main.cpp"
    "${RESOURCE_FILES_DIR}/MainWindow.cpp"
    "${RESOURCE_FILES_DIR}/OptionsDialog.cpp"
    "${RESOURCE_FILES_DIR}/PDNfind.cpp"
    "${RESOURCE_FILES_DIR}/PDNparser.cpp"
    "${RESOURCE_FILES_DIR}/PieceSetDialog.cpp"
    "${RESOURCE_FILES_DIR}/SearchThreadWorker.cpp"
    "${RESOURCE_FILES_DIR}/SingleApplication.cpp"
    "${RESOURCE_FILES_DIR}/UserBookManager.cpp"
    "${RESOURCE_FILES_DIR}/utility.cpp"
)

# Output Executable
OUTPUT_EXE="${PROJECT_ROOT}/checkerboard_app"

# Array to hold object files
OBJECT_FILES=()

# Run moc on all header files containing Q_OBJECT
for header_file in "${RESOURCE_FILES_DIR}"/*.h; do
    if grep -q "Q_OBJECT" "${header_file}"; then
        base_name=$(basename "${header_file}" .h)
        moc_file="${RESOURCE_FILES_DIR}/moc_${base_name}.cpp"
        echo "Running moc on ${base_name}.h..."
        moc "${header_file}" -o "${moc_file}"
        CPP_SRCS+=("${moc_file}")
    fi
done

# --- Run uic on all .ui files ---
for ui_file in "${RESOURCE_FILES_DIR}"/*.ui; do
    base_name=$(basename "${ui_file}" .ui)
    h_file="${RESOURCE_FILES_DIR}/ui_${base_name}.h"
    echo "Running uic on ${base_name}.ui..."
    uic "${ui_file}" -o "${h_file}" || { echo "uic failed for ${ui_file}"; exit 1; }
done

# --- Generate and run rcc on resources.qrc ---
echo "Running rcc on resources.qrc..."
QRC_FILE="${RESOURCE_FILES_DIR}/resources.qrc"
RCC_FILE="${RESOURCE_FILES_DIR}/resources.cpp"

rcc -name resources -o "${RCC_FILE}" "${QRC_FILE}" || { echo "rcc failed for resources.qrc"; exit 1; }
CPP_SRCS+=("${RCC_FILE}")

# --- Compile C++ source files ---
echo "Compiling C++ source files..."
for src in "${CPP_SRCS[@]}"; do
    obj="${src%.*}.o"
    OBJECT_FILES+=("${obj}")
    g++ -c "${src}" -o "${obj}" ${QT_CFLAGS} -I${RESOURCE_FILES_DIR} -fPIC -O2 -std=c++11 || { echo "Compilation failed for ${src}"; exit 1; }
done

# --- Link all object files into the executable ---
echo "Linking object files into executable..."
g++ -o "${PROJECT_ROOT}/checkerboard_app" "${OBJECT_FILES[@]}" \
    ${QT_LIBS} \
    -L"${PROJECT_ROOT}/engine" -lkingsrow_egdb \
    -fPIC -O2 -std=c++11 -lrt -lstdc++ || { echo "Linking failed"; exit 1; }

echo "CheckerBoard application compiled to: ${OUTPUT_EXE}"

echo "Build process completed successfully."
