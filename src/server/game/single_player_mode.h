#ifndef CODUO_GAME_SINGLE_PLAYER_MODE_H
#define CODUO_GAME_SINGLE_PLAYER_MODE_H

/*
 * NOT_FROM_ORIGINAL_SOURCE: code-side single-player game support for the
 * CODUO_SP game-module build.
 *
 * The recovered multiplayer game module relies on the maps/mp/gametypes
 * script framework to spawn and respawn players. The single-player product
 * has no gametype layer, so this support places the local player at a map
 * spawn point and respawns after death without scripts. When a level script
 * or script callback does spawn the player first, that script flow stays
 * authoritative and these hooks simply find the client already active.
 */

#include "recovered_game.h"

/* Place the client at a map spawn point and spawn it into the game. */
void coduosp_client_begin(gentity_t *ent);

/* Record a script-less player death for the code-side respawn timer. */
void coduosp_client_died(gentity_t *ent);

/* Respawn dead single-player clients whose respawn delay has elapsed. */
void coduosp_run_frame(void);

#endif
