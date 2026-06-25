# Screen Translator (v4.0.0)

**A modernized, lightweight, and fast fork of ScreenTranslator designed for Windows 10 & 11.**

## Introduction

This software allows you to translate any text on screen. It is a powerful combination of screen capture, OCR (Optical Character Recognition), and translation tools.

Unlike the original version, **v4.0.0** has been modernized to eliminate heavy and obsolete third-party dependencies:
*   **Windows Native OCR (WinRT)**: Replaced Tesseract OCR. There is no need to download or manage massive `.traineddata` files. It runs instantly using the Windows built-in character recognition engine.
*   **Native Google Cloud Translation API**: Replaced the heavy `QWebEngine` browser backend and Javascript scraper scripts. Network requests are handled natively via C++ for maximum speed, security, and stability.

---

## Installation

### Windows (v4.0.0+)
1. Download the latest release package (`ScreenTranslator-4.0.0-win64.zip`) from the releases page.
2. Extract the archive to any folder.
3. Run `screen-translator.exe`.

*Note: If the application complains about missing VC++ runtimes, install or repair `vc_redist.x64.exe` included in the folder.*

---

## Setup & Configuration

The app runs entirely in the background and only shows a system tray icon.

### 1. Configure the Translation API
Since version 4.0.0 utilizes the official Google Cloud Translation API, you need to provide a Google Cloud API Key:
1. Right-click the ScreenTranslator icon in the system tray and select **Settings**.
2. Go to the **Translation** tab on the left.
3. Paste your key into the **Google Cloud API Key** input box.
4. Ensure the **Do translation** checkbox at the top is checked.
5. Click **OK** to save.

### 2. Adding OCR Languages
Because the software uses Windows Native OCR, you do not download language packs inside the app. Instead, you manage them directly through Windows Settings:
1. Open Windows **Settings** -> **Time & language** -> **Language & region**.
2. Click **Add a language** and search for the language you wish to recognize (e.g. Japanese, French).
3. Make sure to check **Basic typing / Optical Character Recognition (OCR)** during installation.
4. Once Windows completes the download, restart ScreenTranslator. The language will automatically appear under **Settings** -> **Recognition** tab.

---

## Usage

1. Start the program (it runs quietly in the system tray).
2. Press the capture hotkey (default is `Ctrl + Alt + Z`).
3. Select the region on screen to recognize.
4. The translated text will appear on screen.

---

## Dependencies

*   [Qt 5 (Widgets, Network, Testlib)](https://qt-project.org/)
*   Windows 10/11 Native OCR (WinRT APIs)
*   [Hunspell](https://github.com/hunspell/hunspell) (For spelling correction)

---

## Build from Source

Look at the scripts (python3) in the `share/ci` folder:
1. Set the environment variable `$env:OS="win64"` (or `win32`).
2. Run `python share/ci/release.py` to build and package.
