# Exact Mahjong Solitaire PWA

An installable browser Mahjong Solitaire game with muted tile colors for easy visual matching.

## Features

- 144-tile layout (Easy/Turtle layout from GNOME Mahjongg).
- Muted color coding by suit group for easy identification.
- Hint system highlights a valid matching pair.
- Undo moves.
- Restart the same layout or start a new random layout.
- Save/Load a manual restore point.
- Works offline after first load.
- Installable via Chrome "Add to Home Screen."

## Building

```bash
npm install
npm run typecheck
npm run build
```

## Rules

Click a free tile (one that is not covered from above and has at least one open
side) to select it, then click a matching free tile to remove the pair.

Special matching rules (matching within group):
- Seasons (春夏秋冬) all match each other.
- Flowers (梅蘭菊竹) all match each other.
- White Dragon tiles (⬜) match each other.

## Attribution

Rules engine ported from the GNOME Mahjongg / Exact Mahjong Solitaire native
Kindle implementation. Part of the Exact Games / GnomeGames4Kindle project.

## License

GPL-3.0-or-later. See THIRD_PARTY.md for dependency details.
