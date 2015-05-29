#include <nds.h>
#include "ball.h"

#define c_gravity		   80				 // gravity constant (add to vertical velocity) (*.8 fixed)

#define c_air_friction	  15				  // friction in the air... multiply X velocity by (256-f)/256
#define c_ground_friction   15				 // friction when the ball hits the ground, multiply X by (256-f)/256
#define c_platform_level	((192-64) << 8)	// the level of the brick platform in *.8 fixed point
#define c_bounce_damper	 20				 // the amount of Y velocity that is absorbed when you hit the ground

#define min_height		  (1200)			 // the minimum height of the ball (when it gets squished) (*.8)

#define min_yvel			(1200)			 // the minimum Y velocity (*.8)
#define max_xvel			(1000<<4)		  // the maximum X velocity (*.12)


//-----------------------------------------------------------------
// clamp integer to range
//-----------------------------------------------------------------
static inline int clampint( int value, int low, int high )
//-----------------------------------------------------------------
{
	if( value < low ) value = low;
	if( value > high) value = high;
	return value;
}

//-----------------------------------------------------------------
// update ball object (call once per frame)
//-----------------------------------------------------------------
void ballUpdate( ball* b )
//-----------------------------------------------------------------
{
	// add X velocity to X position
	b->x += (b->xvel>>4);

	// apply air friction to X velocity
	b->xvel = (b->xvel * (256-c_air_friction)) >> 8;
	
	// clamp X velocity to the limits
	b->xvel = clampint( b->xvel, -max_xvel, max_xvel );

	// add gravity to Y velocity
	b->yvel += c_gravity;
	b->y += (b->yvel);

	if( b->y >= c_platform_level )
	{
		// apply ground friction to X velocity
		// (yes this may be done multiple times)
		b->xvel = (b->xvel * (256-c_ground_friction)) >> 8;
		
		// check if the ball has been squished to minimum height
		if( b->y > c_platform_level )
		{
			// mount Y on platform
			b->y = c_platform_level;
		
		}
	   
		// calculate the height
		b->height = (c_platform_level - b->y) * 2;
	}
	else
	{
		
		b->height = c_platform_level<< 8;
	}

}

void ballRender( ball* b, int camera_x, int camera_y )
{
	u16* sprite = OAM + b->sprite_index * 4;

	int x, y;
    x = (b->x  >> 8) - camera_x;
    y = (b->y  >> 8) - camera_y;

	if( x <= -16 || y <= -16 || x >= 256 || y >= 192 )
   	{
		// sprite is out of bounds
		// disable the sprite
		sprite[0] = ATTR0_DISABLED;
		return;
	}

	sprite[0] = y & 255;
	sprite[1] = (x & 511) | ATTR1_SIZE_16;

	sprite[2] = 0;
}
