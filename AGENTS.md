# AGENTS.md — AI Agent Guidance for ScreenTranslator

> **Repository:** `Max97k/ScreenTranslator`  
> **Default Branch:** `master`  
> **Primary Technology Stack:** C++17, Qt5 (Core, Gui, Widgets, Network, TestLib), WinRT OCR API, Hunspell, Miniz, Google Test, Python 3 (CI Tooling)  
> **Visibility:** Public  

---

## 1. Project Overview & Architecture

### 1.1 Purpose & Mission
ScreenTranslator is a modernized, high-performance screen capture, Optical Character Recognition (OCR), spell check, and multi-engine machine translation tool tailored for Windows 10/11 (with cross-platform Linux support). It runs in the background system tray, listens for customizable global hotkeys, provides an interactive screen region selector, recognizes on-screen text via native OS APIs, corrects typos via Hunspell, and displays translated text in lightweight floating overlays or a rich text editor.

### 1.2 System Architecture & Component Diagram
The system operates as a modular, event-driven pipeline:
1. **Screen Capture Engine (`src/capture/`)**: Captures selected screen regions using native DirectX/DXGI, Direct3D 11, and Windows GDI APIs (`capturer.cpp`, `captureareaselector.cpp`, `capturearea.cpp`). Supports multi-monitor setups and DPI-aware coordinate mapping.
2. **OCR Engine (`src/ocr/`)**: Integrates Windows 10/11 WinRT Native OCR (`Windows.Media.Ocr.OcrEngine` via `winocr.cpp`) running asynchronously inside background worker threads (`recognizerworker.cpp`).
3. **Text Correction Engine (`src/correct/`)**: Post-processes OCR text using Hunspell spell checking (`hunspellcorrector.cpp`) and custom user substitution tables (`substitutionstable.cpp`) to fix OCR misidentifications.
4. **Translation Coordinator (`src/translate/`, `translators/`)**: Coordinates translation requests across modular JavaScript translation scripts (`translators/google.js`, `google_api.js`, `deepl.js`, `bing.js`, `baidu.js`, `papago.js`, `yandex.js`) and native REST APIs.
5. **Presentation & UI (`src/represent/`, `src/service/`)**: Renders non-intrusive floating translation overlays (`resultwidget.cpp`), full result editors (`resulteditor.cpp`), system tray icon (`trayicon.cpp`), settings configuration (`settingseditor.cpp`), and updates manager (`updates.cpp`).
6. **System Integration (`src/service/`)**: Handles global Windows hotkeys (`globalaction.cpp`), single-instance application control (`singleapplication.cpp`), and Windows autostart configuration (`runatsystemstart.cpp`).

### 1.3 Key File & Directory Map
| Path | Purpose / Description |
|---|---|
| `screen-translator.pro` | Master QMake project file defining compiler flags, Qt modules, and platform libraries |
| `src/main.cpp` | Application entrypoint, single-instance verification, and event loop initialization |
| `src/manager.cpp` | Central coordinator linking capture, OCR, correction, translation, and UI modules |
| `src/capture/` | Screen capture pipeline, region selection overlay, and coordinate transformation |
| `src/ocr/` | WinRT Native OCR implementation (`winocr.cpp`) and asynchronous worker thread (`recognizerworker.cpp`) |
| `src/correct/` | Spell checking via Hunspell (`hunspellcorrector.cpp`) and substitution tables |
| `src/translate/` | Translation manager (`translator.cpp`) and network request dispatch |
| `src/represent/` | Floating result widget (`resultwidget.cpp`) and editor dialog (`resulteditor.cpp`) |
| `src/service/` | Global hotkeys, single application lock, Windows startup registration, auto-updater |
| `translators/` | JavaScript translation engine adapters (Google, DeepL, Bing, Baidu, Papago, Yandex) |
| `external/` | Embedded third-party dependencies (Google Test, Miniz) |
| `tests/` | Unit and integration test suite (`tests.pro`, `capturearea_test.cpp`, `translation_pipeline_test.cpp`, etc.) |
| `share/ci/` | Python automation scripts for building, testing, and packaging releases (`build.py`, `test.py`, `release.py`) |
| `docs/translation_apis.md` | Documentation for configuring external translation API keys |

---

## 2. Development, Build & Verification Commands

### 2.1 Prerequisites & Environment Setup
- **Compiler / IDE**: Microsoft Visual Studio 2022 (MSVC v143 toolset) or GCC on Linux.
- **Qt Framework**: Qt 5.15+ with modules `core`, `gui`, `widgets`, `network`, and `testlib`.
- **Windows SDK**: Windows 10/11 SDK providing WinRT headers and libraries (`windowsapp.lib`, `d3d11.lib`, `dxgi.lib`, `User32.lib`).
- **Build Tools**: Python 3.10+ (for CI scripts), QMake, `nmake` / `jom` / `make`.

### 2.2 Build & Compilation Commands
```powershell```
# Automated CI Build (Windows MSVC x64)
$env:OS="win64"
python share/ci/build.py

# Direct QMake & NMake compilation (Release mode)
qmake screen-translator.pro CONFIG+=release
nmake

# Automated Release Packaging (creates standalone zip / installer)
$env:OS="win64"
python share/ci/release.py
``````

### 2.3 Verification & Testing Suite
```powershell```
# Automated Test Execution via Python CI Runner
python share/ci/test.py

# Direct Test Compilation and Execution
cd tests
qmake tests.pro CONFIG+=debug
nmake
.\debug\tests.exe
``````

### 2.4 Code Style & Linting
```powershell```
# Clang-Format check & apply
clang-format -i src/**/*.cpp src/**/*.h tests/**/*.cpp

# Uncrustify style enforcement (using repository configuration)
uncrustify -c share/uncrustify.cfg --replace --no-backup src/*.cpp src/**/*.cpp
``````

### 2.5 Clean & Reset
```powershell```
# Clean build artifacts
nmake distclean
# Or clean git working tree
git clean -fdx
``````

---

## 3. Coding Standards & Conventions

### 3.1 Code Style & Idioms
- **C++ Standard**: C++17 standard (`CONFIG += c++17`). Use modern C++ idioms (`std::optional`, `std::string_view`, structured bindings, range-based for loops).
- **Qt Conventions**: Prefer new-style `QObject::connect` with member function pointers over string-based `SIGNAL()`/`SLOT()` macros for compile-time type checking.
- **Memory Management & RAII**: Leverage Qt parent-child object hierarchy for automatic `QObject` / `QWidget` lifecycle management. Use `std::unique_ptr` and `std::shared_ptr` for non-Qt dynamic allocations. Avoid manual `delete` calls.
- **Formatting**: Strictly follow the `.clang-format` configuration defined at the repository root.

### 3.2 File & Module Organization
- Header files (`.h`) and implementation files (`.cpp`) are co-located within module subdirectories (`src/capture/`, `src/ocr/`, `src/correct/`, `src/translate/`, `src/represent/`, `src/service/`).
- Forward declarations in headers are preferred over heavy includes; use `src/stfwd.h` for common forward declarations.
- Tests are organized in the `tests/` directory with `_test.cpp` suffixes and execute through the Google Test framework linked with Qt TestLib.

### 3.3 State Management & Error Handling
- **Non-blocking UI**: OCR recognition and network translation requests MUST execute asynchronously inside worker threads (`recognizerworker.cpp`, `correctorworker.cpp`) using Qt signal/slot message passing to keep the GUI thread at 60fps.
- **Error Propagation**: Log diagnostics via `src/service/debug.h`. Handle network failures gracefully by notifying `resultwidget.cpp` without crashing or freezing the application.

---

## 4. Safety, Security & Resource Constraints

### 4.1 Secrets & Environment Management
- **API Keys**: External translation API keys (e.g. Google Cloud Translation API) must be entered by users via the application Settings dialog or stored in user configuration. Never commit hardcoded API keys, bearer tokens, or secrets to the codebase.

### 4.2 Resource, Hardware & Platform Constraints
- **DirectX / GDI Handles**: Screen capture routines must explicitly release Direct3D 11 device contexts, DXGI surfaces, and GDI device contexts immediately after frame acquisition to prevent Windows GDI handle exhaustion.
- **WinRT OCR Prerequisites**: WinRT OCR relies on Windows OS native language packs. Ensure proper error handling and descriptive UI hints if a user selects a language pack not installed in Windows Settings.

---

## 5. Git & Branch Workflow

- **Target Default Branch**: `master` *(Note: ScreenTranslator uses `master` as its primary branch)*.
- **Commit Format**: Use atomic commits with Conventional Commit prefixes:
  - `feat:` New user-facing feature or enhancement
  - `fix:` Bug fix or error resolution
  - `refactor:` Code refactoring without behavioral changes
  - `docs:` Documentation updates
  - `test:` Adding or improving test cases
  - `ci:` Build scripts or GitHub Actions workflow changes
