#!/bin/bash

do_clean() {
    echo "Cleaning up object files and moc files..."
    find . -name "*.o" -delete
    find . -name "moc_*.cpp" -delete
    rm -f "headless_checkers"
    rm -f "${RESOURCE_FILES_DIR}"/*.o
    rm -f "${RESOURCE_FILES_DIR}"/moc_*.cpp
    rm -f "${RESOURCE_FILES_DIR}"/moc_*.o
    rm -f "${RESOURCE_FILES_DIR}"/ui_*.h
    rm -f "${RESOURCE_FILES_DIR}"/resources.cpp
    rm -f "${RESOURCE_FILES_DIR}"/resources.o
    rm -f "${PROJECT_ROOT}"/headless_checkers

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
QT_CFLAGS=$(pkg-config --cflags Qt5Core)
QT_LIBS=$(pkg-config --libs Qt5Core)

# Core C++ Source Files
INCLUDE_DIRS=(
    "${RESOURCE_FILES_DIR}"
    "${RESOURCE_FILES_DIR}/CB172source"
)

INCLUDE_FLAGS="$(printf -- "-I%s " "${INCLUDE_DIRS[@]}")"

# All source files for the main application
APP_SOURCES="headless_main.cpp c_logic.cpp DBManager.cpp GeminiAI.cpp AIWorker.cpp log.cpp core_types.cpp Logger.cpp"

# Object files for the main application
APP_OBJS=()

# Generate moc files
APP_MOC_OBJS=() 

for header_file in "${RESOURCE_FILES_DIR}"/*.h; do
    base_name=$(basename "${header_file}" .h)
    
    # Skip headers that don't use Q_OBJECT
    if ! grep -q Q_OBJECT "${header_file}"; then
        continue
    fi

    # Skip GUI-specific headers
    if [[ "${base_name}" == "MainWindow" || "${base_name}" == "BoardWidget" || "${base_name}" == "Dialogs" || "${base_name}" == "GameManager" || "${base_name}" == "engine_wrapper" ]]; then
        continue
    fi

    moc_cpp_file="${RESOURCE_FILES_DIR}/moc_${base_name}.cpp"
    moc_obj_file="${RESOURCE_FILES_DIR}/moc_${base_name}.o"
    echo "Running moc on ${base_name}.h..."
    moc "${header_file}" -o "${moc_cpp_file}" ${INCLUDE_FLAGS}
    echo "Compiling ${moc_cpp_file}..."
    g++ -c "${moc_cpp_file}" -o "${moc_obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -g -std=c++17
    
    APP_MOC_OBJS+=("${moc_obj_file}")
done



# --- Compile Application Source Files ---
echo "Compiling application source files..."
for src_file in ${APP_SOURCES}; do
    obj_file="${src_file%.*}.o"
    echo "Compiling ${src_file}..."
    if [[ "${src_file}" == *.c ]]; then
        g++ -c "${src_file}" -o "${obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -g -std=c++17
    else
        g++ -c "${src_file}" -o "${obj_file}" ${INCLUDE_FLAGS} ${QT_CFLAGS} -fPIC -O2 -g -std=c++17
    fi
    APP_OBJS+=("${obj_file}")
done

# --- Link Application ---
echo "Linking application..."
g++ -o "${RESOURCE_FILES_DIR}/headless_checkers" "${APP_OBJS[@]}" "${APP_MOC_OBJS[@]}" ${QT_LIBS} -L/usr/local/lib -lrt -lstdc++ -g
if [ $? -ne 0 ]; then
    echo "Application linking failed!"
    exit 1
fi
echo "Application built successfully: ${PROJECT_ROOT}/headless_checkers"

echo "Build process completed successfully."
