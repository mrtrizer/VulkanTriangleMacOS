#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void createMacOsApp (void (*initHandler)(void*, void*), void (*updateHandler)(void*));

void setUserData(void* pWindow, void* userDataPtr);

void initVulkan(void* controller, void* caMetalLayer);

#ifdef __cplusplus
}
#endif

