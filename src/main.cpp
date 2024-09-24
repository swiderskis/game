#include "Text.hpp"
#include "Window.hpp"

constexpr auto WINDOW_TITLE = "Game Title";
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 450;
constexpr int TARGET_FPS = 60;

int main()
{
    RWindow window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    window.SetTargetFPS(TARGET_FPS);
    window.SetExitKey(KEY_NULL);

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        window.ClearBackground(GRAY);
        RText::Draw("Congrats! You created your first window!", 190, 200, 20, BLACK);
        window.EndDrawing();
    }

    return 0;
}
