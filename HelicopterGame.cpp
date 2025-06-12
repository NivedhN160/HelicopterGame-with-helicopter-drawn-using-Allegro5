#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <iostream>

const int WIDTH = 640;
const int HEIGHT = 480;
const int FPS = 60;

// Helicopter
float heli_x, heli_y;
const int heli_width = 40;
const int heli_height = 20;

// Obstacle
float obs_x, obs_y;
const int obs_width = 8;
const int obs_height = 160;

int score = 0;
int high_score = 0;
bool game_over = false;

void reset_game() {
    heli_x = WIDTH / 4;
    heli_y = HEIGHT / 2;
    obs_x = WIDTH;
    obs_y = 160;
    score = 0;
    game_over = false;
}

bool check_collision() {
    if (heli_y < 0 || heli_y + heli_height > HEIGHT)
        return true;

    if (heli_x + heli_width > obs_x && heli_x < obs_x + obs_width &&
        heli_y + heli_height > obs_y && heli_y < obs_y + obs_height)
        return true;

    return false;
}

int main() {
    if (!al_init()) {
        std::cerr << "Failed to initialize Allegro.\n";
        return -1;
    }

    ALLEGRO_DISPLAY* display = al_create_display(WIDTH, HEIGHT);
    if (!display) {
        std::cerr << "Failed to create display.\n";
        return -1;
    }

    al_install_keyboard();
    al_init_primitives_addon();
    al_init_font_addon();

    ALLEGRO_FONT* font = al_create_builtin_font();
    if (!font) {
        std::cerr << "Failed to create built-in font.\n";
        return -1;
    }

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    reset_game();

    bool redraw = true;
    bool keys[ALLEGRO_KEY_MAX] = { false };

    int throttle = 10;        // More responsive upward motion
    int gravity = 4;          // Faster descent
    int air_resistance = 6;   // Faster tick rate
    int check = 0;
    int check_obs = 0;
    int speed_obs = 3;        // Faster obstacles

    al_start_timer(timer);

    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            check++;
            check_obs++;

            if (!game_over && check >= air_resistance) {
                check = 0;
                if (keys[ALLEGRO_KEY_SPACE])
                    heli_y -= throttle;
                else
                    heli_y += gravity;
            }

            if (!game_over && check_obs >= speed_obs) {
                check_obs = 0;
                obs_x -= 2;

                if (obs_x + obs_width < 0) {
                    obs_x = WIDTH;
                    obs_y += 70;
                    if (obs_y < 0 || obs_y + obs_height > HEIGHT)
                        obs_y = 1;

                    score++;
                    if (score > high_score) high_score = score;

                    // Increase difficulty
                    if (score % 10 == 0 && speed_obs > 1)
                        speed_obs--;
                    else if (score % 5 == 0 && air_resistance > 3)
                        air_resistance--;
                }
            }

            if (!game_over && check_collision()) {
                game_over = true;
            }

            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            keys[ev.keyboard.keycode] = true;

            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                break;

            if (game_over && ev.keyboard.keycode == ALLEGRO_KEY_Y) {
                reset_game();
                speed_obs = 3;
                air_resistance = 6;
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            keys[ev.keyboard.keycode] = false;
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            redraw = false;

            al_clear_to_color(al_map_rgb(25, 155, 200));

            // Draw upgraded helicopter
            al_draw_filled_rectangle(heli_x, heli_y, heli_x + heli_width, heli_y + heli_height, al_map_rgb(205, 100, 100));
            al_draw_filled_circle(heli_x + 30, heli_y + heli_height / 2, 5, al_map_rgb(240, 240, 240)); // nose
            al_draw_filled_rectangle(heli_x - 8, heli_y + 7, heli_x, heli_y + 13, al_map_rgb(200, 200, 200)); // tail rotor
            al_draw_line(heli_x + 10, heli_y, heli_x + 30, heli_y - 10, al_map_rgb(80, 80, 80), 2); // rotor blade

            // Draw obstacle
            al_draw_filled_rectangle(obs_x, obs_y, obs_x + obs_width, obs_y + obs_height, al_map_rgb(205, 20, 20));

            // Scoreboard
            al_draw_textf(font, al_map_rgb(250, 200, 100), WIDTH - 10, HEIGHT - 40, ALLEGRO_ALIGN_RIGHT, "Score: %d", score);
            al_draw_textf(font, al_map_rgb(250, 250, 150), WIDTH - 10, HEIGHT - 20, ALLEGRO_ALIGN_RIGHT, "High Score: %d", high_score);
            al_draw_text(font, al_map_rgb(209, 247, 100), WIDTH / 2, 10, ALLEGRO_ALIGN_CENTER, "Throttle with SPACE. Press ESC to exit");

            if (game_over) {
                al_draw_text(font, al_map_rgb(255, 255, 255), WIDTH / 2, HEIGHT / 2 - 20, ALLEGRO_ALIGN_CENTER, "Game Over! Press Y to retry or ESC to exit");
                al_draw_textf(font, al_map_rgb(255, 200, 200), WIDTH / 2, HEIGHT / 2 + 20, ALLEGRO_ALIGN_CENTER, "Your Score: %d", score);
            }

            al_flip_display();
        }
    }

    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);

    return 0;
}
