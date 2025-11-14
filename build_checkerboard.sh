#!/bin/bash

do_clean() {
    echo "Cleaning up object files and moc files..."
    find . -name "*.o" -delete
    find . -name "moc_*.cpp" -delete
    rm -f "checkerboard_app"
    rm -f "AI_test_app"
    rm -f "${RESOURCE_FILES_DIR}"/*.o
    rm -f "${RESOURCE_FILES_DIR}"/moc_*.cpp
    rm -f "${RESOURCE_FILES_DIR}"/moc_*.o
    rm -f "${RESOURCE_FILES_DIR}"/ui_*.h
    rm -f "${RESOURCE_FILES_DIR}"/resources.cpp
    rm -f "${RESOURCE_FILES_DIR}"/resources.o
    rm -f "${PROJECT_ROOT}"/checkerboard_app
    echo "Cleanup complete."
}

# Define paths
PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
RESOURCE_FILES_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ "$1" == "clean" ]; then
    do_clean
    exit 0
fi

# Force recompilation of c_logic.cpp
rm -f "${RESOURCE_FILES_DIR}/c_logic.cpp.o"

# Set PKG_CONFIG_PATH for Qt5
export PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH

# Qt specific flags
QT_CFLAGS=$(pkg-config --cflags Qt5Widgets Qt5Gui Qt5Core Qt5Multimedia)
QT_LIBS=$(pkg-config --libs Qt5Widgets Qt5Gui Qt5Core Qt5Multimedia)

# Core C++ Source Files
INCLUDE_DIRS=(
    "${RESOURCE_FILES_DIR}"
    "${RESOURCE_FILES_DIR}/CB172source"
    "${RESOURCE_FILES_DIR}/dblookup"
)

INCLUDE_FLAGS="$(printf -- "-I%s " "${INCLUDE_DIRS[@]}")"

# All source files for the main application
APP_SRCS=("${RESOURCE_FILES_DIR}/main.cpp" "${RESOURCE_FILES_DIR}/MainWindow.cpp" "${RESOURCE_FILES_DIR}/GameManager.cpp" "${RESOURCE_FILES_DIR}/c_logic.cpp" "${RESOURCE_FILES_DIR}/dblookup.c" "${RESOURCE_FILES_DIR}/BoardWidget.cpp" "${RESOURCE_FILES_DIR}/engine_wrapper.cpp" "${RESOURCE_FILES_DIR}/GameDatabaseDialog.cpp" "${RESOURCE_FILES_DIR}/FindPositionDialog.cpp" "${RESOURCE_FILES_DIR}/FindCRDialog.cpp" "${RESOURCE_FILES_DIR}/EngineSelectDialog.cpp" "${RESOURCE_FILES_DIR}/EngineOptionsDialog.cpp" "${RESOURCE_FILES_DIR}/PieceSetDialog.cpp" "${RESOURCE_FILES_DIR}/PriorityDialog.cpp" "${RESOURCE_FILES_DIR}/ThreeMoveOptionsDialog.cpp" "${RESOURCE_FILES_DIR}/DirectoriesDialog.cpp" "${RESOURCE_FILES_DIR}/UserBookDialog.cpp" "${RESOURCE_FILES_DIR}/GeminiAI.cpp")

# All source files for the test application
TEST_SRCS=("${RESOURCE_FILES_DIR}/test_main.cpp" "${RESOURCE_FILES_DIR}/GameManager.cpp" "${RESOURCE_FILES_DIR}/c_logic.cpp" "${RESOURCE_FILES_DIR}/dblookup.c" "${RESOURCE_FILES_DIR}/GeminiAI.cpp" "${RESOURCE_FILES_DIR}/engine_wrapper.cpp")

# Object files for the main application
APP_OBJS=()
# Object files for the test application
TEST_OBJS=()

# Generate moc files
APP_MOC_OBJS=() # New array for application-specific moc objects
TEST_MOC_OBJS=() # New array for test-specific moc objects

for header_file in "${RESOURCE_FILES_DIR}"/*.h; do
    if grep -q Q_OBJECT "${header_file}"; then
        base_name=$(basename "${header_file}" .h)
        moc_cpp_file="${RESOURCE_FILES_DIR}/moc_${base_name}.cpp"
        moc_obj_file="${RESOURCE_FILES_DIR}/moc_${base_name}.o"
        echo "Running moc on ${base_name}.h..."
        moc "${header_file}" -o "${moc_cpp_file}" ${INCLUDE_FLAGS}
        echo "Compiling ${moc_cpp_file}..."
        g++ -c "${moc_cpp_file}" -o "${moc_obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11
        
        if [[ "${base_name}" == "test_game_logic" ]]; then
            TEST_MOC_OBJS+=("${moc_obj_file}")
        elif [[ "${base_name}" == "GameManager" || "${base_name}" == "GeminiAI" || "${base_name}" == "engine_wrapper" ]]; then
            APP_MOC_OBJS+=("${moc_obj_file}")
            TEST_MOC_OBJS+=("${moc_obj_file}")
        else
            APP_MOC_OBJS+=("${moc_obj_file}")
        fi
    fi
done

# Handle Qt UI Files
echo 'Running uic on UI files...'
for ui_file in "${RESOURCE_FILES_DIR}"/*.ui; do
    base_name=$(basename "${ui_file}" .ui)
    ui_header_file="${RESOURCE_FILES_DIR}/ui_${base_name}.h"
    echo "Running uic on ${base_name}.ui..."
    uic "${ui_file}" -o "${ui_header_file}"
done

# Handle Qt Resource File
echo 'Running rcc on resources.qrc...'
QRC_FILE="${RESOURCE_FILES_DIR}/resources.qrc"
RCC_FILE="${RESOURCE_FILES_DIR}/resources.cpp"
RCC_OBJ_FILE="${RESOURCE_FILES_DIR}/resources.o"
rcc -name resources -o "${RCC_FILE}" "${QRC_FILE}"
echo "Compiling ${RCC_FILE}..."
g++ -c "${RCC_FILE}" -o "${RCC_OBJ_FILE}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11
APP_MOC_OBJS+=("${RCC_OBJ_FILE}") # Add resources.o to APP_MOC_OBJS

# --- Compile Application Source Files ---
echo "Compiling application source files..."
for src_file in "${APP_SRCS[@]}"; do
    obj_file="${src_file%.*}.o"
    echo "Compiling ${src_file}..."
    if [[ "${src_file}" == *.c ]]; then
        g++ -c "${src_file}" -o "${obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11 # Changed gcc to g++ and added QT_CFLAGS and -std=c++11
    else
        g++ -c "${src_file}" -o "${obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11
    fi
    APP_OBJS+=("${obj_file}")
done

# --- Compile Test Source Files ---
echo "Compiling test source files..."
for src_file in "${TEST_SRCS[@]}"; do
    obj_file="${src_file%.*}.o"
    echo "Compiling ${src_file}..."
    if [[ "${src_file}" == *.c ]]; then
        g++ -c "${src_file}" -o "${obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11 # Changed gcc to g++ and added QT_CFLAGS and -std=c++11
    else
        g++ -c "${src_file}" -o "${obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11
    fi
    TEST_OBJS+=("${obj_file}")
done

# --- Link Application ---
echo "Linking application..."
# ----- THIS IS THE FIXED CODE -----
g++ -o "${RESOURCE_FILES_DIR}/checkerboard_app" "${APP_OBJS[@]}" "${APP_MOC_OBJS[@]}" ${QT_LIBS} -L/usr/local/lib -lrt -lstdc++
if [ $? -ne 0 ]; then
    echo "Application linking failed!"
    exit 1
fi
echo "Application built successfully: ${PROJECT_ROOT}/checkerboard_app"

# --- Link Test Runner ---
echo "Linking test runner..."
g++ -o "${RESOURCE_FILES_DIR}/test_runner" "${TEST_OBJS[@]}" "${TEST_MOC_OBJS[@]}" ${QT_LIBS} -L/usr/local/lib -lrt -lstdc++ -lQt5Test
if [ $? -ne 0 ]; then
    echo "Test runner linking failed!"
    exit 1
fi
echo "Test runner built successfully: ${PROJECT_ROOT}/test_runner"

# --- Test Target ---
if [ "$1" == "test" ]; then
    echo "Running tests..."
    rm -f app.log # Clear the log file before running tests
    "${RESOURCE_FILES_DIR}/test_runner"
    if [ $? -ne 0 ]; then
        echo "Tests failed!"
        exit 1
    fi
    echo "All tests passed!"
    exit 0
fi

echo "Build process completed successfully."
