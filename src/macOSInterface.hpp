#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct MacOsApp {
        void* nsViewController;
        void* caMetalLayer;
        void* nsWindow;
    } MacOsApp;

    MacOsApp createMacOsApp (void (*updateHandler)(void*));

    void setUserData(const MacOsApp* macOsApp, void* userDataPtr);

    void runMacOsApp(MacOsApp* macOsApp);

#ifdef __cplusplus
}
#endif

