#include <math.h>
#include <raylib.h>
#include <string.h>

#define WIDTH 900
#define HEIGHT 600

#define LINE_THICK 5.0f
#define MASS_RAD 18.0f
#define L1 220.0f
#define L2 180.0f
#define M1 18.0f
#define M2 18.0f
#define G 900.0f
#define TRAIL_MAX 700

typedef struct PendulumState {
    float theta1;
    float theta2;
    float omega1;
    float omega2;
    Vector2 trail[TRAIL_MAX];
    int trail_count;
} PendulumState;

static const Vector2 start_pos = { WIDTH / 2.0f, 90.0f };

Vector2 get_end(Vector2 start, float length, float theta)
{
    return (Vector2){
        start.x + length * sinf(theta),
        start.y + length * cosf(theta),
    };
}

void reset_state(PendulumState *state)
{
    state->theta1 = 120.0f * DEG2RAD;
    state->theta2 = -10.0f * DEG2RAD;
    state->omega1 = 0.0f;
    state->omega2 = 0.0f;
    state->trail_count = 0;
}

static void push_trail(PendulumState *state, Vector2 point)
{
    if (state->trail_count == TRAIL_MAX) {
        memmove(state->trail, state->trail + 1, sizeof(Vector2) * (TRAIL_MAX - 1));
        state->trail_count--;
    }

    state->trail[state->trail_count] = point;
    state->trail_count++;
}

void step_simulation(PendulumState *state, float dt)
{
    float theta1 = state->theta1;
    float theta2 = state->theta2;
    float omega1 = state->omega1;
    float omega2 = state->omega2;
    float delta = theta1 - theta2;
    float denom = 2.0f * M1 + M2 - M2 * cosf(2.0f * delta);

    float alpha1 =
        (-G * (2.0f * M1 + M2) * sinf(theta1) -
         M2 * G * sinf(theta1 - 2.0f * theta2) -
         2.0f * sinf(delta) * M2 *
             (omega2 * omega2 * L2 + omega1 * omega1 * L1 * cosf(delta))) /
        (L1 * denom);

    float alpha2 =
        (2.0f * sinf(delta) *
         (omega1 * omega1 * L1 * (M1 + M2) +
          G * (M1 + M2) * cosf(theta1) +
          omega2 * omega2 * L2 * M2 * cosf(delta))) /
        (L2 * denom);

    state->omega1 += alpha1 * dt;
    state->omega2 += alpha2 * dt;
    state->theta1 += state->omega1 * dt;
    state->theta2 += state->omega2 * dt;

    Vector2 joint = get_end(start_pos, L1, state->theta1);
    Vector2 bob = get_end(joint, L2, state->theta2);
    push_trail(state, bob);
}

#ifndef UNIT_TEST
static void draw_trail(const PendulumState *state)
{
    for (int i = 1; i < state->trail_count; i++) {
        float alpha = (float)i / (float)state->trail_count;
        Color color = Fade(SKYBLUE, alpha * 0.75f);
        DrawLineEx(state->trail[i - 1], state->trail[i], 2.0f, color);
    }
}

static void draw_arm(Vector2 start, Vector2 end, float radius, Color color)
{
    DrawLineEx(start, end, LINE_THICK, WHITE);
    DrawCircleV(end, radius, color);
}

static void draw_double_pendulum(const PendulumState *state)
{
    Vector2 joint = get_end(start_pos, L1, state->theta1);
    Vector2 bob = get_end(joint, L2, state->theta2);

    draw_trail(state);
    DrawCircleV(start_pos, 6.0f, RAYWHITE);
    draw_arm(start_pos, joint, MASS_RAD, ORANGE);
    draw_arm(joint, bob, MASS_RAD, RED);
}

int main(void)
{
    PendulumState state;

    InitWindow(WIDTH, HEIGHT, "Double Pendulum");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    reset_state(&state);

    while (!WindowShouldClose()) {
        step_simulation(&state, GetFrameTime());

        BeginDrawing();
        ClearBackground((Color){ 12, 14, 18, 255 });
        draw_double_pendulum(&state);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
#endif
