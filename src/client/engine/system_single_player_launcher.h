#ifndef CODUOMP_SYSTEM_SINGLE_PLAYER_LAUNCHER_H
#define CODUOMP_SYSTEM_SINGLE_PLAYER_LAUNCHER_H

/*
 * NOT_FROM_ORIGINAL_SOURCE: launch the sibling single-player client.
 *
 * The retail multiplayer main menu's Single Player entry launched the retail
 * CoDUOSP.exe. This command restores that launcher behavior for the
 * reconstructed products: it starts the CoDUOSP client that ships beside this
 * executable in the same distribution layout.
 */

void coduosp_launch_single_player(void);

#endif
