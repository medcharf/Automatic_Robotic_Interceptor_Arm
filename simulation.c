#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <time.h>
#endif

/* World measurements are in metres; the display maps them to terminal cells. */
#define WORLD_WIDTH 18.0f
#define WORLD_HEIGHT 10.0f
#define BALL_RADIUS 0.30f
#define GRAVITY 9.81f
#define FLOOR_RESTITUTION 0.78f
#define AIR_DRAG 0.015f
#define REST_SPEED 0.22f

#define SCREEN_COLS 76
#define SCREEN_ROWS 24
#define PHYSICS_HZ 120
#define FRAME_HZ 60

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    unsigned int bounces;
} Ball;

static void wait_for_frame(void)
{
#ifdef _WIN32
    Sleep(1000 / FRAME_HZ);
#else
    struct timespec delay = {0, 1000000000L / FRAME_HZ};
    nanosleep(&delay, NULL);
#endif
}

static int should_quit(void)
{
#ifdef _WIN32
    if (_kbhit()) {
        int key = _getch();
        return key == 'q' || key == 'Q' || key == 27;
    }
#endif
    return 0;
}

static void update_ball(Ball *ball, float dt)
{
    /* A resting ball stays on the floor instead of receiving a new tiny impact. */
    if (ball->y <= BALL_RADIUS && ball->vy == 0.0f) {
        ball->y = BALL_RADIUS;
        return;
    }

    /* Semi-implicit Euler keeps the acceleration and collision response stable. */
    ball->vy -= GRAVITY * dt;
    ball->vx *= 1.0f - AIR_DRAG * dt;
    ball->x += ball->vx * dt;
    ball->y += ball->vy * dt;

    if (ball->y - BALL_RADIUS < 0.0f) {
        float rebound_speed;

        ball->y = BALL_RADIUS;
        rebound_speed = -ball->vy * FLOOR_RESTITUTION;
        if (rebound_speed >= REST_SPEED) {
            ball->vy = rebound_speed;
            ball->bounces++;
        } else {
            ball->vy = 0.0f;
        }
    }
}

static int world_to_col(float x)
{
    return 1 + (int)(x / WORLD_WIDTH * (SCREEN_COLS - 3));
}

static int world_to_row(float y)
{
    return 1 + (int)((WORLD_HEIGHT - y) / WORLD_HEIGHT * (SCREEN_ROWS - 3));
}

static void render(const Ball *ball, float time)
{
    char screen[SCREEN_ROWS][SCREEN_COLS + 1];
    int ball_col = world_to_col(ball->x);
    int ball_row = world_to_row(ball->y);
    int shadow_col = world_to_col(ball->x);
    int shadow_row = SCREEN_ROWS - 2;

    for (int row = 0; row < SCREEN_ROWS; row++) {
        for (int col = 0; col < SCREEN_COLS; col++) {
            screen[row][col] = ' ';
        }
        screen[row][SCREEN_COLS] = '\0';
    }

    for (int col = 0; col < SCREEN_COLS; col++) {
        screen[SCREEN_ROWS - 2][col] = '=';
    }

    if (shadow_col > 0 && shadow_col < SCREEN_COLS - 1 && ball_row < shadow_row - 1) {
        screen[shadow_row][shadow_col] = '.';
    }
    if (ball_col > 0 && ball_col < SCREEN_COLS - 1 && ball_row > 0 && ball_row < SCREEN_ROWS - 2) {
        screen[ball_row][ball_col] = 'O';
    }

    printf("\x1b[H");
    printf("BALL PHYSICS  |  gravity: %.2f m/s^2  |  press Q or Esc to quit\n", GRAVITY);
    for (int row = 0; row < SCREEN_ROWS - 1; row++) {
        puts(screen[row]);
    }
    printf("t %5.2fs  pos(%5.2f,%5.2f)m  vel(%6.2f,%6.2f)m/s  bounces %u\n",
           time, ball->x, ball->y, ball->vx, ball->vy, ball->bounces);
    fflush(stdout);
}

int main(void)
{
    Ball ball = {
        .x = 2.0f,
        .y = 8.0f,
        .vx = 0.0f,
        .vy = 1.0f,
        .bounces = 0
    };
    const float physics_dt = 1.0f / PHYSICS_HZ;
    const int steps_per_frame = PHYSICS_HZ / FRAME_HZ;
    float time = 0.0f;

#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(console, &mode)) {
        SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif

    printf("\x1b[2J\x1b[H\x1b[?25l");
    while (!should_quit()) {
        for (int step = 0; step < steps_per_frame; step++) {
            update_ball(&ball, physics_dt);
            time += physics_dt;
        }
        render(&ball, time);
        wait_for_frame();
    }
    printf("\x1b[?25h\nSimulation ended after %.2f seconds with %u floor bounces.\n", time, ball.bounces);
    return EXIT_SUCCESS;
}
