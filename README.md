# acrylic-window

**acrylic-window** is a lightweight command-line tool for applying modern window effects like **acrylic blur**, **transparency**, and **rounded corners** to any open window on Windows 10 and 11. It uses the `SetWindowCompositionAttribute` and `DwmSetWindowAttribute` APIs to dynamically customize existing windows.

## ✨ Features

- Apply **Acrylic** or **Blur** effects
- Customize **corner styles**: none, round, or roundsmall
- Control **opacity** and **tint color**
- Toggle **border visibility**
- Set **border color** (Windows 11+)
- Works with any window by its title

## ⚙️ Requirements

- Windows 10 or 11 (Build 1809+ for acrylic, 22000+ for border color)
- C++17 compiler
- Admin rights may be required depending on the target window

## 📦 Build Instructions

```bash
git clone https://github.com/nstechbytes/acrylic-window.git
cd acrylic-window
cl /EHsc acrylic_window.cpp /link dwmapi.lib
````

> Replace `cl` with your preferred C++ compiler if you're not using MSVC.

## 🧪 Usage

```bash
acrylic_window.exe -type "blur|acrylic" -corner "none|round|roundsmall" -title "Window Title"
                  [-opacity 0-255] [-tintColor RRGGBB]
                  [-borderVisible true|false] [-borderColor RRGGBB]
```

### 🔍 Examples

```bash
# Apply blur with 50% opacity and red tint to Notepad
acrylic_window.exe -type blur -corner none -title "Untitled - Notepad" -opacity 128 -tintColor FF0000

# Apply acrylic with small rounded corners and a green border (Windows 11)
acrylic_window.exe -type acrylic -corner roundsmall -title "MyApp" -borderColor 00FF88
```

## 📘 Parameters

| Option           | Description                                                    |
| ---------------- | -------------------------------------------------------------- |
| `-type`          | Effect type: `blur` or `acrylic`                               |
| `-corner`        | Window corner style: `none`, `round`, `roundsmall`             |
| `-title`         | Window title (case-sensitive)                                  |
| `-opacity`       | Tint opacity (0–255). Default: `204`                           |
| `-tintColor`     | Tint color in hex `RRGGBB`. Default: `FFFFFF`                  |
| `-borderVisible` | Show or hide window border: `true` or `false`. Default: `true` |
| `-borderColor`   | Border color in hex `RRGGBB`. Default: `FFFFFF`                |

## 🛠 Known Limitations

* Only works with windows that have a standard Win32 window handle.
* Effects may not be visible on some UWP or sandboxed apps.
* Corner rounding and border color require Windows 11 (build 22000+).

## 📄 License

[MIT](LICENSE)

---

Made with ❤️ for Windows customization enthusiasts.
