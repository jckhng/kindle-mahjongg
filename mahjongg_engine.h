#ifndef KINDLE_MAHJONGG_ENGINE_H
#define KINDLE_MAHJONGG_ENGINE_H

#include <stddef.h>

#define MAHJONGG_MAX_TILES 144
#define MAHJONGG_KIND_COUNT 36
#define MAHJONGG_MAX_MOVES 72

typedef struct {
    int x;
    int y;
    int layer;
    int kind;
    int removed;
} MahjonggTile;

typedef struct {
    int a;
    int b;
} MahjonggMove;

typedef struct {
    MahjonggTile tiles[MAHJONGG_MAX_TILES];
    MahjonggMove history[MAHJONGG_MAX_MOVES];
    int tile_count;
    int history_count;
    unsigned int seed;
} MahjonggGame;

void mahjongg_game_init(MahjonggGame *game, unsigned int seed);
int mahjongg_tiles_remaining(const MahjonggGame *game);
int mahjongg_is_free(const MahjonggGame *game, int tile_index);
int mahjongg_can_match(const MahjonggGame *game, int a, int b);
int mahjongg_apply_match(MahjonggGame *game, int a, int b);
int mahjongg_undo(MahjonggGame *game);
int mahjongg_collect_matches(const MahjonggGame *game, MahjonggMove *moves, int max_moves);
int mahjongg_find_hint(const MahjonggGame *game, int *a, int *b);
const char *mahjongg_kind_label(int kind);

#endif

