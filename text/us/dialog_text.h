// Parameters: dialog enum ID, (unused), lines per box, left offset, width
// temporary

#define END_DIALOG {NULL, NULL, 0, -1, -1}

u8 name_unknown[] = { _("???") };
u8 name_monobup[] = { _("Monobup") };

u8 name_mario[] =   { _("Mario Naegi") };
u8 name_kyoko[] =   { _("Kyoko Spinidrifti") };
u8 name_leon[] =    { _("Leon Blizzwata") };
u8 name_celeste[] = { _("Celestia Boodenburg") };
u8 name_hifumi[] =  { _("Hifumi Bullyada") };
u8 name_miyuto[] =  { _("Miyuto Kurumi") };
u8 name_tobu[] =    { _("Tobu Otoko") };
u8 name_ongaku[] =  { _("Ongaku Meishu") };
u8 name_rifuto[] =  { _("Rifuto Popoi") };
u8 name_ai[] =      { _("Ai Kumo") };
u8 name_okane[] =   { _("Okane Fukuro") };
u8 name_eiga[] =    { _("Eiga Meka") };
u8 name_akuno[] =   { _("Akuno Kaze") };
u8 name_yasai[] =   { _("Yasai Nofu") };
u8 name_janai[] =   { _("Janai Raibu") };
u8 name_romuko[] =  { _("Romuko Gasahiro") };

u8 dialog_test_monobup_1[] = { _("Pupupupuh...")};
u8 dialog_test_janai_1[] = { _("You absolute moron, you idiot, you fucking baboon./nFuck you")};

struct DialogBox dialog_test[] = {
    {name_monobup, dialog_test_monobup_1, 0, DIALOG_HEADER_ORANGE, -1},
    {name_kyoko, dialog_test_janai_1, 2, DIALOG_HEADER_ORANGE, -1},
    END_DIALOG,
};

u8 dialog_hifumi_intro_1[] = { _("Hey, you're Mario, right? I've heard so, so, sooo much about you! I've seen you all over for so many years! Would you mind if I got your autograph??") };
u8 dialog_hifumi_intro_2[] = { _("Ohhh, this is so exciting!! I'm /cyHifumi Bullyada/cw by the way. It's so exciting to meet you!!") };

struct DialogBox dialog_hifumi_intro[] = {
    {name_hifumi, dialog_hifumi_intro_1, 0, DIALOG_HEADER_ORANGE, -1},
    {name_hifumi, dialog_hifumi_intro_2, 0, DIALOG_HEADER_ORANGE, -1},
    END_DIALOG,
};

u8 dialog_game_start_1[] = { _("/cg...") };
u8 dialog_game_start_2[] = { _("/cgHow did I get here?") };
u8 dialog_game_start_3[] = { _("/cgAll I did was walk through the doors to Peach's Castle, as I have a hundred times before...") };
u8 dialog_game_start_4[] = { _("/cgShe said she was hosting a party, how could I not come?") };
u8 dialog_game_start_5[] = { _("/cgNow I fell... asleep?") };
u8 dialog_game_start_6[] = { _("/cgAll there is here is this void and that door up there...") };

struct DialogBox dialog_game_start[] = {
    {name_mario, dialog_game_start_1, 0, DIALOG_HEADER_ORANGE, -1},
    {name_mario, dialog_game_start_2, 0, DIALOG_HEADER_GREEN, -1},
    {name_mario, dialog_game_start_3, 0, DIALOG_HEADER_BLUE, -1},
    {name_mario, dialog_game_start_4, 0, DIALOG_HEADER_BLUE, -1},
    {name_mario, dialog_game_start_5, 0, DIALOG_HEADER_GREEN, -1},
    {name_mario, dialog_game_start_6, 0, DIALOG_HEADER_ORANGE, DIALOG_PROC_PLAY_MUSIC_BBH},
    END_DIALOG,
};

u8 name_jukebox[] =  { _("Jukebox") };
u8 dialog_change_music_1[] = { _("What song would you like to play?") };

u8 option_music_bbh[] = { _("Big Boo's Haunt") };
u8 option_music_koopa_final[] = { _("Ultimate Koopa") };
u8 option_music_file_select[] = { _("File Select") };
u8 option_music_ddd[] = { _("Dire, Dire Docks") };

struct DialogBox dialog_change_music[] = {
    {name_jukebox, dialog_change_music_1, 0, DIALOG_HEADER_GREEN, -1},
    END_DIALOG,
};

struct DialogOption options_change_music[] = {
    {option_music_bbh, DIALOG_CHANGE_MUSIC_BBH},
    {option_music_koopa_final, DIALOG_CHANGE_MUSIC_KOOPA_FINAL},
    {option_music_file_select, DIALOG_CHANGE_MUSIC_FILE_SELECT},
    {option_music_ddd, DIALOG_CHANGE_MUSIC_DDD},
    {NULL, 0},
};

u8 dialog_change_music_bbh_1[] = { _("Playing Big Boo's Haunt...") };
u8 dialog_change_music_koopa_final_1[] = { _("Playing Ultimate Koopa...") };
u8 dialog_change_music_file_select_1[] = { _("Playing File Select...") };
u8 dialog_change_music_ddd_1[] = { _("Playing Dire, Dire Docks...") };

struct DialogBox dialog_change_music_bbh[] = {
    {name_jukebox, dialog_change_music_bbh_1, 0, DIALOG_HEADER_GREEN, DIALOG_PROC_PLAY_MUSIC_BBH},
    END_DIALOG,
};

struct DialogBox dialog_change_music_koopa_final[] = {
    {name_jukebox, dialog_change_music_koopa_final_1, 0, DIALOG_HEADER_GREEN, DIALOG_PROC_PLAY_MUSIC_SLIDE},
    END_DIALOG,
};

struct DialogBox dialog_change_music_file_select[] = {
    {name_jukebox, dialog_change_music_file_select_1, 0, DIALOG_HEADER_GREEN, DIALOG_PROC_PLAY_MUSIC_FILE_SELECT},
    END_DIALOG,
};

struct DialogBox dialog_change_music_ddd[] = {
    {name_jukebox, dialog_change_music_ddd_1, 0, DIALOG_HEADER_GREEN, DIALOG_PROC_PLAY_MUSIC_DDD},
    END_DIALOG,
};
