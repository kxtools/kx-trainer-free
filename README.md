# KX Trainer Free

**This is the free, open-source version of KX Trainer. For the full-featured premium version with advanced tools and support, please visit our official website: [kxtools.xyz](https://kxtools.xyz)**

---

KX Trainer Free is a simple game utility for Guild Wars 2 that we've decided to make open-source. The code has evolved since its origin in 2018, and we believe in transparency for tools like this.

[➡️ Download the Latest Release](https://github.com/Krixx1337/KX-Trainer-Free/releases/latest)

> **Disclaimer:** This tool is provided for educational purposes. Users are responsible for using this tool in compliance with all applicable terms of service. Use responsibly and at your own risk.

![KX Trainer Free GUI](./images/kx_trainer_free_imgui_v3.png)

## Why Open-Source?

We believe transparency is important, especially for game tools. KX Trainer Free has been around for a while, and opening it up allows the community to see how it works, learn from it, or contribute.

While our main development efforts focus on the advanced features in our **[premium tools available at kxtools.xyz](https://kxtools.xyz)**, KX Trainer Free will be actively maintained to ensure it remains functional after Guild Wars 2 updates and to fix critical bugs. New features exclusive to the premium version will not be added here. We hope the community finds this base useful and potentially builds upon it.

## A Little History

Originating as a hobby project in 2018, KX Trainer Free has evolved significantly. The codebase has recently been refactored into an in-game overlay (DLL injection) to improve structure, clarity, and maintainability.

## Contributions

We welcome contributions! Pull requests for **bug fixes**, **compatibility updates** following game patches, or **code improvements** are highly encouraged. While new features mirroring the paid version won't be merged, community-driven additions that fit the scope of the free trainer are welcome for discussion. Your input is valuable in keeping this tool functional for everyone.

## How to Use

KX Trainer Free is a C++ project for Windows. It injects into Guild Wars 2 as a DLL and draws an in-game menu overlay.

### Prerequisites
- **CMake 3.28+** and **Ninja**
- **Visual Studio 2026** with **Desktop development with C++**, or **Build Tools 2026** with **C++ build tools** (MSVC **v145** toolset)

### Build Instructions

1.  **Clone the Repository**:
    Open your terminal or Git Bash, and clone the repository:
    ```bash
    git clone https://github.com/Krixx1337/KX-Trainer-Free.git
    ```

2.  **Build** from the repository root:
    ```bash
    cmake --preset release
    cmake --build --preset release
    ```

    Outputs are written to `bin/Release/` (`KX-Trainer-Free.dll` and `KXTrainerInject.exe`). Use `--preset debug` for a Debug build (`bin/Debug/`).

### Running

1.  Start Guild Wars 2 and load into the game world.
2.  Run `KXTrainerInject.exe` from `bin/Release/` (keep the DLL in the same folder).
3.  Press **Insert** to toggle the menu. Use the Hotkeys section to set bindings (**Apply Recommended Defaults** for suggested keys). Settings are saved to `config.json` alongside the binaries.

## Contact & Community

For any questions or support, feel free to join our [Discord server](https://discord.gg/z92rnB4kHm).

Explore all our Guild Wars 2 projects and the premium KX Trainer at **[kxtools.xyz](https://kxtools.xyz)**.