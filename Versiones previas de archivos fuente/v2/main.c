#include <nds.h>

#include "gfx_ball.h"
#include "gfx_stage1.h"

#define tile_empty 0

#define bg0map    ((u16*)BG_MAP_RAM(1))
#define bg1map    ((u16*)BG_MAP_RAM(2))

#define tile2bgram(t) (BG_GFX + (t) * 16)

#define c_platform_level	((192-48) << 8)	// the level of the brick platform in *.8 fixed point
#define backdrop_colour   RGB8( 0, 0, 0 )
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
	
}

//-----------------------------------------------------------
// update graphical information (call during vblank)
//-----------------------------------------------------------
void updateGraphics( void )
//-----------------------------------------------------------
{
	// update ball sprite, camera = 0, 0
	ballRender( &g_ball, 0, 0 );
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

void setupGraphics(void){
	// setup de vram
	vramSetBankE( VRAM_E_MAIN_BG );
	vramSetBankF( VRAM_F_MAIN_SPRITE ); 
	// generate the first blank tile by clearing it to zero
	int n;
	for( n = 0; n < 16; n++ )
		BG_GFX[n] = 0;
	
	//Cargar sprites
	dmaCopyHalfWords( 3, gfx_stage1Tiles, tile2bgram(1), gfx_stage1TilesLen );

	// Copiar paleta a la memoria
	dmaCopyHalfWords( 3, gfx_stage1Pal, pal2bgram(1), gfx_stage1PalLen );

	// set backdrop color
	BG_PALETTE[0] = backdrop_colour;

	// libnds prefixes the register names with REG_
	REG_BG0CNT = BG_MAP_BASE(1);

    for( n = 0; n < 1024; n++ )
        bg0map[n] = 0;
	
	int i, j;
	// cargamos desde j=4 ya que los tiles de antes contienen basura
	// por razones desconocidas
	for ( i = 0; i < 32; i++)
		for (j=4; j<29; j++)
			bg0map[ i + j*32 ] = (i+j*32+1) | 1 << 12;


	// Si reducimos este valor la imagen baja y aparecen pixeles de basura
	// que no deberían estar. Si se hace algún tipo de elevación vertical
	// habra que tener cuidado.
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
