# Operational Requirements

## Tech Inspirations

### Game Boy Advance

#### Processor & Architecture

- 32‑bit ARM7TDMI RISC CPU
    - ~16.78 MHz clock speed
    - 16 general-purpose registers
    - 32-bit data bus & ALU
    - 32-bit addressing
    - No MMU & cache
        - Utilizes a ROM pre-fetcher buffer instead
- 16 KiB BIOS ROM
    - Address: `0x0000_0000`—`0x0000_3FFF`
    - Plays startup animation
    - Bootloader
    - Hardware interrupt vectors
    - Arithmetic functions
    - Affine matrix calculator

#### Memory

- 256 KiB 16-bit External Work RAM (on-board)
    - Address: `0x0200_0000`—`0x0203_FFFF`
    - Houses 16-bit THUMB instructions
- 32 KiB 32-bit Internal Work RAM (on-chip)
    - Address: `0x0300_0000`–`0x0300_7FFF`
    - Houses 32-bit ARM instructions
- 96 KiB 16-bit VRAM
    - Address: `0x0600_0000`—`0x0601_7FFF`
    - 64 KiB for backgrounds
    - 32 KiB for sprites

#### Display & Graphics

- 240 × 160 px
- ~59.73 Hz

##### Pixel Processing Unit

- 1 KiB Palette RAM
    - Address: `0x0500_0000`—`0x0500_03FF`
        - Backgrounds: `0x0500_0000`—`0x0500_01FF`
        - Sprites: `0x0500_0200`—`0x0500_03FF`
    - 256 15‑bit colors (each for backgrounds and sprites)
        - 16 colors per 16 palettes
        - XBGR555 encoding
- 1 KiB Object Attribute Memory
    - Address: `0x0700_0000`—`0x0700_03FF`
    - 128 sprite entries

- Must vait for VBlank / HBlank

#### Game Pak

- 32 MB ROM

### _Pokémon Emerald Version_

- Top-down overworld graphics
- 16 × 16 grid blocks
- 64 × 64 battle sprites

## Target Architectures

### RaspberryPi Pico 2

#### Processor

- ~150 MHz clock speed

- Dual 32-bit Arm Cortex-M33
    - 16 general-purpose registers
    - SIMD instructions
    - Single-precision FPU

OR

- Dual 32-bit Hazard3 RISC-V

#### Memory

- 520 KiB on-chip SRAM

- 4 MiB on-board QSPI flash

### Display

320 × 240 px viewport

Requires:

- 320 × 240 × 2 = 153,600 B = 150 KiB
  framebuffer for 2-byte colors
    - 307,200 B = 300 KiB for a double buffer
- Leaves 220 KiB work RAM

- TODO: Look into line buffering

## Simulation Engine Features

### Main Menu

- Continue
- New Simulation
- Settings

### Overworld

- Q13.19 fixed-point kinematics
    - Top-down discrete grid view
    - Continuous, smooth scrolling camera

- 4 or 8-directional movement: walking, running, and jumping
    - Continuous movement on discrete grid
- Interact with other characters and environment objects
    - Character portraits when talking?
- Idle and environmental animation
    - 3-frame animation for characters
    - 6-frame animation for supernatural
    - Physics-based particles ✨
    - Day / night cycle
- Enter/exit buildings and other locations
    - Loading screens
    - Separate submaps for interiors?

- 32 × 32 px sprites for characters
    - 16-bit color
- 8-bit color for supernatural and environment
- 2-bit alpha? Glow?

#### Pause Menu

- **Sketchbook**: Story missions & fast travel
- **To-Do List**: Side quests
- **Notes**: Party’s stats
- **Toolbag**: Items & equipment
- **Save**
- **Settings**
- **Quit**: Go to home menu

#### Virginia Exclusion Zone

- Upgraded Q32.32 fixed-point kinematics
- 8-bit colors for characters
    - Occasionally glitches
- 16-bit colors for supernatural
- Reversed music & sound effects / off-sync sound effects
- Broken physics
    - Overshoot or invert movement and actions
    - Randomly invert or null controls
    - Randomly teleport to certain tiles

### Battle

- Turn-based
- Actions
    - Attack
    - Defend
    - Cast/Skill
    - Item
    - Move
    - Flee
- Stats
    - **HP**: Health Points
    - **EP**: Energy Points
    - **Str**: Affects physical actions
    - **Mag**: Affects magical actions
    - **Def**: Affects physical defense
    - **Res**: Affects magical resistance
    - **Spd**: Affects turn cooldown
    - **Mov**: Affects travel distance
- Environment hazards/obstacles
- Top-down _Fire Emblem_-like grid or _Naruto: Path of the Ninja_ arena
    - Both discrete grids

#### Final Boss

- Upgraded Q32.32 fixed-point kinematics
- 12-frame animation for boss idle
    - 15-frame animation for special move
- 15-frame animation for special move
    - Causes all colors to glitch for a few clock ticks
- Occasionally use the Exclusion Zone’s broken physics
    - Rare chance to null user-input
    - Glitch a random stat during a turn
