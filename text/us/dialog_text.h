// Parameters: dialog enum ID, (unused), lines per box, left offset, width
// temporary

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
u8 dialog_test_janai_1[] = { _("Shut the fuck up, boomer.")};

struct DialogBox dialog_test[] = {
    {name_monobup, 0, dialog_test_monobup_1},
    {name_janai, 0, dialog_test_janai_1},
    {NULL, 0, NULL},
};

u8 dialog_hifumi_intro_1[] = { _("Hey, you're Mario, right? I've heard so, so, sooo much about you! I've seen you all over for so many years! Would you mind if I got your autograph??") };
u8 dialog_hifumi_intro_2[] = { _("Ohhh, this is so exciting!! I'm /cyHifumi Bullyada/cw by the way. It's so exciting to meet you!!") };

struct DialogBox dialog_hifumi_intro[] = {
    {name_hifumi, 0, dialog_hifumi_intro_1},
    {name_hifumi, 0, dialog_hifumi_intro_2},
    {NULL, 0, NULL},
};

u8 dialog_game_start_1[] = { _("/cg...") };
u8 dialog_game_start_2[] = { _("/cgHow did I get here?") };
u8 dialog_game_start_3[] = { _("/cgAll I did was walk through the doors to Peach's Castle, as I have a hundred times before...") };
u8 dialog_game_start_4[] = { _("/cgShe said she was hosting a party, how could I not come?") };
u8 dialog_game_start_5[] = { _("/cgNow I fell... asleep?") };
u8 dialog_game_start_6[] = { _("/cgAll there is here is this void and that door up there...") };

struct DialogBox dialog_game_start[] = {
    {name_mario, 0, dialog_game_start_1},
    {name_mario, 0, dialog_game_start_2},
    {name_mario, 0, dialog_game_start_3},
    {name_mario, 0, dialog_game_start_4},
    {name_mario, 0, dialog_game_start_5},
    {name_mario, 0, dialog_game_start_6},
    {NULL, 0, NULL},
};

u8 dialog_char_options_1[] = { _("Hmm... How about we test out /cydialogue options?/cw") };
u8 dialog_char_options_2[] = { _("Who is the best character from Danganronpa: Trigger Happy Havoc?") };

u8 option_char_kyoko[] = { _("Kyoko Kirigiri") };
u8 option_char_byakuya[] = { _("Byakuya Togami") };
u8 option_char_hifumi[] = { _("Hifumi Yamada") };
u8 option_char_monokuma[] = { _("Monokuma") };

struct DialogBox dialog_char_options[] = {
    {NULL, 0, dialog_char_options_1},
    {NULL, 0, dialog_char_options_2},
    {NULL, 0, NULL},
};

struct DialogOption options_char_options[] = {
    {option_char_kyoko, DIALOG_CHAR_OPTIONS_KYOKO},
    {option_char_byakuya, DIALOG_CHAR_OPTIONS_BYAKUYA},
    {option_char_hifumi, DIALOG_CHAR_OPTIONS_HIFUMI},
    {option_char_monokuma, DIALOG_CHAR_OPTIONS_MONOKUMA},
    {NULL, 0},
};

u8 dialog_char_options_kyoko_1[] = { _("Not a bad choice, but...") };
u8 dialog_char_options_kyoko_2[] = { _("Don't you think there's a slightly better answer?") };

u8 dialog_char_options_byakuya_1[] = { _("Yes! Yes! That's it! You got it right!") };
u8 dialog_char_options_byakuya_2[] = { _("You truly have the /cyUltimate Correct Opinion!/cw") };

u8 dialog_char_options_hifumi_1[] = { _("Bruh") };

u8 name_monokuma[] =  { _("Monokuma") };
u8 dialog_char_options_monokuma_1[] = { _("Puhuhuhuhu! Aw, you make me blush!") };

struct DialogBox dialog_char_options_kyoko[] = {
    {NULL, 0, dialog_char_options_kyoko_1},
    {NULL, 0, dialog_char_options_kyoko_2},
    {NULL, 0, NULL},
};

struct DialogBox dialog_char_options_byakuya[] = {
    {NULL, 0, dialog_char_options_byakuya_1},
    {NULL, 0, dialog_char_options_byakuya_2},
    {NULL, 0, NULL},
};

struct DialogBox dialog_char_options_hifumi[] = {
    {NULL, 0, dialog_char_options_hifumi_1},
    {NULL, 0, NULL},
};

struct DialogBox dialog_char_options_monokuma[] = {
    {name_monokuma, 0, dialog_char_options_monokuma_1},
    {NULL, 0, NULL},
};
