#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define SCALE_FACTOR 1.6
#define BULLET_SPEED 900
#define MAX_BULLETS 40

typedef struct {
    Vector2 position;
    int size;
    int direction;
} Bullet;

void bullet_draw( Bullet *bullet ) {
    DrawCircleV( bullet->position, bullet->size, YELLOW );
}

typedef struct {
    Texture2D stand_right;
    Texture2D stand_left;
    Texture2D jump_right;
    Texture2D jump_left;
    Texture2D walk1_right;
    Texture2D walk1_left;
    Texture2D walk2_right;
    Texture2D walk2_left;
    Vector2 *gun_position;
} CharacterImage;

void character_image_load( CharacterImage *character_image, char *standing_image_path, char *jumping_image_path, char *walking_image_path1, char *walking_image_path2, Vector2 *gun_position ) {
    Image char_stand_img = LoadImage( standing_image_path );
    Texture2D char_stand_right = LoadTextureFromImage( char_stand_img );
    ImageFlipHorizontal( &char_stand_img );
    Texture2D char_stand_left = LoadTextureFromImage( char_stand_img );
    UnloadImage( char_stand_img );

    Image char_jump_img = LoadImage( jumping_image_path );
    Texture2D char_jump_right = LoadTextureFromImage( char_jump_img );
    ImageFlipHorizontal( &char_jump_img );
    Texture2D char_jump_left = LoadTextureFromImage( char_jump_img );
    UnloadImage( char_jump_img );

    Image char_walk1_img = LoadImage( walking_image_path1 );
    Texture2D char_walk1_right = LoadTextureFromImage( char_walk1_img );
    ImageFlipHorizontal( &char_walk1_img );
    Texture2D char_walk1_left = LoadTextureFromImage( char_walk1_img );
    UnloadImage( char_walk1_img );

    Image char_walk2_img = LoadImage( walking_image_path2 );
    Texture2D char_walk2_right = LoadTextureFromImage( char_walk2_img );
    ImageFlipHorizontal( &char_walk2_img );
    Texture2D char_walk2_left = LoadTextureFromImage( char_walk2_img );
    UnloadImage( char_walk2_img );

    character_image->jump_left = char_jump_left;
    character_image->jump_right = char_jump_right;
    character_image->stand_left = char_stand_left;
    character_image->stand_right = char_stand_right;
    character_image->walk1_left = char_walk1_left;
    character_image->walk1_right = char_walk1_right;
    character_image->walk2_left = char_walk2_left;
    character_image->walk2_right = char_walk2_right;
    character_image->gun_position = gun_position;
}

typedef struct {
    Bullet bullets[MAX_BULLETS];
    int index;
} Bullets;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int velocity;
    int direction;
    int speed;
    int jump_strength;
    bool jumping;
    bool walking;
    CharacterImage image;
    int jump_key;
    bool should_reappear;
    Bullets *bullets;
} Character;

void character_shoot( Character *character ) {
    if( ! character->image.gun_position ) {
        printf( "ERROR: No gun position set for character\n" );
        return;
    }

    printf( "INFO: Shooting bullet\n" );

    int gun_position_x = character->image.gun_position->x;
    if( character->direction == -1 ) {
        gun_position_x = character->width - gun_position_x;
    }

    Bullet bullet = {
        .position = (Vector2){ character->x + gun_position_x, character->y + character->image.gun_position->y },
        .direction = character->direction,
        .size = 15,
    };

    character->bullets->bullets[character->bullets->index] = bullet;
    character->bullets->index = (character->bullets->index + 1) % MAX_BULLETS;
}

void character_destroy( Character *character ) {
    UnloadTexture( character->image.stand_right );
    UnloadTexture( character->image.stand_left );
    UnloadTexture( character->image.jump_right );
    UnloadTexture( character->image.jump_left );
    UnloadTexture( character->image.walk1_right );
    UnloadTexture( character->image.walk1_left );
    UnloadTexture( character->image.walk2_right );
    UnloadTexture( character->image.walk2_left );
}

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

    bool game_over = false;

    CharacterImage character_image = {0};
    character_image_load( &character_image, "img/standing.png", "img/jumping.png", "img/walk1.png", "img/walk2.png", NULL );

    CharacterImage monster_image = {0};
    Vector2 monster_gun_position = {
        .x = 400,
        .y = 170,
    };
    character_image_load( &monster_image, "img/monster_right_1.png", "img/monster_right_1.png", "img/monster_right_1.png", "img/monster_right_2.png", &monster_gun_position );

    Bullets character_bullets = {0};
    Character character = {
        .x = window_width/2,
        .y = window_height/2,
        .width = 101,
        .height = 260,
        .velocity = 200 * SCALE_FACTOR,
        .speed = 250,
        .walking = false,
        .jumping = false,
        .jump_strength = 1000,
        .image = character_image,
        .jump_key = KEY_SPACE,
        .direction = 1,
        .should_reappear = false,
        .bullets = &character_bullets,
    };

    Bullets monster_bullets = {0};

    int monster_width = 406;
    Character monster = {
        .x = window_width + 500,
        .y = window_height/2,
        .width = monster_width,
        .height = 339,
        .velocity = 200 * SCALE_FACTOR,
        .speed = 200,
        .walking = true,
        .jumping = false,
        .jump_strength = 1000,
        .image = monster_image,
        .jump_key = KEY_F,
        .direction = -1,
        .should_reappear = true,
        .bullets = &monster_bullets,
    };

    int gravity = 3000 * SCALE_FACTOR;

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

    Character *characters[] = { &character, &monster };
    int characters_count = 2;

    while( ! WindowShouldClose() ) {
        BeginDrawing();

        if( game_over ) {
            ClearBackground( RED );
            DrawText( "Game Over", window_width/2 - 100, window_height/2 - 50, 40, WHITE );
            EndDrawing();
            continue;
        }

        float dt = GetFrameTime();

        if( character.x > window_width * 0.1 ) {
            camera.offset.x = -(character.x - window_width * 0.1);

        } else if( character.x < window_width * 0.05 ) {
            camera.offset.x = -(character.x - window_width * 0.05);
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

        if( IsKeyDown( KEY_LEFT ) ) {
            character.walking = true;
            character.direction = -1;
        } else if( IsKeyDown( KEY_RIGHT ) ) {
            character.walking = true;
            character.direction = 1;
        } else {
            character.walking = false;
        }

        for( int i = 0; i < characters_count; i++ ) {
            if( characters[i]->walking ) {
                characters[i]->x += characters[i]->direction * characters[i]->speed * dt * SCALE_FACTOR;
            }

            characters[i]->y += characters[i]->velocity * dt;
            characters[i]->velocity += gravity * dt;

            int current_platform = character_on_platform(
                *(characters[i]), platforms, platform_count
            );

            if( current_platform != -1 ) {
                if( characters[i]->velocity > 0 ) {
                    characters[i]->y = platforms[current_platform].y - characters[i]->height;
                    characters[i]->velocity = 0;
                    characters[i]->jumping = false;
                }

                if( characters[i]->jump_key && IsKeyPressed( characters[i]->jump_key ) ) {
                    characters[i]->velocity = -characters[i]->jump_strength * SCALE_FACTOR;
                    characters[i]->jumping = true;
                }
            }
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

        for( int i = 0; i < characters_count; i++ ) {
            Texture2D char_texture;
            if( characters[i]->jumping ) {
                if( characters[i]->direction == -1 ) {
                    char_texture = characters[i]->image.jump_left;
                } else {
                    char_texture = characters[i]->image.jump_right;
                }
            } else if( characters[i]->walking ) {
                double time = GetTime() * 10;
                if( characters[i]->direction == -1 ) {
                    if( ((int) time) % 2 == 0 ) {
                        char_texture = characters[i]->image.walk1_left;
                    } else {
                        char_texture = characters[i]->image.walk2_left;
                    }
                } else {
                    if( ((int) time) % 2 == 0 ) {
                        char_texture = characters[i]->image.walk1_right;
                    } else {
                        char_texture = characters[i]->image.walk2_right;
                    }
                }
            } else {
                if( characters[i]->direction == -1 ) {
                    char_texture = characters[i]->image.stand_left;
                } else {
                    char_texture = characters[i]->image.stand_right;
                }
            }

            if( characters[i]->should_reappear ) {
                if( characters[i]->x < -camera.offset.x - 600 ) {
                    characters[i]->x = -camera.offset.x + window_width;
                }
            } else {
                if( characters[i]->x < 0 ) {
                    characters[i]->x = 0;
                }
            }

            if( characters[i]->x > world_width - characters[i]->width ) {
                characters[i]->x = world_width - characters[i]->width;
            }

            DrawTexture( char_texture, characters[i]->x, characters[i]->y, WHITE );
        }

        if( rand_float() < 0.025 ) {
            character_shoot( &monster );
        }

        // Draw bullets
        for( int j = 0; j < characters_count; j++ ) {
            for( int i = 0; i < MAX_BULLETS; i++ ) {
                Bullet *bullet = &characters[j]->bullets->bullets[i];
                bullet->position.x += bullet->direction * BULLET_SPEED * dt;
                bullet_draw( bullet );

                Rectangle character_rec = {
                    .x = character.x,
                    .y = character.y,
                    .width = character.width,
                    .height = character.height,
                };

                if( CheckCollisionCircleRec( bullet->position, bullet->size, character_rec ) ) {
                    game_over = true;
                }
            }
        }

        EndMode2D();
        EndDrawing();
    }

    character_destroy( &character );
    character_destroy( &monster );
    UnloadTexture( floor_piece_texture );
    UnloadTexture( platform1_texture );
    UnloadTexture( platform2_texture );
    UnloadTexture( background_texture );

    return 0;
}
