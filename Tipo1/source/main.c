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

//object character
#include "character.h"

character g_character;
int scrolling = 0; // Indica si se esta scrolleando y cual es el tipo
int horizontal = 0; // Indica el desplazamiento en el eje de las x del scroll
int actualStage; // Indica en que escenario se encuentra el personaje

#define x_tweak	(2<<8)  // for user input
#define y_tweak	800	  // for user input

//-----------------------------------------------------------------
// Los siguiente tres metodos dibujan los fondos
//-----------------------------------------------------------------
void draw0inY(int sector){
//-----------------------------------------------------------------
	int i, j;
	for( i = sector*1024; i < (sector+1)*1024; i++ )
        	bg0map[i] = 0; /** IMPORTANTE */
	
	// Suelo
	// Bloque 1: 	8  9
	//		10 11
	// Bloque 2:	12 13
	//		14 15
	for ( i = sector*32; i < (sector+1)*32; i++){
		if ( i%4 < 2 ){
			bg0map[ i + (sector*31+25)*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+26)*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+27)*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+28)*32 ] = (bg0+14+i%2) | pal_bg0 << 12;
		}
		else{
			bg0map[ i + (sector*31+25)*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+26)*32 ] = (bg0+14+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+27)*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+28)*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
		}	
	}

	// Plataforma
	// 4 5
	// 6 7
	for ( i = sector*32; i < (sector+1)*32; i++){
		bg0map[ i + (sector*31+23)*32 ] = (bg0+4+i%2) | pal_bg0 << 12;
		bg0map[ i + (sector*31+24)*32 ] = (bg0+6+i%2) | pal_bg0 << 12;
	}

	// Rocas
	// Roca 1:	16 17
	// 		18 19
	// Roca 2:	20 21
	// 		22 23
	for ( i = sector*32; i < sector*32+8; i++){
		for (j=sector*31+19; j<sector*31+23; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+16+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+20+i%2) | pal_bg0 << 12;
		for (j=sector*31+20; j<sector*31+23; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+18+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+22+i%2) | pal_bg0 << 12;

	}

	for ( i = sector*32; i < sector*32+4; i++){
		for (j=sector*31+15; j<sector*31+19; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+16+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+20+i%2) | pal_bg0 << 12;
		for (j=sector*31+16; j<sector*31+19; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+18+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+22+i%2) | pal_bg0 << 12;

	}

	// Escaleras
	// 24 25
	for ( i = sector*32; i < sector*32+2; i++){
		for (j=sector*32; j<sector*31+15; j++)
			bg0map[ i + j*32 ] = (bg0+24+i%2) | pal_bg0 << 12;
	}

}

void draw1inY(int sector){
	int i, j;
	for( i = sector*1024; i < (sector+1)*1024; i++ )
        	bg0map[i] = 0; /** IMPORTANTE */
	
	// Suelo
	// Bloque 1: 	8  9
	//		10 11
	// Bloque 2:	12 13
	//		14 15
	for ( i = sector*32; i < (sector+1)*32; i++){
		if ( i%4 < 2 ){
			bg0map[ i + (sector*31+25)*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+26)*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+27)*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+28)*32 ] = (bg0+14+i%2) | pal_bg0 << 12;
		}
		else{
			bg0map[ i + (sector*31+25)*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+26)*32 ] = (bg0+14+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+27)*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+28)*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
		}	
	}

	// Plataforma
	// 4 5
	// 6 7
	for ( i = sector*32; i < (sector+1)*32; i++){
		bg0map[ i + (sector*31+23)*32 ] = (bg0+4+i%2) | pal_bg0 << 12;
		bg0map[ i + (sector*31+24)*32 ] = (bg0+6+i%2) | pal_bg0 << 12;
	}

	// Rocas
	// Roca 1:	16 17
	// 		18 19
	// Roca 2:	20 21
	// 		22 23
	for ( i = sector*32+12; i < sector*32+20; i++){
		for (j=sector*31+19; j<sector*31+23; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+16+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+20+i%2) | pal_bg0 << 12;
		for (j=sector*31+20; j<sector*31+23; j+=2)
			if ( i%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+18+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+22+i%2) | pal_bg0 << 12;

	}
	for ( i = sector*32+14; i < sector*32+18; i++){
		for (j=sector*31+15; j<sector*31+19; j+=2)
			if ( (i+2)%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+16+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+20+i%2) | pal_bg0 << 12;
		for (j=sector*31+16; j<sector*31+19; j+=2)
			if ( (i+2)%4 < 2 )
				bg0map[ i + j*32 ] = (bg0+18+i%2) | pal_bg0 << 12;
			else
				bg0map[ i + j*32 ] = (bg0+22+i%2) | pal_bg0 << 12;

	}

}
void draw2inY(int sector){
	int i, j;
	for( i = sector*1024; i < (sector+1)*1024; i++ )
        	bg0map[i] = 0; /** IMPORTANTE */
	
	// Suelo
	// Bloque 1: 	8  9
	//		10 11
	// Bloque 2:	12 13
	//		14 15
	for ( i = sector*32; i < (sector+1)*32; i++){
		if ( i%4 < 2 ){
			bg0map[ i + (sector*31+25)*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+26)*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+27)*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+28)*32 ] = (bg0+14+i%2) | pal_bg0 << 12;
		}
		else{
			bg0map[ i + (sector*31+25)*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+26)*32 ] = (bg0+14+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+27)*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
			bg0map[ i + (sector*31+28)*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
		}	
	}

	// Plataforma
	// 4 5
	// 6 7
	for ( i = sector*32; i < (sector+1)*32; i++){
		bg0map[ i + (sector*31+23)*32 ] = (bg0+4+i%2) | pal_bg0 << 12;
		bg0map[ i + (sector*31+24)*32 ] = (bg0+6+i%2) | pal_bg0 << 12;
	}

	// Pared con tiles de suelo
	for ( i = sector*32+28; i < (sector+1)*32; i++){
		for ( j = 0; j<32; j+=4){
			if ( i%4 < 2 ){
				bg0map[ i + (sector*31+j)*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
				bg0map[ i + (sector*31+j+1)*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
				bg0map[ i + (sector*31+j+2)*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
				bg0map[ i + (sector*31+j+3)*32 ] = (bg0+14+i%2) | pal_bg0 << 12;
			}
			else{
				bg0map[ i + (sector*31+j)*32 ] = (bg0+12+i%2) | pal_bg0 << 12;
				bg0map[ i + (sector*31+j+1)*32 ] = (bg0+14+i%2) | pal_bg0 << 12;
				bg0map[ i + (sector*31+j+2)*32 ] = (bg0+8+i%2) | pal_bg0 << 12;
				bg0map[ i + (sector*31+j+3)*32 ] = (bg0+10+i%2) | pal_bg0 << 12;
			}
		}	
	}


	// Escaleras
	// 24 25
	for ( i = sector*32+26; i < sector*32+28; i++){
		for (j=sector*31+23; j<sector*31+29; j++)
			bg0map[ i + j*32 ] = (bg0+24+i%2) | pal_bg0 << 12;
	}


}

//-----------------------------------------------------------------
// Este metodo sirve como switch para simplificar las llamadas
//-----------------------------------------------------------------
void drawXinY(int map, int sector){
//-----------------------------------------------------------------
	switch(map){
		case 0: draw0inY(sector); break;
		case 1: draw1inY(sector); break;
		case 2: draw2inY(sector); break;
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
	// process user input
	if( keysh & KEY_UP && !scrolling)	  // check if UP is pressed
	{
		// Este codigo recrea el salto
		// Si el personaje esta en al nivel de la plataforma, puede saltar
		if ( g_character.y == c_platform_level )
			g_character.yvel = -y_tweak;
		// Si el personaje sigue subiendo y no ha alcanzado la altura maxima
		// su velocidad de subida puede aumentar
		else if ( (g_character.yvel < 0) && (g_character.y < 4 + c_platform_level) )
			g_character.yvel -= y_tweak/14;
		
	}
	if( keysh & KEY_DOWN && !scrolling )	// check if DOWN is pressed
	{
		// tweak y velocity of character
		g_character.yvel += y_tweak;
	}
	if( keysh & KEY_LEFT && !scrolling )	// check if LEFT is pressed
	{
		// tweak x velocity
		g_character.xvel -= x_tweak;
	}
	if( keysh & KEY_RIGHT && !scrolling )   // check if RIGHT is pressed
	{
		// tweak y velocity
		g_character.xvel += x_tweak;
	}

}

void processLogic( void )
{ 
	processInput();
	
	// Si el personaje esta tocando el borde derecho de 
	// la pantalla y no estamos en el ultimo escenario,
	// realizaremos un scroll a derechas.
	if( g_character.x >= 61000 && actualStage<2 ) 
		scrolling = 2;

	// Si el personaje esta tocando el borde izquierdo de 
	// la pantalla y no estamos en el primer escenario,
	// realizaremos un scroll a izquierdas.
	if( g_character.x <= 0 && actualStage>0 ){
		if (!scrolling )
			horizontal = 256;
		scrolling = 1;	
	}

	// Si no estamos scrolleando, debemos actualizar el
	// personaje de forma normal
	if (scrolling == 0)
		characterUpdate( &g_character );

	// Si estamos scrolleando a izquierdas debemos...
	if (scrolling == 1){
		// Indicar que la pelota se debe desplazar
		// a la derecha
		scrollingUpdate( &g_character, 1 );
		// El fondo se debe desplazar hacia la derecha
		horizontal -= 4;
		// Si hemos terminado de scrollear...
		if ( horizontal <= 0 ){
			// lo indicamos
			scrolling = 0; 
			// y marcamos que estamos en un fondo
			// a la izquierda
			actualStage -= 1;
		}
	}

	// Si estamos scrolleando a derechas...
	if (scrolling == 2){
		// El fondo y el personaje se desplazan
		// a la izquierda
		scrollingUpdate( &g_character, -1 );
		horizontal += 4;
		// Si hemos terminado...
		if ( horizontal >= 256 ){
			// se indica, avanzamos al siguiente escenario
			// y reseteamos el horizontal a 0
			scrolling = 0; 
			actualStage += 1;
			horizontal = 0;
		}
	}

	iprintf("\x1b[2J");
	iprintf("x: %d y: %d", g_character.x, g_character.y);
}

//-----------------------------------------------------------
// update graphical information (call during vblank)
//-----------------------------------------------------------
void updateGraphics( void )
//-----------------------------------------------------------
{ 
	// update character sprite, camera = 0, 0
	characterRender( &g_character, 0, 0 );
	
	// Si...
	switch (scrolling){
		// No scrolleamos, pintamos el fondo actual
		case 0: drawXinY(actualStage, 0); 
			break;
		// Vamos a la izquierda, el actual en la segunda
		// parte y el nuevo en la primera parte del fondo
		case 1: drawXinY(actualStage, 1);
			drawXinY(actualStage-1, 0);
			break;
		// Si vamos a la derecha, el nuevo en la segunda
		// parte del fondo
		case 2: drawXinY(actualStage+1, 1);
			break;
	}
	
	// Actualizamos el registro de scroll
	REG_BG0HOFS = horizontal;
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
	g_character.x = 128 << 8;
	
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

//-----------------------------------------------------------
// Habilitamos las variables de los escenarios
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
	// Quizas esta sea la parte mas importante del proyecto:
	// activar el bit 14 a 1 indica al programa que debe considerar
	// un doble fondo horizontal
	REG_BG0CNT = 1 << 14 | 1 << 8;

	REG_BG0VOFS = 40;

    	// cargamos la pantalla 1
	draw0inY(0);
	
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
