#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const void* initMetal(const void* pView);

void* createMacOsApp ();

void runMacOsApp(void*, void (*callback_)(void*), void* userDataPtr);

#ifdef __cplusplus
}
#endif

