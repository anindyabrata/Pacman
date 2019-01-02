#include "iGraphics.h"
#include <stdlib.h>
#include <time.h>

int food_left ;
int score ;
int lives ;
int show ;
int paused ;
int move_timer ;
int max_number_of_high_scores = 10 ;
int number_of_high_scores ;
int scores[ 100 ] ;
char players[ 100 ][ 80 ] ;
char player_name[ 80 ] ;

int is_blank[ 14 + 5 ][ 10 + 5 ] ;
int is_food[ 14 + 5 ][ 10 + 5 ] ;

char * pac_image[] = { "pacman_up.bmp" , "pacman_right.bmp" , "pacman_down.bmp" , "pacman_left.bmp" } ;
char * enemy1_image = "enemy1_left.bmp" ;
char * enemy2_image = "enemy2_left.bmp" ;

// up , right , down , left
int dx[] = { 0 , 1 , 0 , -1 } ;
int dy[] = { 1 , 0 , -1 , 0 } ;

struct pos {
	int x , y , dir ;
} player , enemy1 , enemy2 , forward , right , left , backward ;

int at_same_cell( pos p , pos q )
{
	return p.x == q.x && p.y == q.y ;
}

void load_high_scores()
{
	FILE * fp = fopen( "highscores.txt" , "r" ) ;
	fscanf( fp , "%d" , &number_of_high_scores ) ;
	for ( int i = 0 ; i < number_of_high_scores ; i++ ) {
		fscanf( fp , "%s %d" , players[ i ] , &scores[ i ] ) ;
	}
	fclose( fp ) ;
}

void save_high_scores()
{
	FILE * fp = fopen( "highscores.txt" , "w" ) ;
	fprintf( fp , "%d\n" , number_of_high_scores ) ;
	for ( int i = 0 ; i < number_of_high_scores ; i++ ) {
		fprintf( fp , "%s %d\n" , players[ i ] , scores[ i ] ) ;
	}
	fclose( fp ) ;
}

void sort_scores()
{
	for ( int i = 0 ; i < number_of_high_scores ; i++ ) {
		for ( int j = i + 1 ; j < number_of_high_scores ; j++ ) {
			if ( scores[ i ] < scores[ j ] ) {
				int t = scores[ i ] ;
				scores[ i ] = scores[ j ] ;
				scores[ j ] = t ;

				char s[ 80 ] ;
				strcpy( s , players[ i ] ) ;
				strcpy( players[ i ] , players[ j ] ) ;
				strcpy( players[ j ] , s ) ;
			}
		}
	}
}

bool made_high_score()
{
	if ( number_of_high_scores < max_number_of_high_scores ) return true ;
	int lowest_score_index = number_of_high_scores - 1 ;
	return scores[ lowest_score_index ] < score ;
}

void insert_new_score()
{
	int idx = number_of_high_scores ;
	scores[ idx ] = score ;
	strcpy( players[ idx ] , player_name ) ;
	number_of_high_scores++ ;
	sort_scores() ;
	if ( number_of_high_scores > max_number_of_high_scores ) number_of_high_scores-- ;
	save_high_scores() ;
}

void show_high_scores()
{
	for ( int i = 0 , height = 400 ; i < number_of_high_scores ; i++ , height -= ( 400 - 100 ) / ( number_of_high_scores == 1 ? 1 : ( number_of_high_scores - 1 ) ) ) {
		char b[ 80 ] ;
		itoa( i + 1 , b , 10 ) ;
		iText( 300 , height , b ) ;
		iText( 450 , height , players[ i ] ) ;
		itoa( scores[ i ] , b , 10 ) ;
		iText( 600 , height , b ) ;
	}
}

void game_init( int start )
{
	int i ;
	if ( start ) {
		for ( i = 2 ; i <= 12 ; i++ ) is_blank[ i ][ 2 ] = is_blank[ i ][ 8 ] = 1 ;
		for ( i = 2 ; i <= 8 ; i++ ) is_blank[ 2 ][ i ] = is_blank[ 4 ][ i ] = is_blank[ 7 ][ i ] = is_blank[ 12 ][ i ] = 1 ;
		for ( i = 5 ; i <= 7 ; i++ ) is_blank[ i ][ 5 ] = 1 ;
		lives = 3 ;
		score = 0 ;
		food_left = 0 ;
		for ( i = 1 ; i <= 14 ; i++ )
			for ( int j = 1 ; j <= 10 ; j++ )
				if ( is_blank[ i ][ j ] && ( i != 4 || j != 5 ) )
					food_left++ , is_food[ i ][ j ] = ( rand() % 4 ) ? 1 : 50 ;
	}
	player.x = 4 , player.y = 5 , player.dir = 1 ;
	enemy1.x = 2 + rand() % 11 , enemy1.y = 8 , enemy1.dir = rand() % 4 ;
	enemy2.x = 2 + rand() % 11 , enemy2.y = 2 , enemy2.dir = rand() % 4 ;
	iPauseTimer( move_timer ) ;
	paused = 1 ;
}

void calculate_cells( pos p )
{
	forward.x = p.x + dx[ p.dir ] ;
	forward.y = p.y + dy[ p.dir ] ;
	forward.dir = p.dir ;


	backward.x = p.x - dx[ p.dir ] ;
	backward.y = p.y - dy[ p.dir ] ;
	backward.dir = ( p.dir + 2 ) % 4 ;

	right.dir = ( p.dir + 1 ) % 4 ;
	right.x = p.x + dx[ right.dir ] ;
	right.y = p.y + dy[ right.dir ] ;

	left.dir = ( p.dir + 3 ) % 4 ;
	left.x = p.x + dx[ left.dir ] ;
	left.y = p.y + dy[ left.dir ] ;
}

int can_go_to( pos p )
{
	return is_blank[ p.x ][ p.y ] ;
}

pos change_direction( pos p , int dir )
{
	pos changed ;
	changed.x = p.x + dx[ dir ] ;
	changed.y = p.y + dy[ dir ] ;
	if ( can_go_to( changed ) ) {
		p.dir = dir ;
	}
	return p ;
}

void draw_player()
{
	iShowBMP( 150 + (player.x - 1) * 50 + 5 , 50 + (player.y - 1) * 50 + 5 , pac_image[ player.dir ] ) ;
}

void draw_enemy1()
{
	if ( enemy1.dir == 1 ) enemy1_image = "enemy1_right.bmp" ;
	else if ( enemy1.dir == 3 ) enemy1_image = "enemy1_left.bmp" ;
	iShowBMP( 150 + (enemy1.x - 1) * 50 + 5 , 50 + (enemy1.y - 1) * 50 + 5 , enemy1_image ) ;
}

void draw_enemy2()
{
	if ( enemy2.dir == 1 ) enemy2_image = "enemy2_right.bmp" ;
	else if ( enemy2.dir == 3 ) enemy2_image = "enemy2_left.bmp" ;
	iShowBMP( 150 + (enemy2.x - 1) * 50 + 5 , 50 + (enemy2.y - 1) * 50 + 5 , enemy2_image ) ;
}

void draw_food( int x , int y )
{
	if ( is_food[ x ][ y ] == 1 ) iFilledCircle( 150 + (x - 1) * 50 + 25 , 50 + (y - 1) * 50 + 25 , 4 , 4 ) ;//iShowBMP() ;
	else iShowBMP( 150 + (x - 1) * 50 + 9 , 50 + (y - 1) * 50 + 9 , "grape_smaller.bmp" ) ;
}

void draw_stats()
{
	char liv[ 80 ] , sc[ 80 ] ;
	itoa( lives , liv , 10 ) ;
	itoa( score , sc , 10 ) ;
	iText( 65 , 600 - 200 , liv , GLUT_BITMAP_TIMES_ROMAN_24 ) ;
	iText( 900 , 600 - 180 , sc , GLUT_BITMAP_TIMES_ROMAN_24 ) ;
}

void draw_game()
{
	iShowBMP( 0 , 0 , "game.bmp" ) ;
	for ( int i = 1 ; i <= 14 ; i++ ) {
		for ( int j = 1 ; j <= 10 ; j++ ) {
			if ( !is_blank[ i ][ j ] ) {
				//iFilledRectangle( 150 + (i - 1) * 50 + 2 , 50 + (j - 1) * 50 + 2 , 50 - 4 , 50 - 4 ) ;
				iShowBMP( 150 + (i - 1) * 50 + 2 , 50 + (j - 1) * 50 + 2 , "wall.bmp" ) ;
			}
			if ( is_food[ i ][ j ] ) {
				draw_food( i , j ) ;
			}
		}
	}
	draw_player() ;
	draw_enemy1() ;
	draw_enemy2() ;
	draw_stats() ;
}

void draw_menu()
{

	iShowBMP( 0 , 0 , "pacmenu_new.bmp" ) ;
}

void iDraw()
{	
	iClear();
	//iFilledRectangle( 150 , 50 , 700 , 500 ) ; // grid land
	//iFilledRectangle( 150 , 50 , 50 , 50 ) ; // unit cell
	if ( show == 0 ) {
		draw_menu() ;
	}
	else if ( show == 1 ) {
		draw_game() ;
		if ( paused ) iText( 400 , 25 , "Press p to start game" ) ;
	}
	else if ( show == 2 ) {
		iText( 450 , 300 , "Game Over" ) ;
		iText( 457 , 280 , "You Win" ) ;
	}
	else if ( show == 3 ) {
		iText( 450 , 300 , "Game Over" ) ;
		iText( 454 , 280 , "You Lose" ) ;
	}
	else if ( show == 4 ) {
		// take name input
		iShowBMP( 0 , 0 , "input.bmp" ) ;
		iSetColor( 250 , 250 , 255 ) ;
		iText( 360 , 600 - 295 , player_name ) ;
	}
	else if ( show == 5 ) {
		// show high scores
		iShowBMP( 0 , 0 , "high scores.bmp" ) ;
		show_high_scores() ;
	}
	else if ( show == 6 ) {
		iShowBMP( 0 , 0 , "pac_credits.bmp" ) ;
	}
}

void iMouseMove(int mx, int my)
{
	
}

void iMouse(int button, int state, int mx, int my)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if ( show == 0 && mx >= 303 && mx <= 707 && my >= 600 - 152 && my <= 600 - 67 ) game_init( 1 ) , show = 1 ;
		else if ( show == 0 && mx >= 8 && mx <= 140 && my >= 600 - 115 && my <= 600 - 10 ) show = 5 ;
		else if ( show == 0 && mx >= 910 && mx <= 980 && my >= 600 - 95 && my <= 600 - 15 ) show = 6 ;
		else if ( show == 0 && mx >= 410 && mx <= 575 && my >= 600 - 553 && my <= 600 - 464 ) exit( 0 ) ;
	}
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
			
	}
}

void iKeyboard(unsigned char key)
{
	if ( show == 4 ) {
		if ( key == '\r' ) {
			insert_new_score() ;
			for ( int i = 0 ; i < 80 ; i++ ) player_name[ i ] = '\0' ;
			show = 5 ;
		}
		else if ( key == '\b' && player_name[ 0 ] != '\0' ) {
			player_name[ strlen( player_name ) - 1 ] = '\0' ;
		}
		else {
			player_name[ strlen( player_name ) ] = key ;
		}
	}
	else if ( show == 5 && key == '\r' ) {
		if ( lives == 0 ) show = 3 ;
		else show = 2 ;
	}
	else if ( ( show == 2 || show == 3 || show == 6 ) && key == '\r' ) {
		game_init( 0 ) ;
		show = 0 ;
	}
	else if ( show == 1 && key == 's' ) {
		iPauseTimer( move_timer ) ;
		paused = 1 ;
	}
	else if ( show == 1 && key == 'p' ) {
		iResumeTimer( move_timer ) ;
		paused = 0 ;
	}
}

void iSpecialKeyboard(unsigned char key)
{	
	if(key == GLUT_KEY_END)
	{
		exit(0);	
	}
	if ( show == 1 ) {
		if ( key == GLUT_KEY_UP ) {
			player = change_direction( player , 0 ) ;
		}
		else if ( key == GLUT_KEY_RIGHT ) {
			player = change_direction( player , 1 ) ;
		}
		else if ( key == GLUT_KEY_DOWN ) {
			player = change_direction( player , 2 ) ;
		}
		else if ( key == GLUT_KEY_LEFT ) {
			player = change_direction( player , 3 ) ;
		}
	}
}

void game_end()
{
	if ( made_high_score() ) {
		show = 4 ;
	}
	else show = 5 ;
}

void check_food( pos p )
{
	if ( is_food[ p.x ][ p.y ] ) {
		score += is_food[ p.x ][ p.y ] * 10 ;
		is_food[ p.x ][ p.y ] = 0 ;
		food_left-- ;
	}
	if ( !food_left ) game_end() ;
}

pos make_decision( pos p )
{
	calculate_cells( p ) ;
	if ( !can_go_to( forward ) && !can_go_to( left ) && !can_go_to( right ) ) return backward ;
	while ( true ) {
		int dec = rand() % 3 ;
		if ( dec == 0 && can_go_to( forward ) ) return forward ;
		else if ( dec == 1 && can_go_to( left ) ) return left ;
		else if ( dec == 2 && can_go_to( right ) ) return right ;
	}
}

bool check_enemy( pos p )
{
	return at_same_cell( p , enemy1 ) || at_same_cell( p , enemy2 ) ;
}

void make_a_move()
{
	pos prev = player ;
	calculate_cells( player ) ;
	if ( can_go_to( forward ) ) player = forward ;

	enemy1 = make_decision( enemy1 ) ;

	enemy2 = make_decision( enemy2 ) ;

	if ( check_enemy( prev ) || check_enemy( player ) ) {
		lives-- ;
		if ( lives )
			game_init( 0 ) ;
		else game_end() ;
	}

	check_food( player ) ;
}

int main()
{
	srand( (unsigned)( time( NULL ) % 500 ) ) ;
	lives = 3 ;
	show = 0 ;
	move_timer = iSetTimer( 300 , make_a_move ) ;
	game_init( 1 ) ;
	load_high_scores() ;
	iInitialize(1000, 600, "Pacman");
	return 0;
}