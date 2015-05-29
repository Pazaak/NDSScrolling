#ifndef character_H
#define character_H

typedef struct t_character
{
    int x;     // 24.8 fixed point
    int y;     // 24.8 fixed point 
    int xvel;  // 20.12 fixed point
    int yvel;  // 24.8 fixed point
    
    u8  sprite_index;        // OAM entry (0->127)
    u8  sprite_affine_index; // OAM affine entry (0->31)
    
    int height; // height of character
} character;

void characterUpdate( character* b );
void scrollingUpdate( character* b, int scroll );
void characterRender( character* b, int camera_x, int camera_y );

#endif
