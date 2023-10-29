#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define SCALE_FACTOR 1.6

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int velocity;
    int direction;
    int speed;
    bool jumping;
    bool walking;
} Character;

typedef enum {
    PLATFORM,
    FLOOR,
} PlatformType;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    PlatformType type;
} Platform;

float rand_float() {
    return (float) ( (float) rand() / (float) RAND_MAX );
}

int character_on_platform( Character character, Platform platforms[], int count ) {
    for( int i = 0; i < count; i++ ) {
        Rectangle platform_rec = {
            .x = platforms[i].x,
            .y = platforms[i].y,
            .width = platforms[i].width,
            .height = platforms[i].height,
        };

        Rectangle character_rec = {
            .x = character.x + 10,
            .y = character.y + character.height - character.height * 0.2,
            .width = character.width - 15,
            .height = character.height * 0.2 + 1,
        };

        if( CheckCollisionRecs( character_rec, platform_rec ) ) {
            return i;
        }
    }

    return -1;
}

int main() {
    srand(time(NULL));

    int window_width = 800 * SCALE_FACTOR;
    int window_height = 600 * SCALE_FACTOR;

    InitWindow( window_width, window_height, "My Game" );
    SetTargetFPS( 60 );

    Character character = {
        .x = window_width/2,
        .y = window_height/2,
        .width = 101,
        .height = 260,
        .velocity = 4 * SCALE_FACTOR,
        .speed = 6,
        .walking = false,
        .jumping = false,
    };

    int gravity = 2 * SCALE_FACTOR;

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

    int world_width = window_width * 10;
    int platform_width = 180;
    int platform_count = world_width / ( platform_width + platform_spacing * window_width );
    int floor_piece_width = 490;
    int floor_piece_height = 190;
    int floor_piece_count = ceil( (float) world_width / (float) floor_piece_width );
    int floor_whitespace = 33;

    int platform_height = 50;
    int platform_min_y = window_height * 0.2;
    int platform_max_y = window_height - floor_piece_height - platform_height - platform_min_y;
    int platform1_whitespace = 45;
    int platform2_whitespace = 20;

    platform_count += floor_piece_count;

    Platform platforms[platform_count+1];

    Texture2D background_texture = LoadTexture( "img/background.png" );
    int background_width = 1792;
    int background_overflow = background_width - window_width;
    float background_ratio = 1 / ((float)(world_width - window_width) / (float)background_overflow);
    int background_x = 0;

    Image platform1_image = LoadImage( "img/platform1.png" );
    Texture2D platform1_texture = LoadTextureFromImage( platform1_image );
    UnloadImage( platform1_image );

    Image platform2_image = LoadImage( "img/platform2.png" );
    Texture2D platform2_texture = LoadTextureFromImage( platform2_image );
    UnloadImage( platform2_image );

    // Create floor
    Image floor_image = LoadImage( "img/floor.png" );
    Texture2D floor_piece_texture = LoadTextureFromImage( floor_image );
    UnloadImage( floor_image );

    int i = 0;
    int floor_x = 0;
    for( ; i < floor_piece_count; i++ ) {
        platforms[i].x = floor_x;
        platforms[i].y = window_height - floor_piece_height + floor_whitespace;
        platforms[i].width = floor_piece_width;
        platforms[i].height = floor_piece_height;
        platforms[i].type = FLOOR;

        floor_x += platforms[i].width;
    }

    int platform_x = window_width * 0.1;
    for( ; i <= platform_count; i++ ) {
        platforms[i].x = platform_x;
        platforms[i].y = rand_float() * platform_max_y + platform_min_y;
        platforms[i].width = platform_width;
        platforms[i].height = platform_height;
        platforms[i].type = PLATFORM;

        platform_x += platforms[i].width + window_width * platform_spacing;
    }

    Image char_stand_img = LoadImage( "img/standing.png" );
    Texture2D char_stand_right = LoadTextureFromImage( char_stand_img );
    ImageFlipHorizontal( &char_stand_img );
    Texture2D char_stand_left = LoadTextureFromImage( char_stand_img );
    UnloadImage( char_stand_img );

    Image char_jump_img = LoadImage( "img/jumping.png" );
    Texture2D char_jump_right = LoadTextureFromImage( char_jump_img );
    ImageFlipHorizontal( &char_jump_img );
    Texture2D char_jump_left = LoadTextureFromImage( char_jump_img );
    UnloadImage( char_jump_img );

    Image char_walk1_img = LoadImage( "img/walk1.png" );
    Texture2D char_walk1_right = LoadTextureFromImage( char_walk1_img );
    ImageFlipHorizontal( &char_walk1_img );
    Texture2D char_walk1_left = LoadTextureFromImage( char_walk1_img );
    UnloadImage( char_walk1_img );

    Image char_walk2_img = LoadImage( "img/walk2.png" );
    Texture2D char_walk2_right = LoadTextureFromImage( char_walk2_img );
    ImageFlipHorizontal( &char_walk2_img );
    Texture2D char_walk2_left = LoadTextureFromImage( char_walk2_img );
    UnloadImage( char_walk2_img );

    while( ! WindowShouldClose() ) {
        BeginDrawing();

        character.walking = false;

        if( character.x > window_width * 0.6 ) {
            camera.offset.x = -(character.x - window_width * 0.6);

        } else if( character.x < window_width * 0.4 ) {
            camera.offset.x = -(character.x - window_width * 0.4);
        }

        if( camera.offset.x > 0 ) {
            camera.offset.x = 0;
        }

        if( camera.offset.x < -(world_width - window_width) ) {
            camera.offset.x = -(world_width - window_width);
        }

        BeginMode2D( camera );

        background_x = -camera.offset.x;
        background_x -= background_x * background_ratio;

        character.y += character.velocity;
        character.velocity += gravity;

        int current_platform = character_on_platform(
            character, platforms, platform_count
        );

        if( current_platform != -1 ) {
            if( character.velocity > 0 ) {
                character.y = platforms[current_platform].y - character.height;
                character.velocity = 0;
                character.jumping = false;
            }

            if( IsKeyPressed( KEY_SPACE ) ) {
                character.velocity = -30 * SCALE_FACTOR;
                character.jumping = true;
            }
        }

        if( IsKeyDown( KEY_LEFT ) ) {
            character.walking = true;
            character.x -= character.speed * SCALE_FACTOR;
            character.direction = -1;
        }

        if( IsKeyDown( KEY_RIGHT ) ) {
            character.walking = true;
            character.x += character.speed * SCALE_FACTOR;
            character.direction = 1;
        }

        ClearBackground( WHITE );

        DrawTexture( background_texture, background_x, 0, WHITE );

        for( int i = 0; i < platform_count; i++ ) {
            if( platforms[i].type == FLOOR ) {
                DrawTexture( floor_piece_texture, platforms[i].x, platforms[i].y - floor_whitespace, WHITE );
            } else {
                Texture2D platform_texture = platform1_texture;
                int whitespace = platform1_whitespace;
                if( i % 2 == 0 ) {
                    platform_texture = platform2_texture;
                    whitespace = platform2_whitespace;
                }
                DrawTexture( platform_texture, platforms[i].x - 10, platforms[i].y - whitespace, WHITE );
            }
        }

        Texture2D char_texture;
        if( character.jumping ) {
            if( character.direction == -1 ) {
                char_texture = char_jump_left;
            } else {
                char_texture = char_jump_right;
            }
        } else if( character.walking ) {
            double time = GetTime() * 10;
            if( character.direction == -1 ) {
                if( ((int) time) % 2 == 0 ) {
                    char_texture = char_walk1_left;
                } else {
                    char_texture = char_walk2_left;
                }
            } else {
                if( ((int) time) % 2 == 0 ) {
                    char_texture = char_walk1_right;
                } else {
                    char_texture = char_walk2_right;
                }
            }
        } else {
            if( character.direction == -1 ) {
                char_texture = char_stand_left;
            } else {
                char_texture = char_stand_right;
            }
        }

        if( character.x < 0 ) {
            character.x = 0;
        }

        if( character.x > world_width - character.width ) {
            character.x = world_width - character.width;
        }

        DrawTexture( char_texture, character.x, character.y, WHITE );

        EndMode2D();
        EndDrawing();
    }

    UnloadTexture( char_stand_right );
    UnloadTexture( char_stand_left );
    UnloadTexture( char_jump_right );
    UnloadTexture( char_jump_left );
    UnloadTexture( char_walk1_right );
    UnloadTexture( char_walk1_left );
    UnloadTexture( char_walk2_right );
    UnloadTexture( char_walk2_left );
    UnloadTexture( floor_piece_texture );
    UnloadTexture( platform1_texture );
    UnloadTexture( platform2_texture );
    UnloadTexture( background_texture );

    return 0;
}
