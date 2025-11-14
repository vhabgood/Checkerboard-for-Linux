
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

# Core C++ Source Files
INCLUDE_DIRS=(
    "${RESOURCE_FILES_DIR}"
    "${RESOURCE_FILES_DIR}/CB172source"
    "${RESOURCE_FILES_DIR}/dblookup"
)

# Source files
TEST_SRC="${RESOURCE_FILES_DIR}/simple_test.cpp"
DBLOOKUP_SRC="${RESOURCE_FILES_DIR}/dblookup/dblookup.c"
C_LOGIC_SRC="${RESOURCE_FILES_DIR}/c_logic.c"

# Object files
TEST_OBJ="${RESOURCE_FILES_DIR}/simple_test.o"
DBLOOKUP_OBJ="${RESOURCE_FILES_DIR}/dblookup.o"
C_LOGIC_OBJ="${RESOURCE_FILES_DIR}/c_logic.o"

# Output Executable
OUTPUT_EXE="${RESOURCE_FILES_DIR}/simple_test"

# Compile the C source files as C++
g++ -c "${DBLOOKUP_SRC}" -o "${DBLOOKUP_OBJ}" ${INCLUDE_DIRS[@]/#/-I} ${QT_CFLAGS} -fPIC -O2 -std=c++11 || { echo "Compilation failed for ${DBLOOKUP_SRC}"; exit 1; }
g++ -c "${C_LOGIC_SRC}" -o "${C_LOGIC_OBJ}" ${INCLUDE_DIRS[@]/#/-I} ${QT_CFLAGS} -fPIC -O2 -std=c++11 || { echo "Compilation failed for ${C_LOGIC_SRC}"; exit 1; }




# Compile the C++ source files
g++ -c "${TEST_SRC}" -o "${TEST_OBJ}" ${INCLUDE_DIRS[@]/#/-I} ${QT_CFLAGS} -fPIC -O2 -std=c++11 || { echo "Compilation failed for ${TEST_SRC}"; exit 1; }


# Link the object files
g++ -o "${OUTPUT_EXE}" "${TEST_OBJ}" "${DBLOOKUP_OBJ}" "${C_LOGIC_OBJ}" ${QT_LIBS} -lstdc++ || { echo "Linking failed"; exit 1; }

# Run the test
"${OUTPUT_EXE}"
