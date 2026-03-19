# NullNote

Minimal Qt memo app for Windows.

Features:

- Single memo surface only
- No save/load behavior; each launch starts with a blank note
- Click `http://` or `https://` URLs to open them in the browser
- Click Windows paths to open them in Explorer
- Click Linux-style absolute paths to resolve them through WSL and open the result in Explorer

## Build

Local release build:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.7.2\msvc2019_64"
cmake --build build --config Release
cpack --config build/CPackConfig.cmake -C Release
```

The generated ZIP is written to the repository root and includes `NullNote.exe`, deployed Qt runtime files, `README.md`, `LICENSE`, and `THIRD_PARTY_NOTICES.txt`.

## Icon Asset

The app uses `assets/icon.png` for the in-app icon and `assets/icon.ico` for the Windows executable icon.

To regenerate the multi-size `.ico` in this environment, use `uv` instead of a bare `python`:

```powershell
./scripts/generate_icon.ps1
```

## GitHub Actions

Pushing a tag matching `v*` (for example `v1.0.0`) triggers `.github/workflows/release-zip.yml` and publishes the ZIP as both a workflow artifact and a GitHub Release asset.
