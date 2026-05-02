#include "mahjongg_engine.h"

#include <stdio.h>

static int fail(const char *message)
{
    fprintf(stderr, "smoke-test: %s\n", message);
    return 1;
}

int main(void)
{
    MahjonggGame game;
    MahjonggMove moves[MAHJONGG_MAX_MOVES];
    int a = -1;
    int b = -1;
    int count;

    mahjongg_game_init(&game, 1234);

    if (game.tile_count != MAHJONGG_MAX_TILES)
        return fail("tile count is wrong");

    if (mahjongg_tiles_remaining(&game) != MAHJONGG_MAX_TILES)
        return fail("initial remaining count is wrong");

    count = mahjongg_collect_matches(&game, moves, MAHJONGG_MAX_MOVES);
    if (count <= 0)
        return fail("opening layout should have at least one match");

    if (!mahjongg_find_hint(&game, &a, &b))
        return fail("hint failed");

    if (!mahjongg_can_match(&game, a, b))
        return fail("hint was not a legal match");

    if (!mahjongg_apply_match(&game, a, b))
        return fail("failed to apply legal match");

    if (mahjongg_tiles_remaining(&game) != MAHJONGG_MAX_TILES - 2)
        return fail("remaining count after match is wrong");

    if (!mahjongg_undo(&game))
        return fail("undo failed");

    if (mahjongg_tiles_remaining(&game) != MAHJONGG_MAX_TILES)
        return fail("undo did not restore tile count");

    mahjongg_game_init(&game, 4321);
    for (count = 0; count < game.tile_count; count++)
        game.tiles[count].removed = 1;

    game.tiles[0].removed = 0;
    game.tiles[1].removed = 0;
    game.tiles[0].x = 0;
    game.tiles[0].y = 0;
    game.tiles[0].layer = 0;
    game.tiles[1].x = 8;
    game.tiles[1].y = 0;
    game.tiles[1].layer = 0;

    game.tiles[0].kind = 33;
    game.tiles[1].kind = 36;
    if (!mahjongg_can_match(&game, 0, 1))
        return fail("season tiles should match as a group");

    game.tiles[0].kind = 38;
    game.tiles[1].kind = 41;
    if (!mahjongg_can_match(&game, 0, 1))
        return fail("flower tiles should match as a group");

    game.tiles[0].kind = 33;
    game.tiles[1].kind = 38;
    if (mahjongg_can_match(&game, 0, 1))
        return fail("season and flower groups should not match each other");

    puts("smoke-test: ok");
    return 0;
}
