#include "SingleApplication.h"

SingleApplication::SingleApplication(QObject *parent) : QObject(parent)
{
    sharedMemory.setKey("CheckerBoardAppKey");
}

// Function to check if another instance is running
bool SingleApplication::isAnotherInstanceRunning()
{
    if (sharedMemory.attach()) {
        // If we can attach, another instance is running
        return true;
    }

    if (sharedMemory.create(1)) {
        // If we can create, we are the first instance
        return false;
    }

    // If create failed, it might be a race condition or an unclean shutdown.
    // In this case, we can try to attach again.
    if (sharedMemory.attach()) {
        return true;
    }

    return false; // Should not happen
}

// Function to release the shared memory
void SingleApplication::release()
{
    if (sharedMemory.isAttached()) {
        sharedMemory.detach();
    }
}
