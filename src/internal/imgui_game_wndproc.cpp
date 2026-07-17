#include "internal/imgui_game_wndproc.h"

#include "imgui/imgui.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandlerEx(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, ImGuiIO& io);

namespace kx {
namespace {

constexpr size_t kQueueCap = 64;
constexpr uint32_t kCaptureMouse = 1u << 0;
constexpr uint32_t kWantTextInput = 1u << 1;

struct QueuedMessage {
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

class OverlayInputQueue {
public:
    void enqueue(UINT msg, WPARAM wParam, LPARAM lParam) {
        std::lock_guard lock(mutex_);
        const bool isMove = msg == WM_MOUSEMOVE;
        if (isMove && count_ > 0) {
            const size_t last = (tail_ + kQueueCap - 1) % kQueueCap;
            if (ring_[last].msg == WM_MOUSEMOVE) {
                ring_[last] = {msg, wParam, lParam};
                return;
            }
        }
        if (count_ >= kQueueCap && isMove)
            return;
        if (count_ >= kQueueCap) {
            head_ = (head_ + 1) % kQueueCap;
            --count_;
        }
        ring_[tail_] = {msg, wParam, lParam};
        tail_ = (tail_ + 1) % kQueueCap;
        ++count_;
    }

    void requestClearInput() { clearInput_.store(true, std::memory_order_release); }

    uint32_t captureFlags() const { return captureFlags_.load(std::memory_order_acquire); }

    void drain(HWND hwnd) {
        if (!hwnd || !ImGui::GetCurrentContext())
            return;

        ImGuiIO& io = ImGui::GetIO();
        thread_local std::array<QueuedMessage, kQueueCap> pending;
        size_t pendingCount = 0;
        {
            std::lock_guard lock(mutex_);
            while (count_ > 0) {
                pending[pendingCount++] = ring_[head_];
                head_ = (head_ + 1) % kQueueCap;
                --count_;
            }
        }
        for (size_t i = 0; i < pendingCount; ++i) {
            const QueuedMessage& m = pending[i];
            ImGui_ImplWin32_WndProcHandlerEx(hwnd, m.msg, m.wParam, m.lParam, io);
        }
    }

    void applyDeferred() {
        if (!ImGui::GetCurrentContext() || !clearInput_.exchange(false, std::memory_order_acq_rel))
            return;
        ImGuiIO& io = ImGui::GetIO();
        io.ClearInputMouse();
        io.ClearInputKeys();
    }

    void publishCapture() {
        if (!ImGui::GetCurrentContext()) {
            captureFlags_.store(0, std::memory_order_release);
            return;
        }
        const ImGuiIO& io = ImGui::GetIO();
        captureFlags_.store(
            (io.WantCaptureMouse ? kCaptureMouse : 0) | (io.WantTextInput ? kWantTextInput : 0),
            std::memory_order_release);
    }

    void reset() {
        {
            std::lock_guard lock(mutex_);
            head_ = tail_ = count_ = 0;
        }
        captureFlags_.store(0, std::memory_order_release);
        clearInput_.store(false, std::memory_order_release);
    }

private:
    std::mutex mutex_;
    std::array<QueuedMessage, kQueueCap> ring_{};
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
    std::atomic<uint32_t> captureFlags_{0};
    std::atomic<bool> clearInput_{false};
};

OverlayInputQueue g_input;

enum MsgKind : uint8_t { kNone, kMouse, kMouseUp, kKeyDownOrChar };

MsgKind classifyMessage(UINT msg) {
    switch (msg) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
        return kMouse;
    case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: case WM_XBUTTONUP:
        return kMouseUp;
    case WM_KEYDOWN: case WM_SYSKEYDOWN: case WM_CHAR:
        return kKeyDownOrChar;
    default:
        return kNone;
    }
}

} // namespace

ImGuiGameWndProc* ImGuiGameWndProc::instance_ = nullptr;

bool ImGuiGameWndProc::install(HWND hwnd) {
    if (hwnd && hwnd_ == hwnd && instance_ == this)
        return true;
    if (!hwnd || hwnd_)
        return false;
    SetLastError(ERROR_SUCCESS);
    const LONG_PTR previous = SetWindowLongPtrW(
        hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&trampoline));
    if (!previous && GetLastError() != ERROR_SUCCESS)
        return false;
    hwnd_ = hwnd;
    original_ = reinterpret_cast<WNDPROC>(previous);
    instance_ = this;
    return true;
}

void ImGuiGameWndProc::uninstall() {
    if (hwnd_ && original_ &&
        reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd_, GWLP_WNDPROC)) == &trampoline) {
        SetWindowLongPtrW(hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_));
    }
    if (instance_ == this)
        instance_ = nullptr;
    g_input.reset();
    reset();
}

void ImGuiGameWndProc::reset() {
    hwnd_ = nullptr;
    original_ = nullptr;
    menuVisible_.store(false, std::memory_order_release);
    imguiReady_.store(false, std::memory_order_release);
    std::memset(gameKeyDown_, 0, sizeof(gameKeyDown_));
}

void ImGuiGameWndProc::setMenuVisible(bool visible) {
    menuVisible_.store(visible, std::memory_order_release);
}

void ImGuiGameWndProc::setImGuiReady(bool ready) {
    imguiReady_.store(ready, std::memory_order_release);
}

void ImGuiGameWndProc::drainInput() { g_input.drain(hwnd_); }
void ImGuiGameWndProc::applyDeferredInput() { g_input.applyDeferred(); }
void ImGuiGameWndProc::publishCapture() { g_input.publishCapture(); }

LRESULT CALLBACK ImGuiGameWndProc::trampoline(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return instance_ ? instance_->dispatch(hwnd, msg, wParam, lParam)
                     : DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT ImGuiGameWndProc::dispatch(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    const bool focusLost = msg == WM_KILLFOCUS || (msg == WM_ACTIVATEAPP && !wParam);
    if (focusLost)
        g_input.requestClearInput();
    if (focusLost || msg == WM_EXITSIZEMOVE || msg == WM_CAPTURECHANGED)
        syncStuckKeys(hwnd);

    if (!imguiReady_.load(std::memory_order_acquire) ||
        !menuVisible_.load(std::memory_order_acquire))
        return forwardToGame(hwnd, msg, wParam, lParam);

    g_input.enqueue(msg, wParam, lParam);

    const uint32_t flags = g_input.captureFlags();
    if (!flags)
        return forwardToGame(hwnd, msg, wParam, lParam);

    const MsgKind kind = classifyMessage(msg);
    const bool captureMouse = (flags & kCaptureMouse) != 0;
    const bool wantTextInput = (flags & kWantTextInput) != 0;

    if (captureMouse && kind == kMouse)
        return 1;

    if (wantTextInput && kind == kKeyDownOrChar && wParam < 256)
        return 1;

    if (captureMouse && kind == kMouseUp)
        syncStuckKeys(hwnd);

    return forwardToGame(hwnd, msg, wParam, lParam);
}

LRESULT ImGuiGameWndProc::forwardToGame(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!original_)
        return DefWindowProcW(hwnd, msg, wParam, lParam);

    const LRESULT result = CallWindowProcW(original_, hwnd, msg, wParam, lParam);
    if (wParam >= 256)
        return result;

    switch (msg) {
    case WM_KEYDOWN:  case WM_SYSKEYDOWN:  gameKeyDown_[wParam] = true;  break;
    case WM_KEYUP:    case WM_SYSKEYUP:    gameKeyDown_[wParam] = false; break;
    default: break;
    }
    return result;
}

void ImGuiGameWndProc::syncStuckKeys(HWND hwnd) {
    if (!original_)
        return;

    for (int vk = 0; vk < 256; ++vk) {
        if (!gameKeyDown_[vk] || (GetAsyncKeyState(vk) & 0x8000))
            continue;
        const bool alt = (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU);
        const UINT scan = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
        const LPARAM lp = static_cast<LPARAM>((scan << 16) | (1u << 30) | (1u << 31));
        CallWindowProcW(original_, hwnd, alt ? WM_SYSKEYUP : WM_KEYUP, vk, lp);
        gameKeyDown_[vk] = false;
    }
}

} // namespace kx
