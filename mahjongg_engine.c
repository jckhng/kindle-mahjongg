#include "mahjongg_engine.h"

#include <stdint.h>
#include <string.h>

typedef struct {
    int x;
    int y;
    int layer;
} TilePos;

static const TilePos easy_map[MAHJONGG_MAX_TILES] = {
    {13,7,4},{12,8,3},{14,8,3},{12,6,3},{14,6,3},
    {10,10,2},{12,10,2},{14,10,2},{16,10,2},{10,8,2},{12,8,2},{14,8,2},
    {16,8,2},{10,6,2},{12,6,2},{14,6,2},{16,6,2},{10,4,2},{12,4,2},
    {14,4,2},{16,4,2},{8,12,1},{10,12,1},{12,12,1},{14,12,1},{16,12,1},
    {18,12,1},{8,10,1},{10,10,1},{12,10,1},{14,10,1},{16,10,1},{18,10,1},
    {8,8,1},{10,8,1},{12,8,1},{14,8,1},{16,8,1},{18,8,1},{8,6,1},
    {10,6,1},{12,6,1},{14,6,1},{16,6,1},{18,6,1},{8,4,1},{10,4,1},
    {12,4,1},{14,4,1},{16,4,1},{18,4,1},{8,2,1},{10,2,1},{12,2,1},
    {14,2,1},{16,2,1},{18,2,1},{2,14,0},{4,14,0},{6,14,0},{8,14,0},
    {10,14,0},{12,14,0},{14,14,0},{16,14,0},{18,14,0},{20,14,0},{22,14,0},
    {24,14,0},{6,12,0},{8,12,0},{10,12,0},{12,12,0},{14,12,0},{16,12,0},
    {18,12,0},{20,12,0},{4,10,0},{6,10,0},{8,10,0},{10,10,0},{12,10,0},
    {14,10,0},{16,10,0},{18,10,0},{20,10,0},{22,10,0},{0,7,0},{2,8,0},
    {4,8,0},{6,8,0},{8,8,0},{10,8,0},{12,8,0},{14,8,0},{16,8,0},{18,8,0},
    {20,8,0},{22,8,0},{24,8,0},{2,6,0},{4,6,0},{6,6,0},{8,6,0},{10,6,0},
    {12,6,0},{14,6,0},{16,6,0},{18,6,0},{20,6,0},{22,6,0},{24,6,0},
    {4,4,0},{6,4,0},{8,4,0},{10,4,0},{12,4,0},{14,4,0},{16,4,0},{18,4,0},
    {20,4,0},{22,4,0},{6,2,0},{8,2,0},{10,2,0},{12,2,0},{14,2,0},{16,2,0},
    {18,2,0},{20,2,0},{2,0,0},{4,0,0},{6,0,0},{8,0,0},{10,0,0},{12,0,0},
    {14,0,0},{16,0,0},{18,0,0},{20,0,0},{22,0,0},{24,0,0},{26,7,0},{28,7,0}
};

static uint32_t next_rand(uint32_t *state)
{
    *state = (*state * 1664525u) + 1013904223u;
    return *state;
}

static int overlap_1d(int a0, int a1, int b0, int b1)
{
    return a0 < b1 && b0 < a1;
}

static int rects_overlap(const MahjonggTile *a, const MahjonggTile *b)
{
    return overlap_1d(a->x, a->x + 2, b->x, b->x + 2) &&
           overlap_1d(a->y, a->y + 2, b->y, b->y + 2);
}

void mahjongg_game_init(MahjonggGame *game, unsigned int seed)
{
    int kinds[MAHJONGG_MAX_TILES];
    uint32_t rng = seed ? seed : 1u;
    int i;
    int n = 0;

    memset(game, 0, sizeof(*game));
    game->tile_count = MAHJONGG_MAX_TILES;
    game->seed = seed;

    for (i = 0; i <= 32; i++) {
        kinds[n++] = i;
        kinds[n++] = i;
        kinds[n++] = i;
        kinds[n++] = i;
    }

    kinds[n++] = 33;
    kinds[n++] = 34;
    kinds[n++] = 35;
    kinds[n++] = 36;

    kinds[n++] = 37;
    kinds[n++] = 37;
    kinds[n++] = 37;
    kinds[n++] = 37;

    kinds[n++] = 38;
    kinds[n++] = 39;
    kinds[n++] = 40;
    kinds[n++] = 41;

    for (i = MAHJONGG_MAX_TILES - 1; i > 0; i--) {
        int j = (int)(next_rand(&rng) % (uint32_t)(i + 1));
        int tmp = kinds[i];
        kinds[i] = kinds[j];
        kinds[j] = tmp;
    }

    for (i = 0; i < MAHJONGG_MAX_TILES; i++) {
        game->tiles[i].x = easy_map[i].x;
        game->tiles[i].y = easy_map[i].y;
        game->tiles[i].layer = easy_map[i].layer;
        game->tiles[i].kind = kinds[i];
        game->tiles[i].removed = 0;
    }
}

void mahjongg_game_restart(MahjonggGame *game)
{
    unsigned int seed = game->seed;
    mahjongg_game_init(game, seed);
}

static int match_group(int kind)
{
    if (kind >= 33 && kind <= 36)
        return 33;
    if (kind == 37)
        return 34;
    if (kind >= 38 && kind <= 41)
        return 35;
    return kind;
}

int mahjongg_tiles_remaining(const MahjonggGame *game)
{
    int count = 0;
    int i;

    for (i = 0; i < game->tile_count; i++)
        if (!game->tiles[i].removed)
            count++;

    return count;
}

int mahjongg_is_free(const MahjonggGame *game, int tile_index)
{
    const MahjonggTile *tile;
    int left_blocked = 0;
    int right_blocked = 0;
    int i;

    if (tile_index < 0 || tile_index >= game->tile_count)
        return 0;

    tile = &game->tiles[tile_index];
    if (tile->removed)
        return 0;

    for (i = 0; i < game->tile_count; i++) {
        const MahjonggTile *other = &game->tiles[i];

        if (i == tile_index || other->removed)
            continue;

        if (other->layer > tile->layer && rects_overlap(tile, other))
            return 0;

        if (other->layer == tile->layer &&
            overlap_1d(tile->y, tile->y + 2, other->y, other->y + 2)) {
            if (other->x + 2 == tile->x)
                left_blocked = 1;
            if (other->x == tile->x + 2)
                right_blocked = 1;
        }
    }

    return !left_blocked || !right_blocked;
}

int mahjongg_can_match(const MahjonggGame *game, int a, int b)
{
    if (a == b || a < 0 || b < 0 || a >= game->tile_count || b >= game->tile_count)
        return 0;

    return match_group(game->tiles[a].kind) == match_group(game->tiles[b].kind) &&
           mahjongg_is_free(game, a) &&
           mahjongg_is_free(game, b);
}

int mahjongg_apply_match(MahjonggGame *game, int a, int b)
{
    if (!mahjongg_can_match(game, a, b) || game->history_count >= MAHJONGG_MAX_MOVES)
        return 0;

    game->tiles[a].removed = 1;
    game->tiles[b].removed = 1;
    game->history[game->history_count].a = a;
    game->history[game->history_count].b = b;
    game->history_count++;
    return 1;
}

int mahjongg_undo(MahjonggGame *game)
{
    MahjonggMove move;

    if (game->history_count <= 0)
        return 0;

    game->history_count--;
    move = game->history[game->history_count];
    game->tiles[move.a].removed = 0;
    game->tiles[move.b].removed = 0;
    return 1;
}

int mahjongg_collect_matches(const MahjonggGame *game, MahjonggMove *moves, int max_moves)
{
    int count = 0;
    int i;
    int j;

    for (i = 0; i < game->tile_count; i++) {
        if (!mahjongg_is_free(game, i))
            continue;
        for (j = i + 1; j < game->tile_count; j++) {
            if (!mahjongg_can_match(game, i, j))
                continue;
            if (moves && count < max_moves) {
                moves[count].a = i;
                moves[count].b = j;
            }
            count++;
        }
    }

    return count;
}

int mahjongg_find_hint(const MahjonggGame *game, int *a, int *b)
{
    MahjonggMove move;

    if (mahjongg_collect_matches(game, &move, 1) <= 0)
        return 0;

    if (a)
        *a = move.a;
    if (b)
        *b = move.b;
    return 1;
}

const char *mahjongg_kind_label(int kind)
{
    static const char *labels[MAHJONGG_KIND_COUNT] = {
        "1D","2D","3D","4D","5D","6D","7D","8D","9D",
        "1C","2C","3C","4C","5C","6C","7C","8C","9C",
        "1B","2B","3B","4B","5B","6B","7B","8B","9B",
        "N","E","S","W","R","G","S1","S2","S3","S4","Wh","F1","F2","F3","F4"
    };

    if (kind < 0 || kind >= MAHJONGG_KIND_COUNT)
        return "?";
    return labels[kind];
}
