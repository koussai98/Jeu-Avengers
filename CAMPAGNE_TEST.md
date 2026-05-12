# Avengers Campaign - Test Build

This is a test build of the campaign mode added on top of your existing PvP game.
Everything is placeholder shapes for now (per request) - polish phase comes later.

## What was added

### New files (in `src/`)
- **`entites.h` / `entites.c`** - Coins, obstacles, traps, enemies, projectiles. All shape-based (yellow circles for coins, red boxes for traps, brown boxes for obstacles, colored circles for enemies).
- **`campagne.h` / `campagne.c`** - Campaign state machine: tracks current level, handles transitions, win/game-over screens.
- **`thanos_bridge.h` / `thanos_bridge.c`** - Wraps your friend's NPC code. Translates between the player's coordinate system and the NPC's, applies player attacks to Thanos and Thanos attacks to the player.
- **`npc.h` / `npc.c`** - YOUR FRIEND'S CODE (copied from his GitHub). Two small edits:
  1. Asset paths now point to `assets/thanos/` instead of `assets/`
  2. Filename `heavyattackkda1.png` corrected to `heavyattackda1.png` (his repo had a typo)
  3. World size changed from 900x600 to 800x600 to match the game
  4. Sprite size changed from 60x80 to 110x150 to make Thanos imposing as a boss

### Modified files
- `types.h` - Added `NIVEAU_CAMPAGNE = 10`
- `menu_principal.c/h` - Added "Campagne" button (now 6 buttons total)
- `main.c` - Added campaign init/input/update/draw branches

### Assets
- `assets/thanos/` - All Thanos sprites copied from your friend's repo

## How the campaign flows

```
Main Menu → "Campagne" → LEVEL 1-A
   |
   v
LEVEL 1-A (tutorial, no enemies)
   - Walk past the YELLOW DOOR on the right edge → 1B
   - Obstacles (brown boxes) block movement, jump over them
   - Traps (red spikes) damage you - 60 frame i-frames after hit
   - Coins (yellow circles) = +10 score each
   |
   v
LEVEL 1-B (same map idea, enemies spawn gradually)
   - 5 enemies spawn one at a time (3 melee then 2 ranged)
   - First melee, then ranged once you've handled a few
   - Kill all 5 → Level 2
   |
   v
LEVEL 2 (more enemies + obstacles + traps + coins)
   - Starts with 3 enemies on screen, 4 more spawn in waves
   - Clear them all → Boss arena
   |
   v
BOSS (Thanos using friend's NPC code)
   - HP 5, gets faster + more aggressive after each respawn (his "rage" mechanic)
   - Punch attacks at close range, heavy attack when fully enraged
   - You hit him → he stuns and bleeds. He hits you → you lose a life
   - Defeat him → VICTORY screen
```

## Controls (campaign)

- **Q / D** - move left / right
- **Space** - jump
- **A** - attack
- **LShift** - hold to run
- **Esc** - back to main menu (loses progress)
- On victory/game-over screen: **Enter** or **Space** to return to menu

## Build

Same as before:

```bash
make
./game
```

Or manually:

```bash
gcc src/*.c -o game -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm
```

## Known limitations (deliberate, for the test build)

1. **Coins / enemies / traps / obstacles are shapes, not sprites** - per your request. Easy to swap in art later.
2. **No background art** - pure green, per your request.
3. **No music in campaign mode** - only menu music.
4. **Single screen, no scrolling** - levels are 800x600 single-room.
5. **No platform jumping** - obstacles block, jump over them. Player can't stand ON top.
6. **No character pick / unique abilities yet** - using Joueur 1 sprite + standard attack only.
7. **No save** - dying = game over, return to menu.
8. **Puzzle on death is PvP-only** - campaign uses lives + game-over screen instead.
9. **The puzzle module structural mess (separate Game struct, globals) was NOT refactored** - that's a polish-phase task.

## Things to test

1. Walk to right edge of 1A - does it transition?
2. In 1B, do enemies spawn one at a time?
3. Do traps damage you with proper i-frame window?
4. Do coins add score?
5. Does Thanos appear after Level 2 cleared?
6. Can you damage Thanos with your attack?
7. Does Thanos hurt you with his punches/heavy attack?
8. Does game-over trigger when you lose all 5 lives?
9. Does Esc return you to main menu cleanly?
10. Going back into Campagne after a run - does it start fresh from 1A?

## Things I'd polish next (if you agree)

1. **Refactor the puzzle module to use `GameState`/`Bouton`/`Niveau`** - kill the bridge file and globals.
2. **Add character pick screen** with unique abilities (Thor hammer throw, Iron Man hover, Hulk ground pound, Spider-Man double jump).
3. **Real sprites for enemies/coins/traps** instead of shapes.
4. **Background art** per level.
5. **Platform jumping** - so you can stand on top of obstacles.
6. **Save/checkpoint** system.
7. **Sound effects** for hits, coin pickup, enemy death, Thanos roar.
