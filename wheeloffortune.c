#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <input/input.h>
#include <input/input_event.h>
#include <notification/notification-messages.h>
#include <time.h>

typedef struct {
    char* word_to_guess;
    char* guessed_word;
    int8_t lives;
    uint8_t correct_guesses;
} WheelOfFortuneState;

static char current_char = 'A';

void wheel_of_fortune_draw_callback(Canvas* canvas, void* context) {
    static char current_char_str[2] = {'A', '\0'};
    WheelOfFortuneState* state = context;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 0, state->guessed_word);
    char lives_str[16];
    snprintf(lives_str, sizeof(lives_str), "Lives: %d", state->lives);
    canvas_draw_str(canvas, 0, canvas_font_height(canvas), lives_str);
    current_char_str[0] = current_char;
    canvas_draw_str(canvas, canvas_width(canvas) - canvas_font_width(canvas), 0, current_char_str);
}

bool wheel_of_fortune_input_callback(InputEvent* input_event, void* context) {
    WheelOfFortuneState* state = context;

    if(input_event->type == InputTypeShort && input_event->key == InputKeyBack) {
        return false;
    }

    if(input_event->type == InputTypeShort) {
        switch(input_event->key) {
            case InputKeyUp:
                current_char = (current_char == 'A') ? 'Z' : current_char - 1;
                break;
            case InputKeyDown:
                current_char = (current_char == 'Z') ? 'A' : current_char + 1;
                break;
            case InputKeyOk:
                if(state->lives <= 0 || state->correct_guesses == strlen(state->word_to_guess)) {
                    // Reset the game state
                    state->lives = 6;
                    state->correct_guesses = 0;

                    // Choose a new random word
                    size_t random_index = rand() % word_list_size;
                    state->word_to_guess = word_list[random_index];

                    // Update the guessed_word with underscores based on the length of the new word
                    memset(state->guessed_word, '_', strlen(state->word_to_guess));
                    state->guessed_word[strlen(state->word_to_guess)] = '\0';
                } else {
                    bool correct_guess = false;
                    for(size_t i = 0; i < strlen(state->word_to_guess); i++) {
                        if(state->word_to_guess[i] == current_char && state->guessed_word[i] == '_') {
                            state->guessed_word[i] = current_char;
                            correct_guess = true;
                            state->correct_guesses++;
                        }
                    }

                    if(!correct_guess) {
                        state->lives--;
                    }
                }
                break;
        }
    }

    return true;
}

int32_t wheel_of_fortune_game_app() {
    const char* word_list[] = {
        "ROCKETGOD", "ENCRYPTION", "BADUSB", "MALWARE", "PHISHING",
        "BILLNYEUSESMYWIFI", "SNIFFER", "SPYWARE", "VIRUS", "WORM",
        "ZERO_DAY", "RANSOMWARE", "BOTNET", "EXPLOIT", "CIPHER",
        "AUTHENTICATION", "CRYPTOGRAPHY", "KEYLOGGER", "HACKER", "TROJAN",
        "BRUTEFORCE", "DDOS", "UBERGUIDOZ", "SKELETONMAN", "NANO",
        "FLIPPER_ZERO", "RFID_CLONING", "IR_PROTOCOLS", "I2C", "SPI",
        "GPIO", "UART", "SUB_GHZ", "METASPLOIT",
        "PASSWORD_CRACKING", "HASHCAT", "WIRESHARK",
        "ZED", "INFRARED", "REVERSE_ENGINEERING", "SOCIAL_ENGINEERING",
        "PENTEST", "MAN_IN_THE_MIDDLE", "OSINT", "KALI_LINUX",
    };

    const size_t word_list_size = sizeof(word_list) / sizeof(word_list[0]);

    srand(time(NULL));
    size_t random_index = rand() % word_list_size;
    const char* random_word = word_list[random_index];

    WheelOfFortuneState state = {
        .word_to_guess = random_word,
        .lives = 6,
        .correct_guesses = 0,
    };

    state.guessed_word = malloc(strlen(state.word_to_guess) + 1);
    memset(state.guessed_word, '_', strlen(state.word_to_guess));
    state.guessed_word[strlen(state.word_to_guess)] = '\0';

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, wheel_of_fortune_draw_callback, &state);
    view_port_input_callback_set(view_port, wheel_of_fortune_input_callback, &state);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    view_port_enabled_set(view_port, true);

    while(view_port_enabled_get(view_port)) {
        fu_sleep(10);
    }

    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    free(state.guessed_word);

    return 0;
}