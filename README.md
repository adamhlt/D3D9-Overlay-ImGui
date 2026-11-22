<div align="center">

# D3D9 Overlay ImGui

**DirectX 9 overlay with ImGui in C++ (x86 / x64), built as an EXE or a DLL**

![Language](https://img.shields.io/badge/C%2B%2B-16171C?logo=cplusplus)
![Platform](https://img.shields.io/badge/Windows-16171C)
![Arch](https://img.shields.io/badge/x86%20%7C%20x64-16171C)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-16171C)](LICENSE)

</div>

---

D3D9 Overlay ImGui creates a transparent DirectX 9 window with ImGui and draws it on top of a game or an application.

The overlay has its own window, so the target process does not need to use DirectX. The same code base builds as an EXE (external overlay) or as a DLL (internal overlay) and handles x86 and x64.

### DEMO (DLL - OBS)
https://github.com/adamhlt/D3D9-Overlay-ImGui/assets/48086737/3256716b-a66c-4e04-ad53-43e6377aebff

### DEMO (EXE - Window Picker)
https://github.com/adamhlt/D3D9-Overlay-ImGui/assets/48086737/b4e8c7e9-68a1-4b25-94fb-793b4704e693

## Features

**Overlay**
- Transparent window that follows the target window (position, size and focus)
- The overlay is not captured by OBS or window capture applications
- The overlay is click-through, except when the menu is showed
- Closes by itself when the target window is closed

**EXE (external)**
- Window picker to choose the window to draw on

**DLL (internal)**
- Targets the main window of the process it is injected in, and unloads itself when the overlay is closed

**Drawing**
- ImGui menu ready to be customized
- Direct D3D9 drawing, with a filled rectangle as example

## Usage

| Key      | Action                   |
| -------- | ------------------------ |
| `INSERT` | Show / hide the menu     |
| `END`    | Close the overlay        |

- **EXE:** run the executable, select a window in the picker and click on `Start Overlay`.
- **DLL:** load the DLL in the target process, the overlay starts by itself.

To add your own content, edit `Drawing::Draw` (ImGui) and `Drawing::DXDraw` (D3D9) in `Drawing.cpp`.

## Build

1. Open `D3D9 Overlay ImGui.sln` in Visual Studio 2022 (C++17).
2. Choose a configuration and build.

| Configuration              | Output |
| -------------------------- | ------ |
| `Debug` / `Release`        | EXE    |
| `Debug DLL` / `Release DLL`| DLL    |

- **Architecture:** every configuration is available in x86 and x64
- **Dependencies:** the DirectX SDK is not needed, the Windows SDK is enough

> **Note**
> The DLL must have the same architecture as the target process.

## Libraries

- [ImGui](https://github.com/ocornut/imgui) (v1.89.6)

## License

This project is licensed under the [GNU GPLv3](LICENSE).
