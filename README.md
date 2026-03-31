# Bixim: ESP32-Based Virtual Pet Embedded System

[English](README.md) | [Português](README.pt-br.md)

## Project Overview
The **Bixim** project (provisional/code name) is a complete implementation (hardware and software) of a portable digital entertainment device, inspired by the classic virtual pet architecture of 1996.

Unlike traditional emulators that rely on proprietary ROM dumps, this project uses a **High-Level Emulation (HLE) and Custom Logic** approach. This ensures total independence from third-party intellectual property, resulting in a 100% original work that demonstrates proficiency in reverse engineering concepts, embedded systems development, and hardware design.

## Software Architecture and Technology Choice

The system core was developed entirely in **C/C++17**. The choice of this language is justified by the following factors:

1. **Memory Management and Performance:** Bare-metal or RTOS-based development on the ESP32 requires strict control over memory allocation and CPU cycles. C/C++ provides the granularity needed to ensure a stable frame rate (FPS) on the OLED display, without the bottlenecks of interpreted languages.
2. **Energy Efficiency:** Natively compiled code minimizes processor active time, allowing for more efficient use of ESP32 *deep sleep* strategies, which is critical for a device powered by a small LiPo battery.
3. **Hardware Interaction:** Direct manipulation of registers, interrupts (ISRs) for button reading, and pulse-width modulation (PWM) control for the buzzer are implemented with maximum efficiency using native C/C++ libraries.

### Software Architecture: HAL and Game Loop

The project implements a modern game engine architecture designed for deterministic behavior and portability:

* **Hardware Abstraction Layer (HAL):** Key interfaces (`IDisplay`, `ITimer`) allow the same logic code to run unmodified on both Windows (using Raylib for simulation) and ESP32 (using SSD1306 OLED drivers via I2C).
* **Game Loop Architecture:** Uses a **Fixed Timestep Accumulator**. This decouples logic (running at a fixed 10 Hz) from rendering (targeting 30 FPS), ensuring consistent simulation speed regardless of CPU variance or display refreshrate.
* **State Logic (Custom Game Engine):** The internal engine implements a finite state machine (FSM) that simulates the system's organic cycle:
    * **Life Module:** Manages temporal decay variables (Hunger, Sleep, Happiness, Age).
    * **Rendering Module:** A custom **FrameBuffer** class manages the 128x64 monochromatic bit matrix, optimized for I2C transfers.
    * **Interrupt Module:** Handles hardware/software debouncing of physical buttons.

## Hardware Specifications (Bill of Materials - BOM)

The physical project was designed to be modular during the prototyping phase (breadboard) and highly miniaturized for the production version.

* **Microcontroller: ESP32 (WROOM or C3)**
    * Responsible for central processing. The ESP32 allows for future Over-The-Air (OTA) updates and wireless communication via Wi-Fi/Bluetooth (ESP-NOW).
* **Display: 0.96" OLED Module (SSD1306 Controller)**
    * High-contrast, ultra-low power monochromatic video interface. Communication via I2C protocol (SDA/SCL), using only two data pins from the microcontroller.
* **Audio: Passive Piezoelectric Buzzer**
    * Unlike active buzzers, the passive version allows for square wave synthesis via PWM at different frequencies, enabling faithful recreation of 8-bit soundtracks and dynamic sound feedback.
* **Input Interface: Tactile Switches (6x6x4.3mm Push Buttons)**
    * Three strategically placed buttons for menu navigation (Select, Confirm, Cancel), with internal pull-up resistors activated on the microcontroller.
* **Power & Battery Management System:**
    * **Battery:** 3.7V LiPo cell (approx. 300mAh - 500mAh).
    * **Charging Module:** TP4056 board with USB-C interface for safe lithium cell charging.
    * **Slide Switch:** Hardware-level power cut control.

## Physical Design and Assembly

The project's evolution includes the transition from a breadboard circuit to a portable final product. The casing was designed considering ergonomics and the space constraints of electronic components.

The physical prototyping and manufacturing of the final case can be done through digital fabrication techniques such as 3D printing (leveraging printing services and maker spaces available in Aracaju and the region) or through *retrofitting* techniques, adapting the internal circuit to obsolete classic device casings.

## Roadmap and Future Features

- [x] Prototyping state logic and rendering on the OLED matrix (Fixed Timestep & HAL implemented).
- [x] Implementation of hardware-abstracted input system and debouncing (IInput HAL).
- [x] Implementation of Pet FSM, statistics decay system, and HUD rendering.
- [ ] Implementation of audio system (Buzzer PWM).
- [ ] Energy consumption optimization (ESP32 Sleep Modes).
- [ ] Integration of final hardware into the case.
- [ ] **Multiplayer via ESP-NOW:** Implementation of low-latency peer-to-peer communication for interaction between two nearby physical devices.
- [ ] **Educational Integration:** Implementation of advanced custom animations, including an experimental module for the pet to teach basic sign language (Libras) to the user.

## Development Environment Setup (Getting Started)

This section covers the complete setup for a clean Windows machine. Follow each step in the order indicated.

### Prerequisites Overview

| Tool | Purpose |
|---|---|
| VS Code | IDE |
| PlatformIO IDE Extension | Build system, library manager, uploader |
| MSYS2 + MinGW-w64 | C++ compiler toolchain for the `native` (PC) build |
| Raylib (via MSYS2) | Graphics library for the debug build on PC |

---

### Step 1 — Install VS Code

1. Go to **code.visualstudio.com** and download the **Windows x64** installer.
2. Run the installer. During setup, check **"Add to PATH"** and **"Register Code as an editor for supported file types"**.
3. Verify the installation by opening a new terminal (`Win + R` -> `cmd`) and running:
   ```
   code --version
   ```
   It should display a version number (e.g., `1.89.x`).

---

### Step 2 — Install VS Code Extensions

This repository includes a `.vscode/extensions.json` file with all required extensions listed. When you open the project folder in VS Code for the first time, a popup will automatically appear asking if you want to install the recommended extensions. Click **"Install All"**.

If the popup does not appear, install them manually via `Ctrl + Shift + X`. The required extensions and their exact IDs are:

| Extension | Publisher | ID |
|---|---|---|
| PlatformIO IDE | PlatformIO | `platformio.platformio-ide` |
| C/C++ | Microsoft | `ms-vscode.cpptools` |
| C/C++ Extension Pack | Microsoft | `ms-vscode.cpptools-extension-pack` |
| C/C++ Themes | Microsoft | `ms-vscode.cpptools-themes` |
| CMake Tools | Microsoft | `ms-vscode.cmake-tools` |
| Makefile Tools | Microsoft | `ms-vscode.makefile-tools` |
| clangd | LLVM | `llvm-vs-code-extensions.vscode-clangd` |

> **WARNING — PlatformIO:** There are third-party extensions with similar names published by unofficial authors. Install **only** the extension with ID `platformio.platformio-ide` (publisher: **PlatformIO**). Any other will fail to install PlatformIO Core and the `pio` command will not be available. Always check the extension ID before clicking Install.

After installing PlatformIO IDE, VS Code will reload. A PlatformIO icon (alien head) should appear in the left sidebar. Click it and wait for the home screen to load.

> **Note:** PlatformIO manages its own Python environment internally. There is no need to install Python separately.

---

### Step 2.5 — Add PlatformIO CLI (`pio`) to System PATH

The PlatformIO extension installs the `pio` executable in a local user folder that is **not** automatically added to the system PATH. Without this step, running `pio` in PowerShell or Command Prompt will fail with "command not recognized", even if the extension works inside VS Code.

**Verify if the executable exists:**
```powershell
Test-Path "C:\Users\$env:USERNAME\.platformio\penv\Scripts\pio.exe"
```
If it returns `True`, proceed. If it returns `False`, open VS Code, wait for PlatformIO to finish initial configuration (progress bar in the bottom status bar), and check again.

**Add to system PATH:**
1. Open **Start** -> search for **"Edit the system environment variables"**.
2. Click **"Environment Variables..."**.
3. Under **System variables**, select `Path` -> click **Edit** -> click **New**.
4. Add the path below (replace `felip` with your Windows username):
   ```
   C:\Users\felip\.platformio\penv\Scripts
   ```
5. Click **OK** on all dialogs.

**Open a new PowerShell window** (PATH changes only apply to new terminals) and verify:
```
pio --version
```
Expected output: `PlatformIO Core, version 6.x.x`

> **Note:** `pio` commands can also be run from the integrated VS Code terminal (`Ctrl + ` `) without this PATH change, as VS Code configures its own terminal environment. The PATH step is only necessary for running `pio` in standalone PowerShell or Command Prompt windows.

---

### Step 3 — Install MSYS2 and the MinGW-w64 Toolchain

MSYS2 provides the GCC/G++ compiler needed to build and run the `native` (PC) environment.

1. Go to **msys2.org** and download the installer (`msys2-x86_64-YYYYMMDD.exe`).
2. Run the installer. Accept the default installation path: `C:\msys64`.
3. After installation, open the **"MSYS2 MINGW64"** shell (not MSYS2 MSYS).
4. Update the package database:
   ```bash
   pacman -Syu
   ```
   The terminal will close automatically. Reopen the MINGW64 shell and run:
   ```bash
   pacman -Su
   ```
5. Install the MinGW-w64 GCC toolchain:
   ```bash
   pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb make
   ```
6. Add MinGW-w64 to the Windows system PATH:
   - Open **Start** -> search for **"Edit the system environment variables"**.
   - Click **"Environment Variables..."**.
   - Under **System variables**, select `Path` and click **Edit**.
   - Click **New** and add: `C:\msys64\mingw64\bin`
   - Click **OK** on all dialogs.
7. Verify in a **new** Command Prompt or PowerShell window:
   ```
   g++ --version
   ```
   Expected output: `g++.exe (Rev..., Built by MSYS2 project) 14.x.x`

---

### Step 4 — Install Raylib via MSYS2

Raylib is the graphics library used by the `native` build to simulate the OLED display on the PC.

1. Open the **MSYS2 MINGW64** shell.
2. Run:
   ```bash
   pacman -S mingw-w64-x86_64-raylib
   ```
3. Verify the installation:
   ```bash
   ls /mingw64/include/raylib.h
   ls /mingw64/lib/libraylib.a
   ```
   Both files should exist. If they do, Raylib is ready. This repository's `platformio.ini` is already pre-configured to find these paths at `C:/msys64/mingw64/`.

---

### Step 5 — Clone the Repository and Open in VS Code

```bash
git clone https://github.com/jovinull/bixim.git
cd bixim
code .
```

VS Code will open the project. PlatformIO will detect `platformio.ini` and index the project automatically (may take 1-2 minutes on first opening).

---

### Step 6 — Build and Execution: Native Environment (PC)

The `native` environment compiles the project as a standard Windows executable using Raylib for rendering. Use this target for all logic development and debugging before flashing to hardware.

**Via VS Code:**
- Click the PlatformIO icon in the sidebar.
- Under **Project Tasks -> native**, click **Build** and then **Upload and Monitor** (which executes the binary).

**Via terminal (in project root):**
```bash
# Just compile
pio run -e native

# Compile and execute the binary
pio run -e native -t exec
```

A window titled **"Bixim - Native Debug Build"** should open.

---

### Step 7 — Build and Flash: ESP32 Environment

1. Connect your ESP32 board via USB.
2. Identify the COM port: open **Device Manager** (`Win + X` -> Device Manager) -> expand **Ports (COM and LPT)** -> note the port (e.g., `COM3`).
3. In VS Code, PlatformIO detects the port automatically. If not detected, add `upload_port = COMX` to the `[env:esp32dev]` section in `platformio.ini`.

**Via VS Code:**
- Under **Project Tasks -> esp32dev**, click **Upload**.

**Via terminal:**
```bash
# Compile and flash
pio run -e esp32dev -t upload

# Open serial monitor (115200 baud) after flashing
pio device monitor
```

Expected serial output:
```
[Bixim] Booting...
[Bixim] Display OK. Boot complete.
```

---

### Project Directory Structure

```
bixim/
├── platformio.ini      # Build system configuration (all environments)
├── src/
│   └── main.cpp        # Unified entry point with #ifdef guards per platform
├── include/            # Shared header files (.h / .hpp)
├── lib/                # Local libraries (project-specific, not from registry)
├── hardware/           # Schematics, KiCad files, BOM
├── docs/               # Technical milestones and architecture documentation
├── README.md
└── README.pt-br.md
```

---

## How to Run the Project

1. Clone the repository: `git clone https://github.com/jovinull/bixim.git`
2. Follow the **Development Environment Setup** section above.
3. Build for PC: `pio run -e native -t exec`
4. Build and flash for ESP32: `pio run -e esp32dev -t upload`
5. Follow the electrical schematic in the `/hardware` directory for peripheral assembly.

## License

This project is open source and licensed under the MIT License. Feel free to fork, study the architecture, and contribute improvements.