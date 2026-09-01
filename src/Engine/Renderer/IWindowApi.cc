#include "IWindowApi.h"


std::string IWindowApi::GetTitle() const {
    return m_WindowOptions.title;
}
Size IWindowApi::GetSize() const {
    return m_WindowOptions.size;
}
WindowOptions IWindowApi::GetWindowOptions() const{
    return m_WindowOptions;
}