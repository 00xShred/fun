#include <raylib.h>
#include <math.h>
#include <stdio.h>

#define WIDTH 900
#define HEIGHT 600
#define SCALE 2

#define COUNT 14
#define STEP_DUR 0.75f
#define ARC_FRAC 0.60f
#define PAUSE_DUR 2.0f

int nums[COUNT];
Font g_font;

void init_fib() {
    nums[0] = 0;
    nums[1] = 1;
    for (int i = 2; i < COUNT; i++) {
        nums[i] = nums[i-1] + nums[i-2];
    }
}

static Color square_color(int i) {
    float h = 200.0f + (float)i / (COUNT - 2) * 160.0f;
    return ColorFromHSV(h, 0.55f, 0.80f);
}

static float ease_out(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

void draw_fib(float t) {
    Rectangle squares[COUNT - 1];
    int left = 0, top = 0;
    int right = nums[1] * SCALE;
    int bottom = nums[1] * SCALE;

    squares[0] = (Rectangle){ 0, 0, nums[1] * SCALE, nums[1] * SCALE };

    for (int i = 2; i < COUNT; i++) {
        int size = nums[i] * SCALE;
        int x = 0, y = 0;

        switch ((i - 2) % 4) {
            case 0: x = right;        y = bottom - size; right += size;  break;
            case 1: x = left;         y = top - size;    top -= size;    break;
            case 2: x = left - size;  y = top;           left -= size;   break;
            case 3: x = right - size; y = bottom;        bottom += size; break;
        }

        squares[i - 1] = (Rectangle){ x, y, size, size };
    }

    int first_size = nums[1] * SCALE;
    int offset_x = WIDTH  / 2 - first_size / 2;
    int offset_y = HEIGHT / 2 - first_size / 2;

    float cycle = (COUNT - 1) * STEP_DUR + PAUSE_DUR;
    float ct = fmodf(t, cycle);

    int visible = (int)(ct / STEP_DUR);
    if (visible >= COUNT - 1) visible = COUNT - 2;

    float step_t = fmodf(ct, STEP_DUR);
    float arc_raw = step_t / (STEP_DUR * ARC_FRAC);
    if (arc_raw > 1.0f) arc_raw = 1.0f;
    float arc_prog = ease_out(arc_raw);

    for (int i = 0; i <= visible; i++) {
        Rectangle sq = squares[i];
        sq.x += offset_x;
        sq.y += offset_y;

        Color col = square_color(i);

        Color fill = col;
        fill.a = 25;
        DrawRectangleRec(sq, fill);

        Color outline = col;
        outline.a = 140;
        DrawRectangleLinesEx(sq, 1.5f, outline);

        // show number when square is fully settled
        if (i < visible && sq.width >= 22) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", nums[i < 1 ? 1 : i]);
            int fs = (int)(sq.width * 0.22f);
            if (fs < 8) fs = 8;
            if (fs > 28) fs = 28;
            Vector2 tsz = MeasureTextEx(g_font, buf, fs, 1);
            Color tc = col;
            tc.a = 90;
            DrawTextEx(g_font, buf,
                       (Vector2){ sq.x + sq.width / 2 - tsz.x / 2,
                                  sq.y + sq.height / 2 - tsz.y / 2 },
                       fs, 1, tc);
        }

        float start_angle, sweep;
        Vector2 center;
        switch (i % 4) {
            case 0: center = (Vector2){ sq.x + sq.width, sq.y + sq.height }; start_angle = 180; break;
            case 1: center = (Vector2){ sq.x,            sq.y + sq.height }; start_angle = 270; break;
            case 2: center = (Vector2){ sq.x,            sq.y            }; start_angle = 0;   break;
            default:center = (Vector2){ sq.x + sq.width, sq.y            }; start_angle = 90;  break;
        }

        sweep = (i < visible) ? 90.0f : 90.0f * arc_prog;

        if (sweep > 0.0f) {
            Color arc_col = square_color(i);
            arc_col.a = 220;
            DrawCircleSectorLines(center, sq.width, start_angle, start_angle + sweep, 48, arc_col);
        }
    }
}

int main(void) {
    InitWindow(WIDTH, HEIGHT, "Fibonacci Spiral");
    SetWindowState(FLAG_MSAA_4X_HINT);
    g_font = GetFontDefault();
    init_fib();

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){ 8, 10, 18, 255 });
        draw_fib(GetTime());
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
