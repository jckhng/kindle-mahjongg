import { useEffect, useMemo, useRef, useState } from "react";

const MAX_TILES = 144;
// Tile themes match native exact-mahjong-solitaire combo (Postmodern, Smooth).
type Theme = "postmodern" | "smooth";

// PNG sprite: 43 tile faces across, 2 rows tall (row 0 = face used for display)
const SPRITE_COLS = 43;

interface TilePos { x: number; y: number; layer: number; }

const EASY_MAP: TilePos[] = [
  {x:13,y:7,layer:4},{x:12,y:8,layer:3},{x:14,y:8,layer:3},{x:12,y:6,layer:3},{x:14,y:6,layer:3},
  {x:10,y:10,layer:2},{x:12,y:10,layer:2},{x:14,y:10,layer:2},{x:16,y:10,layer:2},{x:10,y:8,layer:2},{x:12,y:8,layer:2},{x:14,y:8,layer:2},
  {x:16,y:8,layer:2},{x:10,y:6,layer:2},{x:12,y:6,layer:2},{x:14,y:6,layer:2},{x:16,y:6,layer:2},{x:10,y:4,layer:2},{x:12,y:4,layer:2},
  {x:14,y:4,layer:2},{x:16,y:4,layer:2},{x:8,y:12,layer:1},{x:10,y:12,layer:1},{x:12,y:12,layer:1},{x:14,y:12,layer:1},{x:16,y:12,layer:1},
  {x:18,y:12,layer:1},{x:8,y:10,layer:1},{x:10,y:10,layer:1},{x:12,y:10,layer:1},{x:14,y:10,layer:1},{x:16,y:10,layer:1},{x:18,y:10,layer:1},
  {x:8,y:8,layer:1},{x:10,y:8,layer:1},{x:12,y:8,layer:1},{x:14,y:8,layer:1},{x:16,y:8,layer:1},{x:18,y:8,layer:1},{x:8,y:6,layer:1},
  {x:10,y:6,layer:1},{x:12,y:6,layer:1},{x:14,y:6,layer:1},{x:16,y:6,layer:1},{x:18,y:6,layer:1},{x:8,y:4,layer:1},{x:10,y:4,layer:1},
  {x:12,y:4,layer:1},{x:14,y:4,layer:1},{x:16,y:4,layer:1},{x:18,y:4,layer:1},{x:8,y:2,layer:1},{x:10,y:2,layer:1},{x:12,y:2,layer:1},
  {x:14,y:2,layer:1},{x:16,y:2,layer:1},{x:18,y:2,layer:1},{x:2,y:14,layer:0},{x:4,y:14,layer:0},{x:6,y:14,layer:0},{x:8,y:14,layer:0},
  {x:10,y:14,layer:0},{x:12,y:14,layer:0},{x:14,y:14,layer:0},{x:16,y:14,layer:0},{x:18,y:14,layer:0},{x:20,y:14,layer:0},{x:22,y:14,layer:0},
  {x:24,y:14,layer:0},{x:6,y:12,layer:0},{x:8,y:12,layer:0},{x:10,y:12,layer:0},{x:12,y:12,layer:0},{x:14,y:12,layer:0},{x:16,y:12,layer:0},
  {x:18,y:12,layer:0},{x:20,y:12,layer:0},{x:4,y:10,layer:0},{x:6,y:10,layer:0},{x:8,y:10,layer:0},{x:10,y:10,layer:0},{x:12,y:10,layer:0},
  {x:14,y:10,layer:0},{x:16,y:10,layer:0},{x:18,y:10,layer:0},{x:20,y:10,layer:0},{x:22,y:10,layer:0},{x:0,y:7,layer:0},{x:2,y:8,layer:0},
  {x:4,y:8,layer:0},{x:6,y:8,layer:0},{x:8,y:8,layer:0},{x:10,y:8,layer:0},{x:12,y:8,layer:0},{x:14,y:8,layer:0},{x:16,y:8,layer:0},{x:18,y:8,layer:0},
  {x:20,y:8,layer:0},{x:22,y:8,layer:0},{x:24,y:8,layer:0},{x:2,y:6,layer:0},{x:4,y:6,layer:0},{x:6,y:6,layer:0},{x:8,y:6,layer:0},{x:10,y:6,layer:0},
  {x:12,y:6,layer:0},{x:14,y:6,layer:0},{x:16,y:6,layer:0},{x:18,y:6,layer:0},{x:20,y:6,layer:0},{x:22,y:6,layer:0},{x:24,y:6,layer:0},
  {x:4,y:4,layer:0},{x:6,y:4,layer:0},{x:8,y:4,layer:0},{x:10,y:4,layer:0},{x:12,y:4,layer:0},{x:14,y:4,layer:0},{x:16,y:4,layer:0},{x:18,y:4,layer:0},
  {x:20,y:4,layer:0},{x:22,y:4,layer:0},{x:6,y:2,layer:0},{x:8,y:2,layer:0},{x:10,y:2,layer:0},{x:12,y:2,layer:0},{x:14,y:2,layer:0},{x:16,y:2,layer:0},
  {x:18,y:2,layer:0},{x:20,y:2,layer:0},{x:2,y:0,layer:0},{x:4,y:0,layer:0},{x:6,y:0,layer:0},{x:8,y:0,layer:0},{x:10,y:0,layer:0},{x:12,y:0,layer:0},
  {x:14,y:0,layer:0},{x:16,y:0,layer:0},{x:18,y:0,layer:0},{x:20,y:0,layer:0},{x:22,y:0,layer:0},{x:24,y:0,layer:0},{x:26,y:7,layer:0},{x:28,y:7,layer:0}
];

function matchGroup(kind: number): number {
  if (kind >= 33 && kind <= 36) return 33;
  if (kind === 37) return 34;
  if (kind >= 38 && kind <= 41) return 35;
  return kind;
}

const KIND_LABEL_C = [
  "1●","2●","3●","4●","5●","6●","7●","8●","9●",
  "1文","2文","3文","4文","5文","6文","7文","8文","9文",
  "1竹","2竹","3竹","4竹","5竹","6竹","7竹","8竹","9竹",
  "北","東","南","西","中","發","白",
  "春","夏","秋","冬","⬜",
  "梅","蘭","菊","竹"
];

function kindLabel(kind: number): string { return KIND_LABEL_C[kind] ?? "?"; }

function kindSuitClass(kind: number): string {
  if (kind <= 8) return "suit-dots";
  if (kind <= 17) return "suit-chars";
  if (kind <= 26) return "suit-bam";
  if (kind <= 30) return "suit-winds";
  if (kind <= 32) return "suit-dragons";
  if (kind <= 36) return "suit-seasons";
  if (kind === 37) return "suit-dragons";
  return "suit-flowers";
}

interface Tile { x: number; y: number; layer: number; kind: number; removed: boolean; }
interface MoveRecord { a: number; b: number; }

interface GameState { tiles: Tile[]; history: MoveRecord[]; seed: number; }
interface PersistedState { game: GameState; savedGame: GameState | null; theme: Theme; }
const STORAGE_KEY = "exact-mahjong-solitaire-pwa-v2";

function lcgNext(state: number): number { return (Math.imul(state, 1664525) + 1013904223) >>> 0; }

function initGame(seed: number): GameState {
  const rng_seed = seed || 1;
  let rng = rng_seed;
  const kinds: number[] = [];
  for (let i = 0; i <= 32; i++) for (let k = 0; k < 4; k++) kinds.push(i);
  kinds.push(33, 34, 35, 36);
  for (let k = 0; k < 4; k++) kinds.push(37);
  kinds.push(38, 39, 40, 41);
  for (let i = MAX_TILES - 1; i > 0; i--) {
    rng = lcgNext(rng);
    const j = rng % (i + 1);
    [kinds[i], kinds[j]] = [kinds[j], kinds[i]];
  }
  const tiles: Tile[] = EASY_MAP.map((pos, i) => ({ x: pos.x, y: pos.y, layer: pos.layer, kind: kinds[i], removed: false }));
  return { tiles, history: [], seed: rng_seed };
}

function replayToStep(seed: number, history: MoveRecord[], steps: number): Tile[] {
  const g = initGame(seed);
  const lim = Math.min(steps, history.length);
  for (let i = 0; i < lim; i++) {
    const { a, b } = history[i];
    g.tiles[a].removed = true;
    g.tiles[b].removed = true;
  }
  return g.tiles;
}

function overlap1d(a0: number, a1: number, b0: number, b1: number): boolean { return a0 < b1 && b0 < a1; }
function rectsOverlap(a: Tile, b: Tile): boolean { return overlap1d(a.x, a.x+2, b.x, b.x+2) && overlap1d(a.y, a.y+2, b.y, b.y+2); }

function isFree(tiles: Tile[], idx: number): boolean {
  const tile = tiles[idx];
  if (tile.removed) return false;
  let leftBlocked = false, rightBlocked = false;
  for (let i = 0; i < tiles.length; i++) {
    if (i === idx || tiles[i].removed) continue;
    const other = tiles[i];
    if (other.layer > tile.layer && rectsOverlap(tile, other)) return false;
    if (other.layer === tile.layer && overlap1d(tile.y, tile.y+2, other.y, other.y+2)) {
      if (other.x + 2 === tile.x) leftBlocked = true;
      if (other.x === tile.x + 2) rightBlocked = true;
    }
  }
  return !leftBlocked || !rightBlocked;
}

function canMatch(tiles: Tile[], a: number, b: number): boolean {
  if (a === b || a < 0 || b < 0) return false;
  return matchGroup(tiles[a].kind) === matchGroup(tiles[b].kind) && isFree(tiles, a) && isFree(tiles, b);
}

function applyMatch(g: GameState, a: number, b: number): GameState | null {
  if (!canMatch(g.tiles, a, b)) return null;
  const tiles = g.tiles.map((t, i) => i === a || i === b ? { ...t, removed: true } : t);
  return { ...g, tiles, history: [...g.history, { a, b }] };
}

function undoMatch(g: GameState): GameState {
  if (g.history.length === 0) return g;
  const last = g.history[g.history.length - 1];
  const tiles = g.tiles.map((t, i) => i === last.a || i === last.b ? { ...t, removed: false } : t);
  return { ...g, tiles, history: g.history.slice(0, -1) };
}

function collectMatches(tiles: Tile[]): MoveRecord[] {
  const moves: MoveRecord[] = [];
  for (let i = 0; i < tiles.length; i++) {
    if (!isFree(tiles, i)) continue;
    for (let j = i + 1; j < tiles.length; j++) if (canMatch(tiles, i, j)) moves.push({ a: i, b: j });
  }
  return moves;
}

function tilesRemaining(tiles: Tile[]): number { return tiles.filter(t => !t.removed).length; }

const VALID_THEMES = new Set<Theme>(["postmodern", "smooth"]);
const fallback: PersistedState = { game: initGame(Date.now() & 0xffffffff), savedGame: null, theme: "postmodern" };
function loadState(): PersistedState {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return fallback;
    const parsed = JSON.parse(raw) as Partial<PersistedState>;
    return { ...fallback, ...parsed, theme: VALID_THEMES.has(parsed.theme as Theme) ? (parsed.theme as Theme) : fallback.theme };
  } catch { return fallback; }
}

const TW = 26, TH = 36, LAYER_OX = 3, LAYER_OY = -3;
const TILE_W = TW * 2, TILE_H = TH * 2; // 52 × 72
const MAX_X = 30, MAX_Y = 16, MAX_LAYER = 5;
const BOARD_W = MAX_X * TW + MAX_LAYER * Math.abs(LAYER_OX) + 8;
const BOARD_H = MAX_Y * TH + MAX_LAYER * Math.abs(LAYER_OY) + 8;
const PUBLIC_BASE = import.meta.env.BASE_URL;

export default function App() {
  const initial = useMemo(loadState, []);
  const [game, setGame] = useState<GameState>(initial.game);
  const [savedGame, setSavedGame] = useState<GameState | null>(initial.savedGame);
  const [selected, setSelected] = useState<number | null>(null);
  const [hint, setHint] = useState<[number, number] | null>(null);
  const [message, setMessage] = useState("");
  const [page, setPage] = useState<"game" | "about">("game");
  const [theme, setTheme] = useState<Theme>(initial.theme);
  const boardAreaRef = useRef<HTMLElement | null>(null);
  const [boardScale, setBoardScale] = useState(1);

  const remaining = useMemo(() => tilesRemaining(game.tiles), [game.tiles]);
  const matches = useMemo(() => collectMatches(game.tiles), [game]);
  const isOver = remaining === 0;
  const isStuck = remaining > 0 && matches.length === 0;

  useEffect(() => {
    localStorage.setItem(STORAGE_KEY, JSON.stringify({ game, savedGame, theme }));
  }, [game, savedGame, theme]);

  useEffect(() => {
    const area = boardAreaRef.current;
    if (!area) return;
    const updateScale = () => {
      const style = window.getComputedStyle(area);
      const horizontalPadding = parseFloat(style.paddingLeft) + parseFloat(style.paddingRight);
      const availableWidth = Math.max(0, area.clientWidth - horizontalPadding);
      setBoardScale(Math.min(1, availableWidth / BOARD_W));
    };
    updateScale();
    const observer = new ResizeObserver(updateScale);
    observer.observe(area);
    return () => observer.disconnect();
  }, []);

  function tapTile(idx: number) {
    setHint(null);
    if (game.tiles[idx].removed || !isFree(game.tiles, idx)) return;
    if (selected === null) { setSelected(idx); setMessage(`Selected: ${kindLabel(game.tiles[idx].kind)}`); return; }
    if (selected === idx) { setSelected(null); setMessage("Deselected."); return; }
    if (canMatch(game.tiles, selected, idx)) {
      const next = applyMatch(game, selected, idx);
      if (next) {
        setGame(next); setSelected(null);
        const rem = tilesRemaining(next.tiles);
        setMessage(rem === 0 ? "You win! All tiles cleared!" : `Matched ${kindLabel(game.tiles[selected].kind)}. ${rem} tiles left.`);
      }
    } else {
      setSelected(idx);
      setMessage(`Selected: ${kindLabel(game.tiles[idx].kind)}`);
    }
  }

  function handleNew() { setGame(initGame(Date.now() & 0xffffffff)); setSelected(null); setHint(null); setMessage("New game."); }
  function handleRestart() { setGame(initGame(game.seed)); setSelected(null); setHint(null); setMessage("Restarted same layout."); }
  function handleUndo() { setGame(undoMatch(game)); setSelected(null); setHint(null); setMessage("Undone."); }
  function handleHint() {
    if (matches.length === 0) { setMessage("No moves available."); return; }
    const pick = matches[Math.floor(Math.random() * matches.length)];
    setHint([pick.a, pick.b]); setSelected(null);
    setMessage(`Hint: ${kindLabel(game.tiles[pick.a].kind)} can match.`);
  }
  function handleSave() { setSavedGame(game); setMessage("Saved."); }
  function handleLoad() {
    if (!savedGame) { setMessage("No saved game."); return; }
    setGame(savedGame); setSelected(null); setHint(null); setMessage("Loaded.");
  }

  const sortedTiles = useMemo(() => {
    return game.tiles
      .map((t, i) => ({ ...t, idx: i }))
      .filter(t => !t.removed)
      .sort((a, b) => a.layer - b.layer || a.y - b.y || a.x - b.x);
  }, [game.tiles]);

  const freeTileSet = useMemo(() => {
    const s = new Set<number>();
    for (let i = 0; i < game.tiles.length; i++) {
      if (!game.tiles[i].removed && isFree(game.tiles, i)) s.add(i);
    }
    return s;
  }, [game.tiles]);

  function statusMsg(): string {
    if (isOver) return "Congratulations! All tiles cleared!";
    if (isStuck) return `Stuck! No moves. ${remaining} tiles left.`;
    return message || `${remaining} tiles remaining. ${matches.length} possible matches.`;
  }

  return (
    <main className="app">
      <header className="hero">
        <h1>Exact Mahjong Solitaire</h1>
        <p>{statusMsg()}</p>
      </header>

      <section className="toolbar" aria-label="Game controls">
        <button onClick={handleNew}>New</button>
        <button onClick={handleRestart}>Restart</button>
        <button onClick={handleUndo} disabled={game.history.length === 0}>Undo</button>
        <button onClick={handleHint} disabled={matches.length === 0}>Hint</button>
        <button onClick={handleSave}>Save</button>
        <button onClick={handleLoad}>Load</button>
      </section>

      <section className="settings-row" aria-label="Settings">
        <label>Theme
          <select value={theme} onChange={e => setTheme(e.target.value as Theme)}>
            <option value="postmodern">Postmodern</option>
            <option value="smooth">Smooth</option>
          </select>
        </label>
        <button onClick={() => setPage(p => p === "game" ? "about" : "game")}>{page === "game" ? "About" : "← Game"}</button>
      </section>

      {page === "about" ? (
        <section className="about-page">
          <h2>About Exact Mahjong Solitaire</h2>
          <p>An installable browser port of Exact Mahjong Solitaire. Rules engine ported from the native Kindle mahjongg implementation (derived from GNOME Mahjongg).</p>
          <p>Select two free matching tiles to remove them. A tile is free if nothing covers it from above and it has at least one open side. Clear all 144 tiles to win.</p>
          <p>Seasons (春夏秋冬) and Flowers (梅蘭菊竹) match any tile in their group. The White Dragon (⬜) matches any other White Dragon.</p>
          <p>Attribution: GNOME Mahjongg authors, KUAL porting lineage. License: GPL-3.0-or-later.</p>
          <button onClick={() => { localStorage.removeItem(STORAGE_KEY); handleNew(); }}>Clear Browser Save</button>
        </section>
      ) : (
        <section ref={boardAreaRef} className="board-area" aria-label="Mahjong board">
          <div className="board-fit" style={{ width: BOARD_W * boardScale, height: BOARD_H * boardScale }}>
          <div
            className="board-inner"
            style={{ width: BOARD_W, height: BOARD_H, transform: `scale(${boardScale})` }}
          >
            {sortedTiles.map(tile => {
              const px = tile.x * TW + tile.layer * LAYER_OX + 4;
              const py = tile.y * TH + tile.layer * LAYER_OY + 4;
              const zIndex = tile.layer * 20 + (tile.y * 2 + tile.x) % 20;
              const isFreeT = freeTileSet.has(tile.idx);
              const isSel = selected === tile.idx;
              const isHintA = hint?.[0] === tile.idx;
              const isHintB = hint?.[1] === tile.idx;
              const label = kindLabel(tile.kind);
              const spriteStyle = {
                backgroundImage: `url(${PUBLIC_BASE}${theme}.png)`,
                backgroundSize: `${SPRITE_COLS * TILE_W}px ${2 * TILE_H}px`,
                backgroundPosition: `-${tile.kind * TILE_W}px 0px`,
                backgroundRepeat: "no-repeat" as const,
              };
              return (
                <div
                  key={tile.idx}
                  className={[
                    "tile",
                    "tile-sprite",
                    !isFreeT ? "blocked" : "",
                    isSel ? "selected" : "",
                    isHintA ? "hint-a" : isHintB ? "hint-b" : ""
                  ].filter(Boolean).join(" ")}
                  style={{ left: px, top: py, width: TILE_W, height: TILE_H, zIndex, ...spriteStyle }}
                  onClick={() => tapTile(tile.idx)}
                  title={label}
                />
              );
            })}
          </div>
          </div>

          <div className="stats-bar">
            <span>Remaining: {remaining}</span>
            <span>Matches: {matches.length}</span>
            <span>Removed: {MAX_TILES - remaining}</span>
            <span>Moves: {game.history.length}</span>
          </div>
        </section>
      )}

      <footer className="notes">
        <p>Click a free tile, then click a matching free tile to remove the pair. State auto-saved.</p>
      </footer>
    </main>
  );
}
