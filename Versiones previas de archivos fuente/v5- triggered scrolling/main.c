#include <nds.h>
#include <stdio.h>

#include "gfx_ball.h"
#include "gfx_tileset.h"

#define bg0 1

#define bg0map    ((u16*)BG_MAP_RAM(1))

#define pal_bg0 0

#define tile2bgram(t) (BG_GFX + (t) * 16)

#define c_platform_level	((192-64) << 8)	// the level of the brick platform in *.8 fixed point
#define backdrop_colour   RGB8( 72, 234, 248 )
#define pal2bgram(p)	  (BG_PALETTE + (p) * 16)

//sprites
#define tiles_ball	  0 // ball tiles (16x16 tile 0->3)

#define tile2objram(t) (SPRITE_GFX + (t) * 16)

//palette of sprites
#define pal2objram(p) (SPRITE_PALETTE + (p) * 16)
#define pal_ball	  0 // ball palette (entry 0->15)

//display de sprites
typedef struct t_spriteEntry
{
	u16 attr0;
	u16 attr1;
	u16 attr2;
	u16 affine_data;
} spriteEntry;

#define sprites ((spriteEntry*)OAM)

//object ball
#include "ball.h"

ball g_ball;
int scrolling = 0;
int stages[3];
int actualStage;

#define x_tweak	(2<<8)  // for user input
#define y_tweak	800	  // for user input

//-----------------------------------------------------------------
// process input
//-----------------------------------------------------------------
void processInput( void )
//-----------------------------------------------------------------
{
	scanKeys();

	int keysh = keysHeld();
	// process user input
	if( keysh & KEY_UP )	  // check if UP is pressed
	{
		if ( g_ball.y == c_platform_level )
			g_ball.yvel = -y_tweak;
		else if ( (g_ball.yvel < 0) && (g_ball.y < 4 + c_platform_level) )
			g_ball.yvel -= y_tweak/14;
		
	}
	if( keysh & KEY_DOWN )	// check if DOWN is pressed
	{
		// tweak y velocity of ball
		g_ball.yvel += y_tweak;
	}
	if( keysh & KEY_LEFT )	// check if LEFT is pressed
	{
		// tweak x velocity
		g_ball.xvel -= x_tweak;
	}
	if( keysh & KEY_RIGHT )   // check if RIGHT is pressed
	{
		// tweak y velocity
		g_ball.xvel += x_tweak;
	}

}

void processLogic( void )
{
	processInput();
	ballUpdate( &g_ball );

	iprintf("\x1b[2J");
	iprintf("x: %d y: %d", g_ball.x, g_ball.y);
}

//-----------------------------------------------------------
// update graphical information (call during vblank)
//-----------------------------------------------------------
void updateGraphics( void )
//-----------------------------------------------------------
{ /* EN ELLO */
	// update ball sprite, camera = 0, 0
	ballRender( &g_ball, 0, 0 );
	if( g_ball.x >= 61000 && actualStage<2 ) scrolling = 2;
	if( g_ball.x <= 0 && actualStage>0 ){
		if (!scrolling ) REG_BG0HOFS = 256;
		scrolling = 1;	
	}

	if (scrolling == 1){
		REG_BG0HOFS -= 2;
		if ( REG_BG0HOFS <= 0 ){
			scrolling = 0; 
			actualStage -= 1;
		}
	}

	if (scrolling == 2){
		REG_BG0HOFS += 2;
		if ( REG_BG0HOFS >= 256 ){
			scrolling = 0; 
			REG_BG0HOFS = 0;
			actualStage += 1;
		}
	}
		
}

//-----------------------------------------------------------
// reset ball attributes
//-----------------------------------------------------------
void resetBall( void )
//-----------------------------------------------------------
{
	// use sprite index 0 (0->127)
	g_ball.sprite_index = 0;
	
	// use affine matrix 0 (0->31)
	g_ball.sprite_affine_index = 0;
	
	// X = 128.0
	g_ball.x = 128 << 8;
	
	// Y = 64.0
	g_ball.y = 64 << 8;
   
	// start X velocity a bit to the right
	g_ball.xvel = 100 << 4;
   
	// reset Y velocity
	g_ball.yvel = 0;
}


//-----------------------------------------------------------
// setup interrupt handler with vblank irq enabled
//-----------------------------------------------------------
void setupInterrupts( void )
//-----------------------------------------------------------
{
	// initialize interrupt handler
	irqInit();
	
	// enable vblank interrupt (required for swiWaitForVBlank!)
	irqEnable( IRQ_VBLANK );
}

//-----------------------------------------------------------
// HECHO
// queria rellenar un array con booleanos a bit, pero no
// funciona como yo quisiera, por ahora lo dejo así
//-----------------------------------------------------------
void setupStages( void )
//-----------------------------------------------------------
{
	actualStage = 0;
}
void setupGraphics(void){
	// setup de vram
	vramSetBankE( VRAM_E_MAIN_BG );
	vramSetBankF( VRAM_F_MAIN_SPRITE ); 
	// generate the first blank tile by clearing it to zero
	int n;

	for( n=0; n<16; n++)
		BG_GFX[n] = 0;

	consoleDemoInit();
	
	//Cargar sprites
	dmaCopyHalfWords( 3, gfx_tilesetTiles, tile2bgram(bg0), gfx_tilesetTilesLen );

	// Copiar paleta a la memoria
	dmaCopyHalfWords( 3, gfx_tilesetPal, pal2bgram(pal_bg0), gfx_tilesetPalLen );

	// set backdrop color
	BG_PALETTE[0] = backdrop_colour;

	// libnds prefixes the register names with REG_
	REG_BG0CNT = 1 << 14 | 1 << 8; /** IMPORTANTE */

    	for( n = 0; n < 2048; n++ )
        	bg0map[n] = 0; /** IMPORTANTE */
	
	int i, j;
	// Suelo
	// Bloque 1: 	8  9
	//		10 11
	// Bloque 2:	12 13
	//		14 15
	for ( i = 0; i < 32; i++){
		for (j=25; j<29; j+=2)
			if ( (i+j-1)%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
		for (j=26; j<29; j+=2)
			if ( (i+j-1)%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+14+i%2) | pal_bg0 << 12;

	}

	// Plataforma
	// 4 5
	// 6 7
	for ( i = 0; i < 32; i++){
		bg0map[ i + 23*32 ] = (bg0+4+i%2) | pal_bg0 << 12;
		bg0map[ i + 24*32 ] = (bg0+6+i%2) | pal_bg0 << 12;
	}

	// Rocas
	// Roca 1:	16 17
	// 		18 19
	// Roca 2:	20 21
	// 		22 23
	for ( i = 0; i < 8; i++){
		for (j=19; j<23; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+16+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+20+i%2) | pal_bg0 << 12;
		for (j=20; j<23; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+18+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+22+i%2) | pal_bg0 << 12;

	}

	for ( i = 0; i < 4; i++){
		for (j=15; j<19; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+16+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+20+i%2) | pal_bg0 << 12;
		for (j=16; j<19; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+18+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+22+i%2) | pal_bg0 << 12;

	}

	// Escaleras
	// 24 25
	for ( i = 0; i < 2; i++){
		for (j=0; j<15; j++)
			bg0map[ i + j*32 ] = (bg0+24+i%2) | pal_bg0 << 12;
	}

	for( n = 1024; n < 2048; n++ )
        	bg0map[n] = (bg0+24) | pal_bg0 << 12;	

	REG_BG0VOFS = 40;
	
	//Cargar sprites
	dmaCopyHalfWords( 3, gfx_ballTiles, tile2objram(tiles_ball), gfx_ballTilesLen );

	// Copiar paleta a la memoria
	dmaCopyHalfWords( 3, gfx_ballPal, pal2objram(pal_ball), gfx_ballPalLen );

	// deshabilitar sprites
	for( n = 0; n < 128; n++ )
			sprites[n].attr0 = ATTR0_DISABLED;

	videoSetMode( MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT );

}

//-----------------------------------------------------------
// program entry point
//-----------------------------------------------------------
int main( void )
//-----------------------------------------------------------
{
	// setup things
	setupInterrupts();
	setupStages();
	setupGraphics();
	resetBall();

	// start main loop
	while( 1 )
	{
		// update game logic
		processLogic();

		// wait for new frame
		swiWaitForVBlank();

		// update graphics
		updateGraphics();
	}
	return 0;
}
