#include <nds.h>
#include <stdio.h>
#include <stdlib.h>


#include "gfx_nave1.h"
#include "gfx_tileset.h"

#define bg0 1

#define bg0map    ((u16*)BG_MAP_RAM(1))

#define pal_bg0 0

#define tile2bgram(t) (BG_GFX + (t) * 16)

#define c_platform_level	((192-64) << 8)	// the level of the brick platform in *.8 fixed point
#define backdrop_colour   RGB8( 256, 256, 256 )
#define pal2bgram(p)	  (BG_PALETTE + (p) * 16)

//sprites
#define tiles_character	  0 // character tiles (16x16 tile 0->3)

#define tile2objram(t) (SPRITE_GFX + (t) * 16)

//palette of sprites
#define pal2objram(p) (SPRITE_PALETTE + (p) * 16)
#define pal_character	  0 // character palette (entry 0->15)

//display de sprites
typedef struct t_spriteEntry
{
	u16 attr0;
	u16 attr1;
	u16 attr2;
	u16 affine_data;
} spriteEntry;

#define sprites ((spriteEntry*)OAM)

//object character
#include "character.h"

character g_character;
int vertical = 64; // Indica el desplazamiento vertical del fondo

int map[1024]; // Se crea un array de tiles para rellenarlos de valores
		// aleatorios en la fase de procesar la logica

#define x_tweak	(2<<8)  // for user input
#define y_tweak	800	  // for user input

//-----------------------------------------------------------------
// Dibujamos el fondo usando el mapa
//-----------------------------------------------------------------
void draw(){
//-----------------------------------------------------------------
	int i;

	for ( i = 0; i < 1024; i++)
		bg0map[ i ] = (bg0+map[i]) | pal_bg0 << 12;

}

//-----------------------------------------------------------------
// Recrearemos el mapa para dar la sensacion de scroll
//-----------------------------------------------------------------
void redraw(){
//-----------------------------------------------------------------
	
	int n;
	// Avanzamos las posiciones menores a 768 a las mayores
	for (n=767; n>=0; n--)
		map[n+256] = map[n];
	
	// Limpiamos los indices inferiores del mapa
	for(n=0; n<256; n++)
		map[n] = 0;

	// Hacemos el dibujo aleatorio en las partes del mapa
	// que seran visibles cuando se scrollee
	for(n=0; n< 256; n++){
		if (!map[n]){
			int temp = rand()%256;
			if ( !map[n+1] && !map[n+32] && !map[n+33] && !temp ){
				map[n] = 4;
				map[n+1] = 5;
				map[n+32] = 6;
				map[n+33] = 7;
			}
			if (temp != 0 && temp < 16){
				if(rand()%2)
					map[n] = 2;
				else
					map[n] = 3;
			}
		}

	}

}


//-----------------------------------------------------------------
// process input
//-----------------------------------------------------------------
void processInput( void )
//-----------------------------------------------------------------
{
	scanKeys();

	int keysh = keysHeld();
	// El input ha sido simplificado a su nivel de pelota
	if( keysh & KEY_UP )	  // check if UP is pressed
	{
		g_character.yvel -= y_tweak;
		
	}
	if( keysh & KEY_DOWN )	// check if DOWN is pressed
	{
		// tweak y velocity of character
		g_character.yvel += y_tweak;
	}
	if( keysh & KEY_LEFT  )	// check if LEFT is pressed
	{

		g_character.xvel -= x_tweak;
		
	}
	if( keysh & KEY_RIGHT  )   // check if RIGHT is pressed
	{
		
		g_character.xvel += x_tweak;

	}

}

void processLogic( void )
{ 
	processInput();

	// Decrementamos SIEMPRE el scroll vertical
	vertical--;
	// Si hemos alcanzado el 0...
	if( !vertical ){
		// Redibujamos el mapa y reseteamos vertical
		redraw();
		vertical = 64;
	}

	characterUpdate( &g_character );

	iprintf("\x1b[2J");
	iprintf("x: %d y: %d\n", g_character.x, g_character.y);
	iprintf("Vertical: %d\n", vertical);
}

//-----------------------------------------------------------
// update graphical information (call during vblank)
//-----------------------------------------------------------
void updateGraphics( void )
//-----------------------------------------------------------
{ 
	// update character sprite, camera = 0, 0
	characterRender( &g_character, 0, 0 );
	
	// Solo tenemos que dibujar el mapa actual, sea
	// cual sea su contenido
	draw();

	// Actualizamos el registro de scroll
	REG_BG0VOFS = vertical;
}

//-----------------------------------------------------------
// reset character attributes
//-----------------------------------------------------------
void resetcharacter( void )
//-----------------------------------------------------------
{
	// use sprite index 0 (0->127)
	g_character.sprite_index = 0;
	
	// use affine matrix 0 (0->31)
	g_character.sprite_affine_index = 0;
	
	// X = 128.0
	g_character.x = 0 << 8;
	
	// Y = 64.0
	g_character.y = 64 << 8;
   
	// start X velocity a bit to the right
	g_character.xvel = 100 << 4;
   
	// reset Y velocity
	g_character.yvel = 0;

	
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
	// Para este ejemplo NO usaremos un fondo expandido
	REG_BG0CNT = 1 << 8; /** IMPORTANTE */

	// Inicializamos el mapa
	for (n=0; n<1024; n++)
		map[n] = 0;

	// Rellenaremos el mapa con estrellas
	for (n=0; n<1024; n++){
		if (!map[n]){
			int temp = rand()%256;
			if ( !map[n+1] && !temp ){
				map[n] = 4;
				map[n+1] = 5;
				map[n+32] = 6;
				map[n+33] = 7;
			}
			if (temp != 0 && temp < 16){
				if(rand()%2)
					map[n] = 2;
				else
					map[n] = 3;
			}
		}
	}

	// Dibujamos el mapa
	draw();
	
	//Cargar sprites
	dmaCopyHalfWords( 3, gfx_nave1Tiles, tile2objram(tiles_character), gfx_nave1TilesLen );

	// Copiar paleta a la memoria
	dmaCopyHalfWords( 3, gfx_nave1Pal, pal2objram(pal_character), gfx_nave1PalLen );

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
	resetcharacter();

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
