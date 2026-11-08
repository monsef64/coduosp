/*
 * NOT_FROM_ORIGINAL_SOURCE: code-side single-player game support for the
 * CODUO_SP game-module build (see single_player_mode.h).
 *
 * This file has no retail counterpart. The spawn-point classnames are the
 * retail spawn entity names already present in the recovered script constant
 * table (info_player_start and the mp_info_player_* / mp_team_*player_*
 * single- and multiplayer spawn entities). Unknown classnames are kept as
 * inert entities by the recovered G_CallSpawn, so these points exist in
 * every loaded map.
 */

#include "recovered_game.h"
#include "g_public.h"
#include "game_globals.h"
#include "g_syscalls.h"
#include "game_functions.h"
#include "scr_vm.h"
#include "single_player_mode.h"

#include "compat/crt/random_compat.h"

#include <string.h>

enum {
    /* Delay between a script-less death and the code-side respawn. */
    CODUOSP_RESPAWN_DELAY_MS = 2000,
    CODUOSP_SPAWN_CANDIDATE_LIMIT = 64
};

/* Spawn-point preference tiers for the first entry into the level and for
 * respawns. Lower tiers win; a random candidate inside the best populated
 * tier is selected. */
enum {
    CODUOSP_TIER_START = 0,
    CODUOSP_TIER_CHECKPOINT = 1,
    CODUOSP_TIER_TEAM_START = 2,
    CODUOSP_TIER_RESPAWN = 3,
    CODUOSP_TIER_DEATHMATCH = 4,
    CODUOSP_TIER_COUNT = 5
};

static const struct {
    const char *classname;
    int32_t initialTier;
    int32_t respawnTier;
} coduospSpawnClassnames[] = {
    { "info_player_start", CODUOSP_TIER_START, CODUOSP_TIER_DEATHMATCH },
    { "info_player_checkpoint", CODUOSP_TIER_CHECKPOINT,
      CODUOSP_TIER_RESPAWN },
    { "mp_team_alliedplayer_start", CODUOSP_TIER_TEAM_START,
      CODUOSP_TIER_TEAM_START },
    { "mp_team_axisplayer_start", CODUOSP_TIER_TEAM_START,
      CODUOSP_TIER_TEAM_START },
    { "mp_team_alliedplayer_respawn", CODUOSP_TIER_RESPAWN,
      CODUOSP_TIER_RESPAWN },
    { "mp_team_axisplayer_respawn", CODUOSP_TIER_RESPAWN,
      CODUOSP_TIER_RESPAWN },
    { "mp_info_player_deathmatch", CODUOSP_TIER_DEATHMATCH,
      CODUOSP_TIER_DEATHMATCH }
};

static int32_t coduosp_deathStartTime[MAX_CLIENTS];

static int32_t coduosp_spawn_tier(const gentity_t *ent, qboolean respawn)
{
    const char *classname;

    if (ent == NULL || ent->scriptClassname == 0) {
        return -1;
    }
    classname = SL_ConvertToString(ent->scriptClassname);
    if (classname == NULL) {
        return -1;
    }
    for (size_t index = 0;
         index < sizeof(coduospSpawnClassnames) /
                     sizeof(coduospSpawnClassnames[0]);
         ++index) {
        if (strcmp(classname, coduospSpawnClassnames[index].classname) ==
            0) {
            return respawn != qfalse
                       ? coduospSpawnClassnames[index].respawnTier
                       : coduospSpawnClassnames[index].initialTier;
        }
    }
    return -1;
}

static int32_t coduosp_rand_pick(int32_t count)
{
    if (count <= 1) {
        return 0;
    }
    return coduo_server_randrange(0, count);
}

/* NOT_FROM_ORIGINAL_SOURCE: choose a spawn point for the local player. The
 * scan covers every world entity the map loader kept; the best populated
 * preference tier is sampled with the shared image-local random provider. */
static qboolean coduosp_find_spawn_point(qboolean respawn, vec3_t origin,
                                         vec3_t angles)
{
    const gentity_t *candidates[CODUOSP_SPAWN_CANDIDATE_LIMIT];
    int32_t candidateCount = 0;
    int32_t bestTier = CODUOSP_TIER_COUNT;

    for (int32_t entityNum = level.maxclients;
         entityNum < level.num_entities; ++entityNum) {
        const gentity_t *const ent = &g_entities[entityNum];
        const int32_t tier = coduosp_spawn_tier(ent, respawn);

        if (tier < 0) {
            continue;
        }
        if (tier < bestTier) {
            bestTier = tier;
            candidateCount = 0;
        }
        if (tier == bestTier &&
            candidateCount < CODUOSP_SPAWN_CANDIDATE_LIMIT) {
            candidates[candidateCount++] = ent;
        }
    }

    if (candidateCount == 0) {
        /* NOT_FROM_ORIGINAL_SOURCE: keep the game playable on maps without
         * recognized spawn entities instead of leaving the player unspawned. */
        G_Printf(
            "WARNING: no single-player spawn point found; using map origin\n");
        origin[0] = 0.0f;
        origin[1] = 0.0f;
        origin[2] = 0.0f;
        angles[0] = 0.0f;
        angles[1] = 0.0f;
        angles[2] = 0.0f;
        return qfalse;
    }

    {
        const gentity_t *const chosen =
            candidates[coduosp_rand_pick(candidateCount)];

        origin[0] = chosen->currentOrigin[0];
        origin[1] = chosen->currentOrigin[1];
        origin[2] = chosen->currentOrigin[2];
        angles[0] = 0.0f;
        angles[1] = chosen->currentAngles[1];
        angles[2] = 0.0f;
    }
    return qtrue;
}

void coduosp_client_begin(gentity_t *ent)
{
    vec3_t origin;
    vec3_t angles;

    if (ent == NULL || ent->client == NULL) {
        return;
    }
    (void)coduosp_find_spawn_point(qfalse, origin, angles);
    ClientSpawn(ent, origin, angles);
}

void coduosp_client_died(gentity_t *ent)
{
    int32_t clientNum;

    if (ent == NULL || ent->client == NULL) {
        return;
    }
    clientNum = (int32_t)(ent - g_entities);
    if (clientNum < 0 || clientNum >= level.maxclients ||
        clientNum >= MAX_CLIENTS) {
        return;
    }
    ent->client->sessionState = SESS_STATE_DEAD;
    coduosp_deathStartTime[clientNum] = level.time;
}

void coduosp_run_frame(void)
{
    for (int32_t clientNum = 0; clientNum < level.maxclients; ++clientNum) {
        gentity_t *const ent = &g_entities[clientNum];
        const gclient_t *const client = &level.clients[clientNum];

        if (client->connectedState != CON_CONNECTED ||
            client->sessionState != SESS_STATE_DEAD) {
            continue;
        }
        if (coduosp_deathStartTime[clientNum] == 0 ||
            level.time - coduosp_deathStartTime[clientNum] <
                CODUOSP_RESPAWN_DELAY_MS) {
            continue;
        }
        /* A script respawn or level transition clears the recorded death;
         * only a still-dead client on the timer respawns here. */
        coduosp_deathStartTime[clientNum] = 0;
        {
            vec3_t origin;
            vec3_t angles;

            (void)coduosp_find_spawn_point(qtrue, origin, angles);
            ClientSpawn(ent, origin, angles);
        }
    }
}
