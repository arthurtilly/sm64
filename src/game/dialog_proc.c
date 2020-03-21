#include <ultra64.h>

#include "sm64.h"
#include "audio/external.h"
#include "seq_ids.h"
#include "dialog_ids.h"
#include "ingame_menu.h"

extern DialogProc dialogProcTable[];


void play_music_bbh(void) {
    stop_background_music((4 << 8) | SEQ_LEVEL_SPOOKY);
    play_music(0, (4 << 8) | SEQ_LEVEL_SPOOKY, 0);
}

void play_music_koopa_final(void) {
    stop_background_music((4 << 8) | SEQ_LEVEL_BOSS_KOOPA_FINAL);
    play_music(0, (4 << 8) | SEQ_LEVEL_BOSS_KOOPA_FINAL, 0);
}

void play_music_file_select(void) {
    stop_background_music((4 << 8) | SEQ_MENU_FILE_SELECT);
    play_music(0, (4 << 8) | SEQ_MENU_FILE_SELECT, 0);
}

void play_music_ddd(void) {
    stop_background_music((4 << 8) | SEQ_LEVEL_WATER);
    play_music(0, (4 << 8) | SEQ_LEVEL_WATER, 0);
}


// MUST MATCH dialog_procs.h
DialogProc dialogProcTable[] = {
    play_music_bbh,
    play_music_koopa_final,
    play_music_file_select,
    play_music_ddd,
};
