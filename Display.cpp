#include <windows.h>  // this is necessary for get_spotify window 
#include <tlhelp32.h> // this is necessary for get_spotify window 

// you will need to include this in your imgui project 
std::string get_spotify_title() {
    struct spotify_info {
        std::wstring title;
        std::vector<DWORD> pids;
    } info;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry = { sizeof(entry) };
    if (Process32FirstW(snap, &entry)) {
        do {
            if (std::wstring(entry.szExeFile) == L"Spotify.exe") info.pids.push_back(entry.th32ProcessID);
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* info = reinterpret_cast<spotify_info*>(lParam);
        DWORD id;
        GetWindowThreadProcessId(hwnd, &id);
        if (std::find(info->pids.begin(), info->pids.end(), id) == info->pids.end()) return TRUE;

        std::wstring title(GetWindowTextLength(hwnd) + 1, L'\0');
        GetWindowTextW(hwnd, &title[0], title.size());
        if (title.empty() || wcsstr(title.c_str(), L"GDI+ Window (Spotify.exe)") ||
            wcsstr(title.c_str(), L"Default IME") || wcsstr(title.c_str(), L"MSCTFIME UI")) return TRUE;

        info->title = title;
        return FALSE;
    }, reinterpret_cast<LPARAM>(&info));

    if (info.title.empty() || wcsstr(info.title.c_str(), L"spotify")) return " Stopped / Paused ";

    int s_n = WideCharToMultiByte(CP_UTF8, 0, info.title.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> buffer(s_n);
    WideCharToMultiByte(CP_UTF8, 0, info.title.c_str(), -1, buffer.data(), s_n, nullptr, nullptr);


    std::string utf(buffer.data());
    if (utf.find("Spotify Premium") != std::string::npos ||
        utf.find("Spotify Free") != std::string::npos ||
        utf.find("Spotify") != std::string::npos) return " Stopped / Paused ";

    return " " + utf + " ";
}

// if you dont input this the window wont scale right >( put it where your other style / theme elements are defined )< 
ImGuiStyle& style = ImGui::GetStyle();
style.WindowMinSize = ImVec2(1.f, 1.f);

static int gradient_size = 40;
static float l_change_time = 0.f, l_change_interval = 0.15f;
static std::string title, rotated_title;
// shit code i know
ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 5 });
if (ImGui::Begin("Spotify", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    if (auto new_title = get_spotify_title(); new_title != title) {
        title = rotated_title = new_title;
    }

    if ((l_change_time += ImGui::GetIO().DeltaTime) >= l_change_interval && rotated_title.length() > 1) {
        std::rotate(rotated_title.begin(), rotated_title.begin() + 1, rotated_title.end());
        l_change_time = 0.f;
    }

    ImVec2 text_size = ImGui::CalcTextSize(rotated_title.c_str());
    ImGui::SetWindowSize(ImVec2(text_size.x, text_size.y + 10));

    Renderer::Text(window->DrawList, window->DC.CursorPos, rotated_title, Menu::Colors::White, Menu::Colors::Black);
    Renderer::Gradient(window->DrawList, window->Pos, ImVec2(gradient_size, window->Size.y), Menu::Colors::Black, Menu::Colors::Clear, Menu::Colors::Clear, Menu::Colors::Black);
    Renderer::Gradient(window->DrawList, ImVec2(window->Pos.x + window->Size.x - gradient_size, window->Pos.y), ImVec2(gradient_size, window->Size.y), Menu::Colors::Clear, Menu::Colors::Black, Menu::Colors::Black, Menu::Colors::Clear);
}
ImGui::End();
ImGui::PopStyleVar();



