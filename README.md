# Bixim: ESP32-Based Virtual Pet Embedded System

[English](README.md) | [Português](README.pt-br.md)

## Project Overview
The **Bixim** project (provisional/code name) is a complete implementation (hardware and software) of a portable digital entertainment device, inspired by the classic virtual pet architecture of 1996.

Unlike traditional emulators that rely on proprietary ROM dumps, this project uses a **High-Level Emulation (HLE) and Custom Logic** approach. This ensures total independence from third-party intellectual property, resulting in a 100% original work that demonstrates proficiency in reverse engineering concepts, embedded systems development, and hardware design.

## Software Architecture and Technology Choice
The system core was developed entirely in **C/C++**. The choice of this language is justified by the following factors:

1. **Memory Management and Performance:** Bare-metal or RTOS-based development on the ESP32 requires strict control over memory allocation and CPU cycles. C/C++ provides the granularity needed to ensure a stable frame rate (FPS) on the OLED display, without the bottlenecks of interpreted languages.
2. **Energy Efficiency:** Natively compiled code minimizes processor active time, allowing for more efficient use of ESP32 *deep sleep* strategies, which is critical for a device powered by a small LiPo battery.
3. **Hardware Interaction:** Direct manipulation of registers, interrupts (ISRs) for button reading, and pulse-width modulation (PWM) control for the buzzer are implemented with maximum efficiency using native C/C++ libraries.

### State Logic (Custom Game Engine)
The internal engine does not emulate the original Epson S1C63 processor. Instead, it implements a finite state machine (FSM) that simulates the system's organic cycle:
* **Life Module:** Manages temporal decay variables (Hunger, Sleep, Happiness, Age).
* **Rendering Module:** Transforms bit matrices into I2C signals for OLED display updates.
* **Interrupt Module:** Handles hardware/software debouncing of physical buttons to ensure precise user responses.

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
- [ ] Prototyping state logic and rendering on the OLED matrix.
- [ ] Implementation of debouncing system and audio PWM control.
- [ ] Energy consumption optimization (ESP32 Sleep Modes).
- [ ] Integration of final hardware into the case.
- [ ] **Multiplayer via ESP-NOW:** Implementation of low-latency peer-to-peer communication for interaction between two nearby physical devices.
- [ ] **Educational Integration:** Implementation of advanced custom animations, including an experimental module for the pet to teach basic sign language (Libras) to the user.

## How to Run the Project
1. Clone the repository: `git clone https://github.com/jovinull/bixim.git`
2. Set up the development environment: VS Code with the PlatformIO extension is recommended.
3. Install the dependencies listed in the `platformio.ini` file (e.g., Adafruit SSD1306 and GFX libraries).
4. Compile and upload to your ESP32 module.
5. Follow the detailed electrical schematic in the `/hardware` directory for peripheral assembly.

## License
This project is open source and licensed under the MIT License. Feel free to fork, study the architecture, and contribute improvements.