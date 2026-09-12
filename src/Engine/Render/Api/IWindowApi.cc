#include "IWindowApi.h"
static IWindowApi* g_WindowApi;

std::string IWindowApi::GetTitle() const {
    return m_WindowOptions.title;
}
Size IWindowApi::GetSize() const {
    return m_WindowOptions.size;
}
WindowOptions IWindowApi::GetWindowOptions() const{
    return m_WindowOptions;
}

IWindowApi* GetGlobalWindowApi() {
    return g_WindowApi;
}
void SetGlobalWindowApi(IWindowApi* api) {
    g_WindowApi = api;
}

