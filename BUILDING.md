# Building Qt_Chess

## Prerequisites

### Required Qt Modules
- Qt5 Core
- Qt5 GUI
- Qt5 Widgets
- Qt5 Multimedia
- Qt5 MultimediaWidgets
- Qt5 Network
- **Qt5 WebSockets** (required for online mode)

### Installing Qt WebSockets

The online chess mode requires the Qt WebSockets module, which may not be included by default in your Qt installation.

#### Ubuntu / Debian
```bash
sudo apt-get update
sudo apt-get install libqt5websockets5-dev
```

#### Fedora / RHEL / CentOS
```bash
sudo dnf install qt5-qtwebsockets-devel
```

#### Arch Linux
```bash
sudo pacman -S qt5-websockets
```

#### macOS (with Homebrew)
```bash
brew install qt5
```
Note: The Homebrew Qt5 package includes WebSockets by default.

#### Windows
1. Open **Qt Maintenance Tool**
2. Select **Add or remove components**
3. Navigate to your Qt version (e.g., Qt 5.15.2)
4. Expand the tree and check **Qt WebSockets**
5. Click **Next** and follow the installation prompts

## Building

### Using qmake
```bash
qmake Qt_Chess.pro
make
```

### Using Qt Creator
1. Open `Qt_Chess.pro` in Qt Creator
2. Configure the project with your Qt kit
3. Build the project (Ctrl+B or Cmd+B)

## Troubleshooting

### Error: "Unknown module(s) in QT: websockets"

This error occurs when the Qt WebSockets module is not installed. Follow the installation instructions above for your platform.

After installing the WebSockets module:
1. Clean your build directory: `make clean` or delete the build folder
2. Re-run qmake: `qmake Qt_Chess.pro`
3. Build again: `make`

### Verifying Qt WebSockets Installation

To check if Qt WebSockets is installed, run:
```bash
# Linux/macOS
find /usr -name "QtWebSockets" -o -name "libQt5WebSockets*" 2>/dev/null

# Or check with qmake
qmake -query
```

Look for paths containing "websockets" or "WebSockets" in the output.

## Running

After successful compilation:
```bash
./Qt_Chess
```

## Notes

- **Online Mode**: Requires connection to the WebSocket server at `wss://chess-server-mjg6.onrender.com`
- **Stockfish Engine**: Place the Stockfish binary in the `engine/` directory for AI mode
- **C++17**: The project requires a C++17 compatible compiler

## Additional Dependencies

### Stockfish (Optional - for AI mode)
Download Stockfish from https://stockfishchess.org/download/ and place the binary in the `engine/` directory.

### Sound Files
Sound files should be automatically included via the resources file. If sounds don't work, check that `resources.qrc` is properly compiled.

## Platform-Specific Notes

### Linux
- Ensure Qt5 development packages are installed
- May need to install additional multimedia codecs for sound support

### macOS
- Use Homebrew to install Qt5: `brew install qt5`
- Add Qt5 to PATH: `export PATH="/usr/local/opt/qt@5/bin:$PATH"`

### Windows
- Install Qt5 from the official Qt installer
- Use MinGW or MSVC compiler
- Ensure Qt bin directory is in your PATH

## Getting Help

If you encounter build issues:
1. Check that all prerequisites are installed
2. Verify your Qt version is 5.x or later
3. Clean and rebuild the project
4. Check the error messages for missing dependencies

For online mode issues, see [docs/ONLINE_MODE_FEATURE.md](docs/ONLINE_MODE_FEATURE.md).
