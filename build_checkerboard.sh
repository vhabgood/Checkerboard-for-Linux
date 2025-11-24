#!/bin/bash

do_clean() {
    echo "Cleaning up object files and moc files..."
    find . -name "*.o" -delete
    find . -name "moc_*.cpp" -delete
    rm -f "checkerboard_app"
    rm -f "${RESOURCE_FILES_DIR}"/*.o
    rm -f "${RESOURCE_FILES_DIR}"/moc_*.cpp
    rm -f "${RESOURCE_FILES_DIR}"/moc_*.o
    rm -f "${RESOURCE_FILES_DIR}"/ui_*.h
    rm -f "${RESOURCE_FILES_DIR}"/resources.cpp
    rm -f "${RESOURCE_FILES_DIR}"/resources.o
    rm -f "${PROJECT_ROOT}"/checkerboard_app
    rm -f "${RESOURCE_FILES_DIR}"/GameDatabaseDialog.cpp
    rm -f "${RESOURCE_FILES_DIR}"/GameDatabaseDialog.h
    rm -f "${RESOURCE_FILES_DIR}"/GameDatabaseDialog.o
    rm -f "${RESOURCE_FILES_DIR}"/moc_GameDatabaseDialog.cpp
    rm -f "${RESOURCE_FILES_DIR}"/moc_GameDatabaseDialog.o
    # Explicitly remove test-related moc files
    rm -f "${RESOURCE_FILES_DIR}"/moc_GameManager_test.cpp
    rm -f "${RESOURCE_FILES_DIR}"/moc_GameManager_test.o
    rm -f "${RESOURCE_FILES_DIR}"/moc_test_game_logic.cpp
    rm -f "${RESOURCE_FILES_DIR}"/moc_test_game_logic.o
    echo "Cleanup complete."
}

# Define paths
PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
RESOURCE_FILES_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Explicitly remove GameDatabaseDialog files before building
rm -f "${RESOURCE_FILES_DIR}/GameDatabaseDialog.cpp"
rm -f "${RESOURCE_FILES_DIR}/GameDatabaseDialog.h"
rm -f "${RESOURCE_FILES_DIR}/GameDatabaseDialog.o"
rm -f "${RESOURCE_FILES_DIR}/moc_GameDatabaseDialog.cpp"
rm -f "${RESOURCE_FILES_DIR}/moc_GameDatabaseDialog.o"

if [ "$1" == "clean" ]; then
    do_clean
    exit 0
fi

# Force recompilation of c_logic.cpp
rm -f "${RESOURCE_FILES_DIR}/c_logic.cpp.o"
rm -f "${RESOURCE_FILES_DIR}/GameManager.o"
rm -f "${RESOURCE_FILES_DIR}/MainWindow.o"
rm -f "${RESOURCE_FILES_DIR}/main.o"
rm -f "${RESOURCE_FILES_DIR}/moc_MainWindow.cpp"

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
APP_SRCS=("${RESOURCE_FILES_DIR}/main.cpp" "${RESOURCE_FILES_DIR}/MainWindow.cpp" "${RESOURCE_FILES_DIR}/GameManager.cpp" "${RESOURCE_FILES_DIR}/c_logic.cpp" "${RESOURCE_FILES_DIR}/dblookup.c" "${RESOURCE_FILES_DIR}/BoardWidget.cpp" "${RESOURCE_FILES_DIR}/engine_wrapper.cpp" "${RESOURCE_FILES_DIR}/FindPositionDialog.cpp" "${RESOURCE_FILES_DIR}/FindCRDialog.cpp" "${RESOURCE_FILES_DIR}/EngineSelectDialog.cpp" "${RESOURCE_FILES_DIR}/EngineOptionsDialog.cpp" "${RESOURCE_FILES_DIR}/PieceSetDialog.cpp" "${RESOURCE_FILES_DIR}/PriorityDialog.cpp" "${RESOURCE_FILES_DIR}/ThreeMoveOptionsDialog.cpp" "${RESOURCE_FILES_DIR}/DirectoriesDialog.cpp" "${RESOURCE_FILES_DIR}/UserBookDialog.cpp" "${RESOURCE_FILES_DIR}/GeminiAI.cpp" "${RESOURCE_FILES_DIR}/log.cpp")

# Object files for the main application
APP_OBJS=()

# Generate moc files
APP_MOC_OBJS=() 

for header_file in "${RESOURCE_FILES_DIR}"/*.h; do
    base_name=$(basename "${header_file}" .h)
    
    # Skip test headers during moc generation for the main application
    if [[ "${base_name}" == "test_game_logic" ]]; then
        continue
    fi

    if grep -q Q_OBJECT "${header_file}"; then
        moc_cpp_file="${RESOURCE_FILES_DIR}/moc_${base_name}.cpp"
        moc_obj_file="${RESOURCE_FILES_DIR}/moc_${base_name}.o"
        echo "Running moc on ${base_name}.h..."
        moc "${header_file}" -o "${moc_cpp_file}" ${INCLUDE_FLAGS}
        echo "Compiling ${moc_cpp_file}..."
        g++ -c "${moc_cpp_file}" -o "${moc_obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11
        
        # All generated moc objects are for the app
        APP_MOC_OBJS+=("${moc_obj_file}")
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
        g++ -c "${src_file}" -o "${obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11
    else
        g++ -c "${src_file}" -o "${obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -std=c++11
    fi
    APP_OBJS+=("${obj_file}")
done

# --- Link Application ---
echo "Linking application..."
g++ -o "${RESOURCE_FILES_DIR}/checkerboard_app" "${APP_OBJS[@]}" "${APP_MOC_OBJS[@]}" ${QT_LIBS} -L/usr/local/lib -lrt -lstdc++
if [ $? -ne 0 ]; then
    echo "Application linking failed!"
    exit 1
fi
echo "Application built successfully: ${PROJECT_ROOT}/checkerboard_app"

echo "Build process completed successfully."