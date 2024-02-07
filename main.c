//setup
#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct ball{
    float x;
    float y;
    float dx;
    float dy;
    int radius;
}Ball;

typedef struct map{
    int rows;
    int columns;
    float brickWidth;
    float brickHeigth;
    Rectangle **bricks;
}Map;

Rectangle **alocBricksMem(int rows, int columns){
    Rectangle **bricks = calloc(rows, sizeof(Rectangle *));
    for (int i = 0; i < rows; i++){
        bricks[i] = calloc(columns, sizeof(Rectangle));
    }
    return bricks;
}

void freeBricksMem(Map map){
    for (int i = 0; i < map.rows; i++){
        free(map.bricks[i]);
    }
    free(map.bricks);
}

void populateMap(Map map){
    for (int i = 0; i < map.rows; i++){
        for (int j = 0; j < map.columns; j++){
            Rectangle brick = {GetScreenWidth() - (i + 1)*30, j*map.brickHeigth, map.brickWidth, map.brickHeigth};
            map.bricks[i][j] = brick; 
        }
    }
}

void DrawMap(Map map){
    Color brickColors[] = {RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET};
    for (int i = 0; i < map.rows; i++){
        for (int j = 0; j < map.columns; j++){
            DrawRectangleRec(map.bricks[i][j], brickColors[j]);            
        }
    }         
}

void ballCollision(Ball *ball, Rectangle rec) { //peguei do gpt, tem alguma coisa errada no calc eu acho
    // Detect point of collision
    float collisionPointX = ball->x;

    // Calculate the dot product using the relative position
    float angleOfIncidence = acos(0);

    // Calculate the deviation factor based on the collision point's position relative to the center
    float deviationFactor = (collisionPointX - (rec.x + rec.width / 2)) / (rec.width / 2);

    // Calculate the new direction vector with deviation
    float angleOfReflection = PI - angleOfIncidence + deviationFactor;

    // Calculate the new direction vector
    float newBallDX = ball->dx * cos(angleOfReflection) - ball->dy * sin(angleOfReflection);
    float newBallDY = ball->dx * sin(angleOfReflection) + ball->dy * cos(angleOfReflection);

    ball->dx = newBallDX;
    ball->dy = newBallDY;
}

typedef struct {
    Rectangle rect;
    const char *text;
    bool clicked;
} Button;

bool CheckButtonClicked(Rectangle rect){
    Vector2 mouse = GetMousePosition();
    return (CheckCollisionPointRec(mouse, rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

void DrawButton(Button button){
    DrawRectangleRec(button.rect, button.clicked ? BLUE : LIGHTGRAY);
    DrawText(button.text, button.rect.x + 10, button.rect.y + 10, 20, button.clicked ? WHITE : DARKGRAY);
}

int main(){
    //game setup
    int screenWidth = 320, screenHeight = 284;
    InitWindow(screenWidth, screenHeight, "Breakout Simulator");
    InitAudioDevice();
    SetTargetFPS(120);

    //player paddle
    Rectangle player = {GetScreenWidth()/2 - 30, GetScreenHeight() - 20, GetScreenWidth()/8, GetScreenHeight()/32};
    
    Button useArrows = {{screenWidth/2 - 75, 25 + screenHeight/8, 150, 50}, " Use Arrows", false};
    float playerSpeed = 2.0f;
    
    Button useMouse = {{screenWidth/2 - 75, 25 + screenHeight/2, 150, 50}, " Use Mouse", false};
    
    //ball
    float ballX = (GetScreenHeight() - GetScreenHeight()/2.0f), ballY = (GetScreenWidth()/2);
    float ballDX = 0.0, ballDY = 1.0;
    float ballSpeed = 1.001;
    int ballRadius = 3;
    Ball ball = {ballX, ballY, ballDX, ballDY, ballRadius};
    
    //destroy brick sound
    Sound brickSound = LoadSound("/brick.wav");
    
    //map
    int rows = 11, columns = 6;
    float brickWidth = screenWidth/8, brickHeigth = screenHeight/16;
    Map map = {rows, columns, brickWidth, brickHeigth, alocBricksMem(rows, columns)};
    populateMap(map);
    
    bool gameOver = false;
    bool win = false;
    int destroyedBricks = 0;
    int totalBricks = rows * columns;
    
    while (!useArrows.clicked && !useMouse.clicked && !WindowShouldClose()){
        useArrows.clicked = CheckButtonClicked(useArrows.rect);
        useMouse.clicked = CheckButtonClicked(useMouse.rect);
        
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawButton(useArrows);
            DrawButton(useMouse);
        EndDrawing();
    }
    Button difficultyEasy = {{screenWidth/2 - 75, 25 + screenHeight/8, 150, 50}, " Easy", false};
    Button difficultyNormal = {{screenWidth/2 - 75, 42 + screenHeight/4, 150, 50}, " Normal", false};
    Button difficultyHard = {{screenWidth/2 - 75, 25 + screenHeight/2, 150, 50}, " Hard", false};
    
    //choose difficulty
    while(!WindowShouldClose() && !difficultyEasy.clicked && !difficultyNormal.clicked && !difficultyHard.clicked){
        difficultyEasy.clicked = CheckButtonClicked(difficultyEasy.rect);
        difficultyNormal.clicked = CheckButtonClicked(difficultyNormal.rect);
        difficultyHard.clicked = CheckButtonClicked(difficultyHard.rect);
        
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawButton(difficultyEasy);
            DrawButton(difficultyNormal);
            DrawButton(difficultyHard);
        EndDrawing(); 
    }
    float ballSpeedAcm;
    //ajusts the acumilative velocity that the ball gets everytime it hits something
    if (difficultyEasy.clicked){
        ballSpeedAcm = 0.001;
    } else if (difficultyNormal.clicked){
        ballSpeedAcm = 0.003;
    } else {
        ballSpeedAcm = 0.007;
    }   
    //countdown for game start
    for (int countdown = 3; countdown > 0; countdown--){
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText(TextFormat("%d", countdown), screenWidth/2 - 20, screenHeight/2 - 40, 80, RED);
        EndDrawing();   
        WaitTime(1);
    }

    while(!WindowShouldClose() && !gameOver && !win){
        
        //update
        //----------------------------------------------------------------------------------------
        if(useArrows.clicked){
            if(IsKeyDown(KEY_RIGHT)) player.x += playerSpeed;
            if(IsKeyDown(KEY_LEFT)) player.x -= playerSpeed;
        }
        if(useMouse.clicked){
            player.x = GetMouseX() - player.width/2.0f;
        }
        //----------------------------------------------------------------------------------------
        //prevent the player paddle from going out of scenario
        if ((player.x + player.width) >= GetScreenWidth()) player.x = GetScreenWidth() - player.width;
        else if (player.x <= 0) player.x = 0.0;
        
        //apply the velocity to the ball position
        ball.x += ball.dx * ballSpeed;
        ball.y += ball.dy * ballSpeed;
            
        //----------------------------------------------------------------------------------------
        // check collision with player paddle
        Vector2 ballVector = {ball.x, ball.y};
        bool collisionPlayer = CheckCollisionPointRec(ballVector, player);
        if (collisionPlayer){
            ballCollision(&ball, player);
            ballSpeed += ballSpeedAcm;
        }
        
        //check collision with brick
        for (int i = 0; i < map.rows; i++){
            for (int j = 0; j < map.columns; j++){
                if (CheckCollisionPointRec(ballVector, map.bricks[i][j])){
                    ballCollision(&ball, map.bricks[i][j]);
                    (map.bricks[i][j]).x = 0;
                    (map.bricks[i][j]).y = 0;
                    (map.bricks[i][j]).width = 0.0;
                    (map.bricks[i][j]).height = 0.0;
                    destroyedBricks++; //increment the counter to check if all bricks are broken
                    ballSpeed += ballSpeedAcm;
                    PlaySound(brickSound);
                    continue;
                }
            }
        }
        //----------------------------------------------------------------------------------------
        //check if ball hits the left or right of the screen
        if ((ball.x + ball.radius) >= GetScreenWidth()){
            Rectangle border = {ball.x + ball.radius, ball.y, 1, GetScreenHeight()};
            ballCollision(&ball, border);
            ballSpeed += ballSpeedAcm;
        } else if ((ball.x - ball.radius) <= 0){
            Rectangle border = {ball.x - ball.radius, ball.y, 1, GetScreenHeight()};
            ballCollision(&ball, border);
            ballSpeed += ballSpeedAcm;
        }
        //check if the ball hits the top or bottom of the screen
        if ((ball.y + ball.radius) >= GetScreenHeight()){ //loose condition, if the ball hits the bottom
            gameOver = true;
            ball.dx = 0, ball.dy = 0;
            playerSpeed = 0;
            BeginDrawing();
            DrawText("You lost", GetScreenWidth()/2, GetScreenHeight()/2, 20, RED);
            EndDrawing();
                
        } else if ((ball.y - ball.radius) <= 0){
            Rectangle border = {ball.x, ball.y - ball.radius, GetScreenWidth(), 1};
            ballCollision(&ball, border);
            ballSpeed += ballSpeedAcm;
        } 
        //----------------------------------------------------------------------------------------
        //checks if you win
        if(destroyedBricks == totalBricks) {
            BeginDrawing();
            DrawText("You Win", GetScreenWidth()/2, GetScreenHeight()/2, 20, GREEN);
            EndDrawing();
            win = true;
        }
        //----------------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            DrawText(TextFormat("SCORE: %d", destroyedBricks), screenWidth - 80, screenHeight - 15, 15, GOLD);
            DrawCircle(ball.x, ball.y, ball.radius, BLACK);
            DrawRectangleRec(player, BLACK);
            DrawMap(map);            
            
        EndDrawing();
        //----------------------------------------------------------------------------------------
    }  
    freeBricksMem(map);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
