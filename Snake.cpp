#include "raylib.h"
#include <deque>
#include <raymath.h>
#include <iostream>

using namespace std;

static Color Green = {173, 204, 96, 255};
static Color DarkGreen = {43, 51, 24, 255};

static int CellSize{30};
static int CellCount{25};
static int Offset{75};

double LastUpdateTime{0};
bool EventTriggered(double Interval)
{
    double CurrentTime = GetTime();
    if (CurrentTime - LastUpdateTime >= Interval)
    {
        LastUpdateTime = CurrentTime;
        return true;
    }
    return false;
}

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i{0}; i < deque.size(); i++)
    {

        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

class Food
{

public:
    Vector2 Position{};
    Texture2D Texture{};

    Food(deque<Vector2> SnakeBody)
    {
        Image image = LoadImage("Food/food.png");
        Texture = LoadTextureFromImage(image);
        UnloadImage(image);
        Position = GenerateRandomPos(SnakeBody);
    }

    ~Food()
    {
        UnloadTexture(Texture);
    }

    void Draw()
    {
        DrawTexture(Texture, Offset + Position.x * CellSize, Offset + Position.y * CellSize, BLACK);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, CellCount - 1);
        float y = GetRandomValue(0, CellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> Snakebody)
    {
        Vector2 Position = GenerateRandomCell();
        while (ElementInDeque(Position, Snakebody))
        {
            Position = GenerateRandomCell();
        }
        return Position;
    }
};

class Snake
{
public:
    deque<Vector2> Body{Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 Direction{1, 0};
    bool AddSegment{false};

    void Draw()
    {
        for (unsigned int i{0}; i < Body.size(); i++)
        {
            float x = Body[i].x;
            float y = Body[i].y;
            Rectangle segment = Rectangle{Offset + x * CellSize, Offset + y * CellSize, (float)CellSize, (float)CellSize};
            DrawRectangleRounded(segment, 0.5, 6, DarkGreen);
        }
    }

    void Update()
    {
        if (AddSegment == true)
        {
            AddSegment = false;
        }
        else
        {
            Body.pop_back();
        }
        Body.push_front(Vector2Add(Body[0], Direction));
    }

    void Reset()
    {
        Body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        Direction = {1, 0};
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.Body);
    bool running{true};
    int Score{0};
    Sound EatSound{};
    Sound WallSound{};
    Music music{};

    Game()
    {
        InitAudioDevice();
        EatSound = LoadSound("Sounds/eat.mp3");
        WallSound = LoadSound("Sounds/wall.mp3");
        music = LoadMusicStream("Sounds/background.mp3");
        PlayMusicStream(music);
    }

    ~Game()
    {
        UnloadSound(EatSound);
        UnloadSound(WallSound);
        UnloadMusicStream(music);
        CloseAudioDevice();
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.Body[0], food.Position))
        {
            food.Position = food.GenerateRandomPos(snake.Body);
            snake.AddSegment = true;
            Score++;
            PlaySound(EatSound);
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.Body[0].x == CellCount || snake.Body[0].x == -1)
        {
            GameOver();
        }
        if (snake.Body[0].y == CellCount || snake.Body[0].y == -1)
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.Position = food.GenerateRandomPos(snake.Body);
        running = false;
        Score = 0;
        PlaySound(WallSound);
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> HeadlessBody = snake.Body;
        HeadlessBody.pop_front();
        if (ElementInDeque(snake.Body[0], HeadlessBody))
        {
            GameOver();
        }
    }
};

int main()
{
    const char *title{"Snake Game"};

    InitWindow(2 * Offset + CellSize * CellCount, 2 * Offset + CellSize * CellCount, title);
    SetTargetFPS(60);

    Game game = Game();

    while (WindowShouldClose() != true || IsKeyPressed(KEY_X))
    {
        if (game.running)
        {
            UpdateMusicStream(game.music);
        }

        if (EventTriggered(0.2))
        {
            game.Update();
        }

        if (IsKeyPressed(KEY_UP) && game.snake.Direction.y != 1)
        {
            game.snake.Direction = {0, -1};
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.Direction.y != -1)
        {
            game.snake.Direction = {0, 1};
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.Direction.x != 1)
        {
            game.snake.Direction = {-1, 0};
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.Direction.x != -1)
        {
            game.snake.Direction = {1, 0};
        }
        if (IsKeyPressed(KEY_SPACE))
        {
            game.running = true;
        }

        BeginDrawing();
        // Drawing //
        ClearBackground(Green);
        DrawRectangleLinesEx(Rectangle{(float)Offset - 5, (float)Offset - 5, (float)CellSize * CellCount + 10, (float)CellSize * CellCount + 10}, 5, DarkGreen);
        DrawText("Retro Snake", Offset - 5, 20, 40, DarkGreen);
        DrawText(TextFormat("%i", game.Score), Offset - 5, Offset + CellSize * CellCount + 10, 40, DarkGreen);

        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
