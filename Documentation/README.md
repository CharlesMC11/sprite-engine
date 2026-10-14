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
    - Address: `0x0500_0000`—`0x0500_003FF`
        - Backgrounds: `0x0500_0000`—`0x0500_001FF`
        - Sprites: `0x0500_0200`—`0x0500_003FF`
    - 256 15‑bit colors (each for backgrounds and sprites)
        - 16 colors per 16 palettes
        - XBGR555 encoding
- 1 KiB Object Attribute Memory
    - Address: `0x0700_0000`—`0x0700_003FF`
    - 128 sprite entries

- Must vait for VBlank / HBlank

#### Game Pak

- 32 MB ROM

### _Pokémon Emerald Version_

- Top-down overworld graphics
- 16 × 16 grid blocks
- 64 × 64 battle sprites

### STMElectronics or RaspberryPi Pico?

## Engine Features

### Main Menu

- Continue
- New Game
- Options?

### Overworld

- Top-down grid view
- 4 or 8-directional movement with walking, running, and jumping
- Interact with other characters and environment objects
- Idle and environmental animation?
- Enter/exit buildings and other locations
- Loading screens?

### Battle

- Turn-based: Attack, Defend, Skill, Item, Move, Run?
- Decide between top-down _Fire Emblem_ or _Disgaea_-like grid or _Naruto: Path of the Ninja_ stage
- Environment hazards?
