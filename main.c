#include "raylib.h"
#include <stdlib.h>
#include <time.h>

typedef struct {
    int x;
    int y;
    float size;
    int velocity;
} Ball;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} Platform;

float rand_float() {
    return (float) ( (float) rand() / (float) RAND_MAX );
}

int ball_on_platform( Ball ball, Platform platforms[], int count ) {
    for( int i = 0; i < count; i++ ) {
        Rectangle platform_rec = {
            .x = platforms[i].x,
            .y = platforms[i].y,
            .width = platforms[i].width,
            .height = platforms[i].height,
        };

        Rectangle ball_rec = {
            .x = ball.x - ball.size + 20,
            .y = ball.y + ball.size / 2,
            .width = ball.size * 2 - 40,
            .height = ball.size / 2 + 1,
        };

        if( CheckCollisionRecs( ball_rec, platform_rec ) ) {
            return i;
        }
    }

    return -1;
}

int main() {
    srand(time(NULL));

    int window_width = 800;
    int window_height = 600;

    Ball ball = {
        .x = window_width/2,
        .y = window_height/2,
        .size = 40.0,
        .velocity = 4,
    };

    int gravity = 2;

    Vector2 camera_offset = {
        .x = 0,
        .y = 0,
    };

    Vector2 camera_target = {
        .x = 0,
        .y = 0,
    };

    Camera2D camera = {
        .offset = camera_offset,
        .target = camera_target,
        .rotation = 0,
        .zoom = 1,
    };

    float platform_spacing = 0.01;
    int platform_min_y = abs( window_height * 0.2 );
    int platform_max_y = abs( window_height * 0.8 );

    int world_width = window_width * 10;
    int platform_width = abs( window_width * 0.3 );
    int platform_count = world_width / ( platform_width + platform_spacing * window_width );

    Platform platforms[platform_count+1];

    // Create floor
    platforms[0].x = 0;
    platforms[0].y = window_height - 10;
    platforms[0].width = window_width * 10;
    platforms[0].height = 10;

    int platform_x = abs( window_width * 0.1 );
    for( int i = 1; i <= platform_count; i++ ) {
        platforms[i].x = platform_x;
        platforms[i].y = abs( rand_float() * platform_max_y ) + platform_min_y;
        platforms[i].width = abs( window_width * 0.3 );
        platforms[i].height = abs( window_height * 0.05 );

        platform_x += platforms[i].width + abs( window_width * platform_spacing );
    }

    InitWindow( window_width, window_height, "My Game" );
    SetTargetFPS( 60 );

    while( ! WindowShouldClose() ) {
        BeginDrawing();
        BeginMode2D( camera );

        if( ball.x > window_width * 0.6 ) {
            camera.offset.x = -(ball.x - window_width * 0.6);
        } else if( ball.x < window_width * 0.4 ) {
            camera.offset.x = -(ball.x - window_width * 0.4);
        }

        if( camera.offset.x > 0 ) {
            camera.offset.x = 0;
        }

        ball.y += ball.velocity;
        ball.velocity += gravity;

        int current_platform = ball_on_platform(
            ball, platforms, platform_count
        );

        if( current_platform != -1 ) {
            if( ball.velocity > 0 ) {
                ball.y = platforms[current_platform].y - ball.size;
                ball.velocity = 0;
            }

            if( IsKeyPressed( KEY_SPACE ) ) {
                ball.velocity = -30;
            }
        }

        if( IsKeyDown( KEY_LEFT ) ) {
            ball.x -= 4;
        }

        if( IsKeyDown( KEY_RIGHT ) ) {
            ball.x += 4;
        }

        ClearBackground( WHITE );

        for( int i = 0; i < platform_count; i++ ) {
            DrawRectangle(
                platforms[i].x,
                platforms[i].y,
                platforms[i].width,
                platforms[i].height,
                BLACK
            );
        }

        DrawCircle( ball.x, ball.y, ball.size, RED );

        EndMode2D();
        EndDrawing();
    }

    return 0;
}
