#pragma once

#include <atomic>
#include <windows.h>

namespace kx {

// Shared-HWND overlay input: WndProc queues messages; Present drains and owns ImGui.
class ImGuiGameWndProc {
public:
    bool install(HWND hwnd);
    void uninstall();

    void setMenuVisible(bool visible);
    void setImGuiReady(bool ready);

    void drainInput();
    void applyDeferredInput();
    void publishCapture();

    HWND hwnd() const { return hwnd_; }

private:
    static LRESULT CALLBACK trampoline(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT dispatch(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT forwardToGame(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void syncStuckKeys(HWND hwnd);
    void reset();

    static ImGuiGameWndProc* instance_;

    HWND hwnd_ = nullptr;
    WNDPROC original_ = nullptr;
    std::atomic<bool> menuVisible_{false};
    std::atomic<bool> imguiReady_{false};
    bool gameKeyDown_[256]{};
};

} // namespace kx
