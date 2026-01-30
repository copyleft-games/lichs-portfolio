# Lich's Portfolio

An idle/incremental strategy game where you play as Malachar the Undying, a lich who discovered that the true path to power isn't conquering kingdoms—it's *owning* them through compound interest.

Slumber for decades, wake to fortunes or ruin, and build an economic empire that outlasts civilizations.

## Features

- **Centuries-Scale Gameplay:** Think in decades, invest for centuries
- **Deep Investment System:** Real estate, trade routes, bonds, magical assets, and darker options
- **Agent Networks:** Cultivate mortal servants, bloodline dynasties, and cults
- **World Simulation:** Kingdoms rise and fall, creating opportunities and risks
- **Prestige System:** When your phylactery falls, rise again with echoes of past lives
- **Dark Humor:** Malachar's sardonic commentary on mortal folly

## Build Requirements

### Fedora / RHEL

```bash
sudo dnf install gcc make pkg-config
sudo dnf install glib2-devel gobject-introspection-devel
sudo dnf install libdex-devel libyaml-devel json-glib-devel
```

### Ubuntu / Debian

```bash
sudo apt install gcc make pkg-config
sudo apt install libglib2.0-dev libgirepository1.0-dev
sudo apt install libdex-dev libyaml-dev libjson-glib-dev
```

## Building

```bash
# Build everything (dependencies + game)
make

# Debug build with symbols
make DEBUG=1

# Run tests
make test

# Clean build artifacts
make clean
```

### Cross-Compilation for Windows

Requires MinGW-w64 toolchain:

```bash
# Fedora
sudo dnf install mingw64-gcc mingw64-glib2 mingw64-pkg-config

# Build for Windows
make WINDOWS=1
```

### Steam Integration (Optional)

The game is fully playable without Steam. Steam provides optional enhancements:
cloud saves, achievement sync, and leaderboards.

```bash
# Download Steamworks SDK and place in deps/steamworks/
make STEAM=1
```

## Running

```bash
# After building
./build/release/lichs-portfolio

# Debug build
./build/debug/lichs-portfolio
```

## Project Structure

```
lichs-portfolio/
├── src/                # Game source code
│   ├── core/           # Application, game data, progression
│   ├── simulation/     # World, kingdoms, events
│   ├── investment/     # Investment types and portfolio
│   ├── agent/          # Agent management
│   ├── ui/             # UI screens and dialogs
│   └── states/         # Game states
├── data/               # YAML data files
├── assets/             # Textures, fonts, audio
├── tests/              # GTest unit tests
├── docs/               # Documentation
├── design/             # Game design documents
│   ├── GAME.md         # Full game design document
│   └── PLAN.md         # Technical implementation plan
└── deps/
    └── libregnum/      # Game engine (submodule)
```

## Documentation

- [Game Design Document](design/GAME.md) - Full game design and narrative
- [Development Plan](design/PLAN.md) - Technical implementation phases
- [Architecture Overview](docs/architecture.md) - System design

## Controls

### Mouse
- Click to select and interact
- Drag sliders for values
- Hover for tooltips

### Keyboard
- **Tab / Shift+Tab:** Navigate between elements
- **Arrow Keys:** Navigate lists and options
- **Enter:** Select / Confirm
- **Escape:** Back / Cancel
- **1-4:** Quick select choices in dialogs

## Configuration

Settings are saved to:
- Linux: `~/.config/lichs-portfolio/settings.yaml`
- Windows: `%APPDATA%/lichs-portfolio/settings.yaml`

Save games are stored in:
- Linux: `~/.local/share/lichs-portfolio/saves/`
- Windows: `%APPDATA%/lichs-portfolio/saves/`

## MCP Server (AI Debugging Integration)

The game can expose an MCP (Model Context Protocol) server for AI-assisted debugging
and control. This allows external tools like Claude Code, Clawdbot, or custom AI agents
to interact with the running game.

### Building with MCP Support

```bash
# Build with MCP server enabled
make MCP=1

# Or with debug + MCP
make DEBUG=1 MCP=1
```

This requires libregnum to also be built with MCP support (handled automatically).

### Running with MCP

When built with `MCP=1`, the game starts an HTTP server for MCP connections:

```bash
# Default port: 5005
./build/release/lichs-portfolio

# Custom port via environment variable
MCP_HTTP_PORT=8080 ./build/release/lichs-portfolio
```

### Connecting to the MCP Server

Use `mcp-remote-client` from mcp-glib to bridge stdio MCP clients to the HTTP server:

```bash
# Connect via mcp-remote-client
mcp-remote-client --http http://localhost:5005/mcp
```

### Claude Desktop Configuration

Add to your Claude Desktop config (`~/.config/claude/claude_desktop_config.json`):

```json
{
  "mcpServers": {
    "lichs-portfolio": {
      "command": "mcp-remote-client",
      "args": ["--http", "http://localhost:5005/mcp"]
    }
  }
}
```

### Clawdbot Configuration

Add to your Clawdbot MCP servers config:

```yaml
mcp:
  servers:
    lichs-portfolio:
      command: mcp-remote-client
      args:
        - --http
        - http://localhost:5005/mcp
```

### Available MCP Tools

When connected, the following tools are available:

**Input Tools:**
- `press_key`, `release_key`, `tap_key` - Keyboard control
- `press_mouse_button`, `release_mouse_button` - Mouse buttons
- `move_mouse_to`, `move_mouse_by` - Mouse movement
- `press_gamepad_button`, `release_gamepad_button` - Gamepad buttons
- `set_gamepad_axis` - Gamepad analog sticks

**Engine Tools:**
- `get_engine_info` - Engine state information
- `pause_game`, `resume_game` - Control game loop
- `take_screenshot` - Capture game screen

**Debug Tools:**
- `log_message` - Write to game log
- `toggle_profiler` - Performance profiling

## Dependencies

This project uses:

- [libregnum](deps/libregnum/) - GObject-based game engine
- [graylib](deps/libregnum/deps/graylib/) - GObject wrapper for raylib
- [yaml-glib](deps/libregnum/deps/yaml-glib/) - YAML parsing with GObject serialization
- [GLib/GObject](https://docs.gtk.org/glib/) - Core utilities and object system

## License

This project is licensed under the **GNU Affero General Public License v3.0 or later** (AGPL-3.0-or-later).

See [COPYING](COPYING) for the full license text.

## Contributing

Contributions are welcome. Please ensure:

1. Code follows the existing style (see CLAUDE.md for conventions)
2. All tests pass (`make test`)
3. New features include appropriate documentation
4. Commits are signed off

## Credits

- Game Design & Development: Zach Podbielniak
- Engine: libregnum
- Inspired by: Cookie Clicker, Universal Paperclips, Victoria series
