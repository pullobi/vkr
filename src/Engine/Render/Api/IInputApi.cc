#include "IInputApi.h"

static IInputApi* g_InputAPI;

void SetGlobalInputApi(IInputApi* inputApi){
    g_InputAPI = inputApi;
};
IInputApi* GetGlobalInputApi() {
    return g_InputAPI;
};