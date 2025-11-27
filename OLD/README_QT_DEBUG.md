To get more verbose debug output from Qt, which might help us pinpoint the specific component triggering the `QSocketNotifier` error, please try running the application with the following environment variables set:

```bash
export QT_LOGGING_RULES="qt.qpa.*=true;qt.gui.*=true;qt.core.*=true"
export QT_DEBUG_PLUGINS=1
./checkerboard_app
```

Please run these commands in your terminal and provide the *entire* output, including any new debug messages from Qt. This might give us clues about which plugin or internal Qt component is causing the issue.
