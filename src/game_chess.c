/*--------------------------------------------------------------------------------------*\
                                      Copyright 2002, 2003, 2004, 2005 by T2Team         |
  Bard Code                           The Bard Code has been developed by T2Team:        |
  Driad Release     ..;;;;;.      (_) - Amlugil, Celyr, Miciko, Onirik, Raul, Sharas     |
  BARD v0.6u      .~`" >------___/  / Older Members of T2Team:                           |
               ,'"  J"!||||||||:/  /  - Jamirithiana, Qaba, Tanegashima                  |
    -~,      ,'  ,;"!|||||||||!/  /   ---------------------------------------------------|
 ;;/   `^~`~_._/!|||||||||||!//  /    The Bard Code is derived from:                     |
 "".  =====!/!|||||||||||||!//  /     - DikuMUD coded by: Hans Staerfeldt, Katja Nyboe,  |
 .'  / |||||||||||||||||||//   /        Tom Madsen, Michael Seifert and Sebastian Hammer |
.'  /  ||||||||||||||||!//    /       - MERC 2.1 coded by Hatchet, Furey, and Kahn       |
;  !   ||||||||||||||!/X/    /        - SMAUG 1.4a coded by Thoric (Derek Snider) with:  |
:  !   ||||||||||||!/X/     /           Altrag, Blodkai, Haus, Narn, Scryn, Swordbearer, |
:  !   |||||||||||/X/      /            Tricops, Gorog, Rennard, Grishnakh, Fireblade    |
:  !   |||||||||!/X/      /             and Nivek                                        |
:   \  |||||||!/X/       /                                                               |
 "   \ |||||!/x/        /                                                                |
  "   \|||!/X/         /                                                                 |
   \   \|/X/          /                                                                  |
    "   !/           /                ... and the blood!                                 |
     ;'/            /                                                                    |
      "------------(                  --------------------------------------------------*/


#ifdef T2_CHESS

#include "mud.h"
#include "interpret.h"


/*
 * Definizioni
 */
#define WHITE_BACK	"^w"
#define BLACK_BACK	"^x"
#define WHITE_FORE	"&W"
#define BLACK_FORE	"&z"

#define COLOR_WHITE	 0
#define COLOR_BLACK	 1
#define COLOR_NONE	 2

#define SAME_COLOR( x1, y1, x2, y2 )							\
	( board->piece[(x1)][(y1)] != PIECE_NONE					\
	&& (board->color[(x1)][(y1)] == board->color[(x2)][(y2)]) )


/*
 * Enumerazione dei tipi di pezzi
 */
typedef enum
{
	PIECE_PAWN,
	PIECE_ROOK,
	PIECE_KNIGHT,
	PIECE_BISHOP,
	PIECE_QUEEN,
	PIECE_KING,
	PIECE_NONE,
	MAX_PIECE
} pieces_types;


/*
 * Enumerazione dei tipi di mosse
 */
typedef enum
{
	MOVE_OK,
	MOVE_INVALID,
	MOVE_BLOCKED,
	MOVE_TAKEN,
	MOVE_OFFBOARD,
	MOVE_SAMECOLOR,
	MOVE_CHECK,
	MOVE_WRONGCOLOR,
	MOVE_INCHECK,
	MOVE_CASTLE,
	MOVE_PROMOTE,
	MAX_MOVE
} moves_types;


/*
 * Enumerazione dei tipi di flag per il gioco
 */
typedef enum
{
	CHESSFLAG_MOVEDKING,	/* Serve per evitare l'arrocco se è stato già mosso il re */
	CHESSFLAG_MOVEDKROOK,	/* Serve per evitare l'arrocco corto se è stata già mossa la torre del re */
	CHESSFLAG_MOVEDQROOK,	/* Serve per evitare l'arrocco lungo se è stata già mossa la torre della regina */
	MAX_CHESSFLAG
} chess_flags;


/*
 * Struttura per la tabella dei pezzi da gioco
 */
struct piece_type
{
	char	*name;
	char	*wgraph1;
	char	*wgraph2;
	char	*bgraph1;
	char	*bgraph2;
};


/*
 * Struttura di una scacchiera da gioco
 */
typedef struct chess_board_data	CHESSBOARD_DATA;

struct chess_board_data
{
	CHESSBOARD_DATA	*next;
	CHESSBOARD_DATA	*prev;
	CHAR_DATA		*player1;
	CHAR_DATA		*player2;
	CHAR_DATA		*turn;
	BITVECTOR		*flags1;			/* Flag di gioco del giocatore 1 */
	BITVECTOR		*flags2;			/* Flag di gioco del giocatore 2 */
	int				 piece[8][8];		/* disposizione dei pezzi sulla scacchiera */
	int				 color[8][8];		/* colore dei pezzi sulla scacchiera */
	int				 moves;
	int				 lastx;
	int				 lasty;
};


/*
 * Variabili
 */
static CHESSBOARD_DATA	*first_chessboard	= NULL;
static CHESSBOARD_DATA	*last_chessboard	= NULL;
	   int				 top_chessboard		= 0;


/*
 * Tabella con le informazioni sui pezzi
 */
const struct piece_type table_pieces[MAX_PIECE] =
{
	{ "pedone",		"  (-)  ",		"  -|-  ",	"  [-]  ",		"  -|-  "	},
	{ "torre",		"  ###  ",		"  { }  ",	"  ###  ",		"  [ ]  "	},
	{ "cavallo",	"  /-@- ",		" / /   ",	"  /-*- ",		" / /   "	},
	{ "alfiere",	"  () + ",		"  {}-| ",	"  [] + ",		"  {}-| "	},
	{ "regina",		"   @   ",		"  /+\\  ",	"   #   ",		"  /+\\  "	},
	{ "re",			"  ^^^^^^  ",	"  {@}  ",	"  ^^^^^^  ",	"  [#]  "	},
	{ "nessuno",	"       ",		"       ",	"       ",		"       "	}
};


/*
 * Passato un pg, ritorna la schacchiera a cui sta giocando, se ci sta giocando
 */
CHESSBOARD_DATA *get_chessboard( CHAR_DATA *ch )
{
	CHESSBOARD_DATA *board;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_chessboard: ch passato è NULL" );
		return NULL;
	}

	for ( board = first_chessboard;  board;  board = board->next )
	{
		if ( board->player1 == ch || board->player2 == ch )
			return board;
	}

	return NULL;
}


void init_board( CHESSBOARD_DATA *board )
{
	int	x;
	int	y;

	if ( !board )
	{
		send_log( NULL, LOG_BUG, "init_board: board passata è NULL" );
		return;
	}

	board->player1	= NULL;
	board->player2	= NULL;
	board->turn		= NULL;
	board->moves	= 0;

	for ( x = 0;  x < 8;  x++ )
	{
		for ( y = 0;  y < 8;  y++ )
		{
			board->piece[x][y] = PIECE_NONE;
			board->color[x][y] = COLOR_NONE;
		}
	}

	/* Prepara il lato del bianco */
	for ( x = 0;  x < 2;  x++ )
	{
		for ( y = 0;  y < 8;  y++ )
			board->color[x][y] = COLOR_WHITE;
	}
	
	board->piece[0][0] = PIECE_ROOK;
	board->piece[0][1] = PIECE_KNIGHT;
	board->piece[0][2] = PIECE_BISHOP;
	board->piece[0][3] = PIECE_QUEEN;
	board->piece[0][4] = PIECE_KING;
	board->piece[0][5] = PIECE_BISHOP;
	board->piece[0][6] = PIECE_KNIGHT;
	board->piece[0][7] = PIECE_ROOK;

	for ( x = 0; x < 8; x++ )
		board->piece[1][x] = PIECE_PAWN;

	/* Prepara il lato del nero */
	for ( x = 6;  x < 8;  x++ )
	{
		for ( y = 0;  y < 8;  y++ )
			board->color[x][y] = COLOR_BLACK;
	}

	for ( x = 0; x < 8; x++ )
		board->piece[6][x] = PIECE_PAWN;

	board->piece[7][0] = PIECE_ROOK;
	board->piece[7][1] = PIECE_KNIGHT;
	board->piece[7][2] = PIECE_BISHOP;
	board->piece[7][3] = PIECE_QUEEN;
	board->piece[7][4] = PIECE_KING;
	board->piece[7][5] = PIECE_BISHOP;
	board->piece[7][6] = PIECE_KNIGHT;
	board->piece[7][7] = PIECE_ROOK;
}


/*
 * Libera la struttura di una scacchiera
 */
void free_chessboard( CHESSBOARD_DATA *board )
{
	char	buf[MSL];

	if ( !board )
	{
		send_log( NULL, LOG_BUG, "free_board: board passata è NULL" );
		return;
	}

	sprintf( buf, "Il gioco è terminato dopo %d mosse.\r\n", board->moves );

	if ( board->player1 )
		send_to_char( board->player1, buf );

	if ( board->player2 )
		send_to_char( board->player2, buf );

	UNLINK( board, first_chessboard, last_chessboard, next, prev );
	top_chessboard--;

	board->player1 = NULL;
	board->player2 = NULL;
	board->turn	   = NULL;
	destroybv( board->flags1 );
	destroybv( board->flags2 );
	DISPOSE( board );
}

/*
 * Libera dalla memoria tutte le scacchiere
 */
void free_all_chessboards( void )
{
	CHESSBOARD_DATA *chess;

	for ( chess = first_chessboard;  chess;  chess = chess->next )
		free_chessboard( chess );

	if ( top_chessboard != 0 )
		send_log( NULL, LOG_BUG, "free_all_chessboards: top_chessboard non è 0 dopo aver liberato le scacchiere: %d", top_chessboard );
}


/*
 * Cerca nella scacchiera il pezzo passato impostando le coordinate passate
 * Ritorna TRUE se lo ha trovato, altrimenti FALSE
 */
bool find_piece( CHESSBOARD_DATA *board, int *x, int *y, int piece )
{
	int	a;
	int	b;

	if ( !board )
	{
		send_log( NULL, LOG_BUG, "find_piece: board passata è NULL" );
		return FALSE;
	}

	if ( piece < 0 || piece >= PIECE_NONE )
	{
		send_log( NULL, LOG_BUG, "find_piece: piece passato è errato: %d", piece );
		return FALSE;
	}

	for ( a = 0;  a < 8;  a++ )
	{
		for ( b = 0;  b < 8;  b++ )
		{
			if ( board->piece[a][b] == piece )
				break;
		}
	}

	*x = a;
	*y = b;

	if ( board->piece[a][b] == piece )
		return TRUE;

	return FALSE;
}


/*
 * Controlla se il re è sotto scacco
 * Ritorna il numero di pezzi che tengono sotto scacco il re
 */
int king_in_check( CHESSBOARD_DATA *board, const int piece, const int color )
{
	int		x = 0;
	int		y = 0;
	int		direction;
	int		count = 0;	/* conta quante volte contemporaneamente il re è sotto scacco */

	if ( !board )
	{
		send_log( NULL, LOG_BUG, "king_in_check: board passata è NULL" );
		return 0;
	}

	if ( piece < 0 || piece >= PIECE_NONE )
	{
		send_log( NULL, LOG_BUG, "king_in_check: pezzo passato errato: %d", piece );
		return 0;
	}

	if ( color < 0 || color >= COLOR_NONE )
	{
		send_log( NULL, LOG_BUG, "king_in_check: color passato errato: %d", color );
		return 0;
	}

	if ( piece != PIECE_KING )
		return 0;

	/* Cercando il pezzo nella scacchiera se lo trova imposta x e y con le coordinate del pezzo */
	if ( !find_piece(board, &x, &y, piece) )
		return 0;

	if ( x < 0 || y < 0 || x >= 8 || y >= 8 )
	{
		send_log( NULL, LOG_BUG, "king_in_check: coordinate del pezzo %s errate: x=%d y=%d",
			table_pieces[piece].name, x, y );
		return 0;
	}

	/* Pedoni - (??) ma qui se un cavallo si trova in posizione di scacco-pedone dà scacco anche lui?? manca il check che il tipo controllato sia un pedone secondo me */
	if ( board->color[x][y] == COLOR_WHITE && x < 7 && ((y > 0 && board->color[x+1][y-1] == COLOR_BLACK)
	  || (y < 7 && board->color[x+1][y+1] == COLOR_BLACK)) )
		count++;
	else if ( board->color[x][y] == COLOR_BLACK && x > 0 && ((y > 0 && board->color[x-1][y-1] == COLOR_WHITE)
	  || (y < 7 && board->color[x-1][y+1] == COLOR_WHITE)) )
		count++;

	/* Cavallo - il cavallo può eseguire una mossa in 8 direzioni max */
	for ( direction = 0;  direction < 8;  direction++ )
	{
		int	dirx = 0;
		int	diry = 0;

		/* Imposta le direzioni per ogni tipo di mossa-direzione */
		switch ( direction )
		{
		  case 0:	dirx = x - 2;	diry = y - 1;	break;
		  case 1:	dirx = x - 2;	diry = y + 1;	break;
		  case 2:	dirx = x - 1;	diry = y - 2;	break;
		  case 3:	dirx = x - 1;	diry = y + 2;	break;
		  case 4:	dirx = x + 1;	diry = y - 2;	break;
		  case 5:	dirx = x + 1;	diry = y + 2;	break;
		  case 6:	dirx = x + 2;	diry = y - 1;	break;
		  case 7:	dirx = x + 2;	diry = y + 1;	break;
		}

		/* Se supera i limiti della scacchiera ignora la mossa */
		if ( dirx <  0 )	continue;
		if ( dirx >= 8 )	continue;
		if ( diry <  0 )	continue;
		if ( diry >= 8 )	continue;

		/* Se nella direzione analizzata non ci sono pezzi ignora */
		if ( board->piece[dirx][diry] == PIECE_NONE )
			continue;

		/* Se il re e il cavallo hanno lo stesso colore ignora */
		if ( SAME_COLOR(x, y, dirx, diry) )
			continue;

		/* Se nella direzione analizza si trova un cavallo allora il re è sotto scacco */
		if ( board->piece[x+dirx][y+diry] == PIECE_KNIGHT )
			count++;
	}

	/* Alfiere, Torre, Regina - Controlla in una botta sola le 8 direzioni per i tre pezzi */
	for ( direction = 0;  direction < 8;  direction++ )
	{
		int	len;		/* lunghezza della ricerca nella direzione */
		int	dirx = 0;
		int	diry = 0;

		/* E' come se partisse da nord e poi continua in senso orario,
		 *	quindi le direzioni in diagonale sono quelle dispari */
		switch ( direction )
		{
		  case 0:	dirx =  0;	diry =  1;	break;	/* nord */
		  case 1:	dirx =  1;	diry =  1;	break;	/* nordest */
		  case 2:	dirx =  1;	diry =  0;	break;	/* est */
		  case 3:	dirx = -1;	diry =  1;	break;	/* sudest */
		  case 4:	dirx = -1;	diry =  0;	break;	/* sud */
		  case 5:	dirx = -1;	diry = -1;	break;	/* sudovest */
		  case 6:	dirx =  0;	diry = -1;	break;	/* ovest */
		  case 7:	dirx =  1;	diry = -1;	break;	/* nordovest */
		}

		/* Aumenta poco a poco il raggio di controllo nella direzione */
		for ( len = 1;  len < 8;  len++ )
		{
			dirx = x + (dirx * len);
			diry = y + (diry * len);

			/* Se supera i limiti della scacchiera ignora la ricerca in quella direzione */
			if ( dirx <  0 )	break;
			if ( dirx >= 8 )	break;
			if ( diry <  0 )	break;
			if ( diry >= 8 )	break;

			/* Se il re e il pezzo nella casella controllata hanno lo stesso colore si ferma */
			if ( SAME_COLOR(x, y, dirx, diry) )
				break;

			/* La regina si muove in tutte le direzioni, quindi il re è sotto scacco */
			if ( board->piece[dirx][diry] == PIECE_QUEEN )
				count++;
			/* L'alfiere si muove nelle diagonali */
			if ( board->piece[dirx][diry] == PIECE_BISHOP && (direction%2 == 1) )
				count++;
			/* Mentre la torre nelle direzioni orizzonatali e verticali */
			if ( board->piece[dirx][diry] == PIECE_BISHOP && (direction%2 == 0) )
				count++;

			/* Se c'è qualche altro pezzo nel controllo della direzione si ferma */
			if ( board->piece[dirx][diry] != PIECE_NONE )
				break;
		}
	}

	return count;
}


/*
 * Controlla se il re sia in possibile scacco matto
 * (TT) Purtroppo questa parte è parziale, visto che si potrebbe coprire lo scacco con
 *	uno spostamento di un'altro pezzo, bisogna vedere in gioco come si comporta
 */
static bool king_in_possible_checkmate( CHESSBOARD_DATA *board, const int piece, const int color )
{
	int	x = 0;
	int	y = 0;
	int direction;

	if ( !board )
	{
		send_log( NULL, LOG_BUG, "king_in_possible_checkmate: board passata è NULL" );
		return FALSE;
	}

	if ( piece < 0 || piece >= PIECE_NONE )
	{
		send_log( NULL, LOG_BUG, "king_in_possible_checkmate: pezzo passato errato: %d", piece );
		return FALSE;
	}

	if ( color < 0 || color >= COLOR_NONE )
	{
		send_log( NULL, LOG_BUG, "king_in_possibile_checkmate: color passato è errato: %d", color );
		return FALSE;
	}

	if ( piece != PIECE_KING )
		return FALSE;

	/* Cercando il re nella scacchiera ritorna le coordinate del pezzo */
	if ( !find_piece(board, &x, &y, piece) )
		return FALSE;

	if ( x < 0 || y < 0 || x >= 8 || y >= 8 )
	{
		send_log( NULL, LOG_BUG, "king_in_possible_checkmate: coordinate del pezzo %s errate: x=%d y=%d",
			table_pieces[piece].name, x, y );
		return FALSE;
	}

	if ( king_in_check(board, board->piece[x][y], board->color[x][y]) == 0 )
		return FALSE;

	for ( direction = 0;  direction < 8;  direction++ )
	{
		int		dirx = 0;
		int		diry = 0;
		int		sk	= -1;
		int		skc = -1;
		bool	incheck = FALSE;

		switch ( direction )
		{
		  case 0:	dirx = x + 1;		diry = y + 1;
		  case 1:	dirx = x - 1;		diry = y + 1;
		  case 2:	dirx = x + 1;		diry = y - 1;
		  case 3:	dirx = x - 1;		diry = y - 1;
		  case 4:	dirx = x;			diry = y + 1;
		  case 5:	dirx = x;			diry = y - 1;
		  case 6:	dirx = x + 1;		diry = y;
		  case 7:	dirx = x - 1;		diry = y;
		}

		if ( dirx <  0 )	continue;
		if ( dirx >= 8 )	continue;
		if ( diry <  0 )	continue;
		if ( diry >= 8 )	continue;

		if ( board->piece[dirx][diry] == PIECE_NONE )
		{
			sk	= board->piece[dirx][diry] = board->piece[x][y];
			skc = board->color[dirx][diry] = board->color[x][y];
			board->piece[x][y] = PIECE_NONE;
			board->color[x][y] = COLOR_NONE;
			if ( king_in_check(board, sk, skc) == 0 )
				incheck = TRUE;

			board->piece[x][y] = sk;
			board->color[x][y] = skc;
			board->piece[dirx][diry] = PIECE_NONE;
			board->color[dirx][diry] = COLOR_NONE;

			if ( incheck )
				return FALSE;
		}
	}

	return TRUE;
}


/*
 * Stampa su scermo la scacchiera in grande
 */
void show_board( CHAR_DATA *ch, CHESSBOARD_DATA *board )
{
	char	buf[MSL*2];
	int		x;
	int		y;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "show_board: ch passato è NULL" );
		return;
	}

	if ( !board )
	{
		send_log( NULL, LOG_BUG, "show_board: board passata è NULL" );
		return;
	}

	send_to_char( ch, "&W     1      2      3      4      5      6      7      8\r\n" );

	buf[0] = '\0';
	for ( x = 0;  x < 8;  x++ )
	{
		/* Prima linea */
		sprintf( buf+strlen(buf), "  " );
		for ( y = 0;  y < 8;  y++ )
		{
			if ( (x%2) == 0 )
				strcat( buf, (y%2 == 0) ? WHITE_BACK : BLACK_BACK );
			else
				strcat( buf, (y%2 == 0) ? BLACK_BACK : WHITE_BACK );
			strcat( buf, table_pieces[PIECE_NONE].wgraph1 );
		}
		sprintf( buf+strlen(buf), "%s\r\n", BLACK_BACK );

		/* Seconda linea */
		sprintf( buf+strlen(buf), "&W%c ", 'A' + x );
		for ( y = 0;  y < 8;  y++ )
		{
			if ( (x%2) == 0 )
				strcat( buf, (y%2 == 0) ? WHITE_BACK : BLACK_BACK );
			else
				strcat( buf, (y%2 == 0) ? BLACK_BACK : WHITE_BACK );
			strcat( buf, (board->color[x][y] == COLOR_WHITE) ? WHITE_FORE : BLACK_FORE );
			strcat( buf, (board->color[x][y] == COLOR_WHITE) ? table_pieces[board->piece[x][y]].wgraph1 : table_pieces[board->piece[x][y]].bgraph1 );
		}
		sprintf( buf+strlen(buf), "%s\r\n", BLACK_BACK );

		/* Terza linea */
		sprintf( buf+strlen(buf), "  " );
		for ( y = 0;  y < 8;  y++ )
		{
			if ( (x%2) == 0 )
				strcat( buf, (y%2 == 0) ? WHITE_BACK : BLACK_BACK );
			else
				strcat( buf, (y%2 == 0) ? BLACK_BACK : WHITE_BACK );
			strcat( buf, (board->color[x][y] == COLOR_WHITE) ? WHITE_FORE : BLACK_FORE );
			strcat( buf, (board->color[x][y] == COLOR_WHITE) ? table_pieces[board->piece[x][y]].wgraph2 : table_pieces[board->piece[x][y]].bgraph2 );
		}
		sprintf( buf+strlen(buf), "%s\r\n", BLACK_BACK );
	}

	ch_printf( ch, "%s&w", buf );
}


/*
 * stampa al pg il suo stato attuale di gioco
 */
void show_status( CHAR_DATA *ch, CHESSBOARD_DATA *board )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "print_status: ch passato è NULL" );
		return;
	}

	if ( !board )
	{
		send_to_char( ch, "Non sto giocando a nessuna partita di scacchi.\r\n" );
		return;
	}

	if ( !board->player1 )
		send_to_char( ch, "Non ho nessun giocatore per i pezzi neri con cui gareggiare.\r\n" );
	else if ( board->player1 == ch )
		send_to_char( ch, "Sono io il giocatore dei pezzi neri.\r\n" );
	else
		ch_printf( ch, "%s è il giocatore che tiene i pezzi neri.\r\n", board->player1->name );

	if ( king_in_possible_checkmate(board, PIECE_KING, COLOR_BLACK) )
		send_to_char( ch, "Il re nero potrebbe essere sotto scacco matto.\r\n" );
	else if ( king_in_check(board, PIECE_KING, COLOR_BLACK) > 0 )
		send_to_char( ch, "Il re nero è sotto scacco.\r\n" );

	if ( !board->player2 )
		send_to_char( ch, "Non ho nessun giocatore per i pezzi bianchi con cui gareggiare.\r\n" );
	else if ( board->player2 == ch )
		send_to_char( ch, "Sono io il giocatore dei pezzi bianchi.\r\n" );
	else
		ch_printf( ch, "%s è il giocatore che tiene i pezzi bianchi.\r\n", board->player2->name );

	if ( king_in_possible_checkmate(board, PIECE_KING, COLOR_WHITE) )
		send_to_char( ch, "Il re bianco potrebbe essere sotto scacco matto.\r\n" );
	else if ( king_in_check(board, PIECE_KING, COLOR_WHITE) > 0 )
		send_to_char( ch, "Il re bianco è sotto scacco.\r\n" );

	if ( !board->player2 || !board->player1 )
		return;

	if ( board->moves < 0 )
		send_to_char( ch, "Il gioco non è ancora iniziato.\r\n" );
	else
		ch_printf( ch, "%d turni.\r\n", board->moves );

	if ( board->turn == ch )
		send_to_char( ch, "E' il mio turno.\r\n" );
	else
		ch_printf( ch, "E' il turno di %s.\r\n", board->turn->name );
}


/*
 * Controlla e ritorna il tipo di mossa effettuata
 */
int is_valid_move( CHAR_DATA *ch, CHESSBOARD_DATA *board, int x, int y, int dx, int dy )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_valid_move: ch è NULL" );
		return MOVE_INVALID;
	}

	if ( !board )
	{
		send_log( NULL, LOG_BUG, "is_valid_move: board è NULL" );
		return MOVE_INVALID;
	}

	if ( x < 0 || y < 0 || x >= 8 || y >= 8 )
	{
		send_log( NULL, LOG_BUG, "is_valid_move: coordinate passate errate: x=%d y =%d", x, y );
		return MOVE_INVALID;
	}

	if ( dx < 0 || dy < 0 || dx >= 8 || dy >= 8 )
	{
		send_log( NULL, LOG_BUG, "is_valid_move: coordinate passate errate: dx=%d dy =%d", dx, dy );
		return MOVE_OFFBOARD;
	}

	if ( board->piece[x][y] == PIECE_NONE )
	{
		send_to_char( ch, "Non trovo nessun pezzo in quella casella.\r\n" );
		return MOVE_INVALID;
	}

	if ( x == dx && y == dy )
	{
		send_to_char( ch, "La sorgente e l'arrivo della mossa sono identiche.\r\n" );
		return MOVE_INVALID;
	}

	if ( (board->color[x][y] == COLOR_WHITE && ch == board->player1)
	  || (board->color[x][y] == COLOR_BLACK && ch == board->player2) )
	{
		send_to_char( ch, "Sto cercando di muovere un pezzo che non è del mio colore.\r\n" );
		return MOVE_WRONGCOLOR;
	}

	switch ( board->piece[x][y] )
	{
	  default:
		send_log( NULL, LOG_BUG, "is_valid_move():chess.c invaild piece: %d", board->piece[x][y] );
		send_to_char( ch, "-> Mossa sbagliata, avverti i Coder <-" );
		return MOVE_INVALID;
		break;

	  case PIECE_PAWN:
		if ( board->color[x][y] == COLOR_WHITE && dx == x+2 && x == 1 && dy == y
		  && board->piece[dx][dy] == PIECE_NONE && board->piece[x+1][dy] == PIECE_NONE )
			return MOVE_OK;
		if ( board->color[x][y] == COLOR_BLACK && dx == x-2 && x == 6 && dy == y
		  && board->piece[dx][dy] == PIECE_NONE && board->piece[x-1][dy] == PIECE_NONE )
			return MOVE_OK;

		if ( (board->color[x][y] == COLOR_WHITE && dx != x+1)
		  || (board->color[x][y] == COLOR_BLACK && dx != x-1) )
		{
			send_to_char( ch, "Debbo muovere il pedone di un passo per volta, solo dalle caselle di partenza del pedone è consentito una mossa da 2.\r\n" );
			return MOVE_INVALID;
		}

		if ( dy != y && dy != y-1 && dy != y+1 )
		{
			send_to_char( ch, "Debbo muovere il pedone sempre in avanti, oppure in diagonale di una casella quando può fare gambetto.\r\n" );
			return MOVE_INVALID;
		}

		if ( dy == y )
		{
			if ( board->piece[dx][dy] == PIECE_NONE )
				return MOVE_OK;
			if ( SAME_COLOR(x, y, dx, dy) )
			{
				send_to_char( ch, "Non posso muovere dove c'è un'altro pezzo del mio stesso colore.\r\n" );
				return MOVE_SAMECOLOR;
			}

			send_to_char( ch, "Non posso eseguire questa mossa.\r\n" );
			return MOVE_BLOCKED;
		}
		else
		{
			if ( board->piece[dx][dy] == PIECE_NONE )
			{
				/* (FF) (bb) Bisognerebbe gestire anche il gambetto */
				send_to_char( ch, "Non c'è nessun pezzo da mangiare in quella casella.\r\n" );
				return MOVE_INVALID;
			}
			if ( SAME_COLOR(x, y, dx, dy) )
			{
				send_to_char( ch, "Non posso mangiare un pezzo del mio stesso colore.\r\n" );
				return MOVE_SAMECOLOR;
			}
			if ( board->piece[dx][dy] != PIECE_KING )
				return MOVE_TAKEN;

			send_to_char( ch, "Non posso fare questa mossa.\r\n" );
			return MOVE_INVALID;
		}
		break;

	  case PIECE_ROOK:
	  {
		int cnt;

		if ( dx != x && dy != y )
		{
			send_to_char( ch, "Devo muovere la torre verticalmente o orizzontalmente dalla propria posizione.\r\n" );
			return MOVE_INVALID;
		}

		if ( dx == x )
		{
			for ( cnt = y;  cnt != dy; )
			{
				if ( cnt != y && board->piece[x][cnt] != PIECE_NONE )
				{
					send_to_char( ch, "Non posso spostare la torre fino a lì, qualche pezzo blocca la strada" );
					return MOVE_BLOCKED;
				}
				if ( dy > y )
					cnt++;
				else
					cnt--;
			}
		}
		else if ( dy == y )
		{
			for ( cnt = x;  cnt != dx; )
			{
				if ( cnt != x && board->piece[cnt][y] != PIECE_NONE )
				{
					send_to_char( ch, "Non posso spostare la torre fino a lì, qualche pezzo blocca la strada" );
					return MOVE_BLOCKED;
				}
				if ( dx > x )
					cnt++;
				else
					cnt--;
			}
		}

		if ( board->piece[dx][dy] == PIECE_NONE )
			return MOVE_OK;

		if ( SAME_COLOR(x, y, dx, dy) )
		{
			send_to_char( ch, "Non posso mangiare un pezzo del mio stesso colore.\r\n" );
			return MOVE_SAMECOLOR;
		}

		return MOVE_TAKEN;
	  }
	  break;

	  case PIECE_KNIGHT:
		if ( (dx == x - 2 && dy == y - 1)
		  || (dx == x - 2 && dy == y + 1)
		  || (dx == x - 1 && dy == y - 2)
		  || (dx == x - 1 && dy == y + 2)
		  || (dx == x + 1 && dy == y - 2)
		  || (dx == x + 1 && dy == y + 2)
		  || (dx == x + 2 && dy == y - 1)
		  || (dx == x + 2 && dy == y + 1) )
		{
			if ( board->piece[dx][dy] == PIECE_NONE )
				return MOVE_OK;
			if ( SAME_COLOR(x, y, dx, dy) )
			{
				send_to_char( ch, "Non posso mangiare un pezzo del mio stesso colore.\r\n" );
				return MOVE_SAMECOLOR;
			}

			return MOVE_TAKEN;
		}
		send_to_char( ch, "Devo posso muovere il mio cavallo così.\r\n" );
		return MOVE_INVALID;
		break;

	  case PIECE_BISHOP:
	  {
		int	l;
		int	m;
		int	blocked = FALSE;

		if ( dx == x || dy == y )
		{
			send_to_char( ch, "Devo muovere l'alfiere diagonalmente dalla sua posizione.\r\n" );
			return MOVE_INVALID;
		}

		l = x;
		m = y;

		while ( 1 )
		{
			if ( dx > x )
				l++;
			else
				l--;

			if ( dy > y )
				m++;
			else
				m--;

			if ( l > 7 || m > 7 || l < 0 || m < 0 )
			{
				send_to_char( ch, "Non posso uscire dalla scacchiera.\r\n" );
				return MOVE_INVALID;
			}
			if ( l == dx && m == dy )
				break;

			if ( board->piece[l][m] != PIECE_NONE )
				blocked = TRUE;
		}

		if ( l != dx || m != dy )
		{
			send_to_char( ch, "Devo muovere l'alfiere diagonalmente dalla sua posizione.\r\n" );
			return MOVE_INVALID;
		}

		if ( blocked )
		{
			send_to_char( ch, "Non posso spostare il mio alfiere lì, qualche pezzo blocca la strada" );
			return MOVE_BLOCKED;
		}

		if ( board->piece[dx][dy] == PIECE_NONE )
			return MOVE_OK;

		if ( SAME_COLOR(x, y, dx, dy) )
		{
			send_to_char( ch, "Non posso mangaire un pezzo del mio stesso colore.\r\n" );
			return MOVE_SAMECOLOR;
		}

		return MOVE_TAKEN;
	  }
	  break;

	  case PIECE_QUEEN:
	  {
		int l;
		int m;
		int blocked = FALSE;

		l = x;
		m = y;

		while ( 1 )
		{
			if ( dx > x )
				l++;
			else if ( dx < x )
				l--;

			if ( dy > y )
				m++;
			else if ( dy < y )
				m--;

			if ( l > 7 || m > 7 || l < 0 || m < 0 )
			{
				send_to_char( ch, "Non posso muovere fuori dalla scacchiera.\r\n" );
				return MOVE_INVALID;
			}
			if ( l == dx && m == dy )
				break;

			if ( board->piece[l][m] != PIECE_NONE )
				blocked = TRUE;
		}

		if ( l != dx || m != dy )
		{
			send_to_char( ch, "Devo muovere la regina orizzonalmente, verticalmente o in diagonale dalla sua posizione.\r\n" );
			return MOVE_INVALID;
		}

		if ( blocked )
		{
			send_to_char( ch, "Non posso spostare la mia regina lì, qualche pezzo blocca la strada" );
			return MOVE_BLOCKED;
		}

		if ( board->piece[dx][dy] == PIECE_NONE )
			return MOVE_OK;

		if ( SAME_COLOR (x, y, dx, dy) )
		{
			send_to_char( ch, "Non posso mangiare un pezzo del mio stesso colore.\r\n" );
			return MOVE_SAMECOLOR;
		}

		return MOVE_TAKEN;
	  }
	  break;

	  case PIECE_KING:
	  {
		int		sp;
		int		spc;
		int		sk;
		int		skc;
		bool	incheck = FALSE;;

		if ( dx > x+1 || dx < x-1 || dy > y+1 || dy < y-1 )
		{
			send_to_char( ch, "Posso muovere il re di un solo passo attorno a sè, non oltre.\r\n" );
			return MOVE_INVALID;
		}

		if ( SAME_COLOR(x, y, dx, dy) )
		{
			send_to_char( ch, "Non posso mangiare un pezzo del mio stesso colore.\r\n" );
			return MOVE_SAMECOLOR;
		}

		sk	= board->piece[x][y];
		skc = board->color[x][y];
		sp	= board->piece[dx][dy];
		spc	= board->color[dx][dy];
		board->piece[x][y] = PIECE_NONE;
		board->piece[x][y] = COLOR_NONE;
		board->piece[dx][dy] = sk;
		board->color[dx][dy] = skc;

		if ( king_in_check(board, sk, skc) > 0 )
			incheck = TRUE;

		board->piece[x][y]	 = sk;
		board->color[x][y]	 = skc;
		board->piece[dx][dy] = sp;
		board->color[dx][dy] = spc;

		if ( incheck )
		{
			send_to_char( ch, "Non posso muovere il re in quella casella, sarebbe sotto scacco.\r\n" );
			return MOVE_CHECK;
		}

		if ( board->piece[dx][dy] == PIECE_NONE )
			return MOVE_OK;

		return MOVE_TAKEN;
	  }
	  break;
	}

	send_log( NULL, LOG_BUG, "is_valid_move: shouldn't get here" );
	send_to_char( ch, "-> Mossa sbagliata, avverti i Coder <-" );
	return MOVE_INVALID;
}


/*
 * Aumenta le mosse della partita e passa il turno all'altro giocatore
 */
void board_move_stuff( CHESSBOARD_DATA *board )
{
	board->moves++;

	if ( board->turn == board->player1 )
		board->turn = board->player2;
	else
		board->turn = board->player1;
}


/*
 * Invia i messaggi delle mosse ai giocatori e a quelli della stanza
 */
void board_move_messages( CHAR_DATA *ch, int ret, char *extra )
{
	CHESSBOARD_DATA *board;
	CHAR_DATA		*opp;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "board_move_messages: ch passato è NULL" );
		return;
	}

	if ( ret < 0 || ret >= MAX_MOVE )
	{
		send_log( NULL, LOG_BUG, "board_move_messages: ret passato errato: %d", ret );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono giocare a scacchi.\r\n" );
		return;
	}

	board = get_chessboard( ch );
	if ( !board )
	{
		send_to_char( ch, "Non sto giocando a nessuna partita di scacchi.\r\n" );
		return;
	}

	if ( ch == board->player1 )
		opp = board->player2;
	else
		opp = board->player1;

	if ( !opp )
	{
		send_log( NULL, LOG_BUG, "board_move_messages: %s non ha nessun avversario", ch->name );
		return;
	}

	show_board( ch, board );
	show_board( opp, board );

	switch ( ret )
	{
	  case MOVE_OK:
		ch_printf( ch, "Muovo, %s.\r\n", extra );
		act( AT_ACTION, "$n muove, $t.\r\n", ch, extra, NULL, TO_ROOM );
		break;
	  case MOVE_CASTLE:
		ch_printf( ch, "Arrocco, %s.\r\n", extra );
		act( AT_ACTION, "$n arrocca, $t.", ch, extra, NULL, TO_ROOM );
		break;
	  case MOVE_PROMOTE:
		ch_printf( ch, "Promuovo un pedone a %s.\r\n", extra );
		act( AT_ACTION, "$n promuove un pedone a $t.", ch, extra, NULL, TO_ROOM );
		break;
	  case MOVE_INVALID:
		send_to_char( ch, "Non posso effettuare questa mossa.\r\n" );
		break;
	  case MOVE_BLOCKED:
		send_to_char( ch, "Sono bloccato in quella direzione.\r\n" );
		break;
	  case MOVE_TAKEN:
		send_to_char( ch, "Hai catturato un pezzo nemico!\r\n" );
		act( AT_ACTION, "$n muove, $t, e cattura uno dei tuoi pezzi!", ch, extra, NULL, TO_ROOM );
		break;
	  case MOVE_OFFBOARD:
		send_to_char( ch, "Questa mossa mi porterebbe fuori la scacchiera.\r\n" );
		break;
	  case MOVE_SAMECOLOR:
		send_to_char( ch, "Uno dei miei stessi pezzi mi blocca la via.\r\n" );
		break;
	  case MOVE_CHECK:
		act( AT_ACTION, "Con questa mossa metto sotto scacco il re di $N.\r\n", ch, NULL, opp, TO_CHAR );
		act( AT_ACTION, "$n prova una mossa e il re di $N è sotto scacco.", ch, NULL, opp, TO_NOVICT );
		act( AT_ACTION, "$n prova una mossa e il tuo re è sotto scacco.", ch, NULL, opp, TO_VICT );
		break;
	  case MOVE_WRONGCOLOR:
		send_to_char( ch, "Questo non è uno dei miei pezzi.\r\n" );
		break;
	  case MOVE_INCHECK:
		send_to_char( ch, "Con questa mossa il mio re sarebbe sotto scacco, non posso eseguirla.\r\n" );
		break;
	}
}


/*
 * Ricava dalla stringa di coordinate passato le coordinate numeriche della scacchiera
 */
void get_coord( char *argument, int *x, int *y )
{
	*x = -1;
	*y = -1;

	if ( !VALID_STR(argument) )
	{
		send_log( NULL, LOG_BUG, "get_coord: argument pasato è NULL" );
		return;
	}

	*x = argument[0] - 'a';
	*y = atoi( argument+1 ) - 1;
}


/*
 * Comando per gestire il gioco degli scacchi
 */
void do_chess( CHAR_DATA *ch, char *argument )
{
	CHESSBOARD_DATA *board;
	char			 arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_chess: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono giocare a scacchi.\r\n" );
		return;
	}

	board = get_chessboard( ch );

	if ( !VALID_STR(argument) || is_name(argument, "sintassi aiuto syntax help ?") )
	{
		char	*cmd;

		cmd = translate_command( ch, "chess" );
		ch_printf( ch, "&YSintassi gioco&w:  %s inizio|smetto|partecipo|forfeit\r\n", cmd );
		ch_printf( ch, "&YSintassi info&w:   %s pezzi\r\n", cmd );
		ch_printf( ch, "&YSintassi mosse&w:  %s muovo <sorgente> <destinazione> [comandi opzionali]\r\n", cmd );
		ch_printf( ch, "&YSintassi extra&w:  %s arrocco|promuovo\r\n", cmd );

		if ( !VALID_STR(argument) && board )
		{
			send_to_char( ch, "\r\n" );
			show_status( ch, board );
			send_to_char( ch, "\r\n" );
			show_board( ch, board );
		}

		return;
	}

	argument = one_argument( argument, arg );

	if ( is_name_prefix(arg, "inizio inizia comincio comincia cominciare start") )
	{
		CHESSBOARD_DATA *newboard;

		if ( board )
		{
			send_to_char( ch, "Sto già partecipando ad una partita di scacchi.\r\n" );
			return;
		}

		CREATE( newboard, CHESSBOARD_DATA, 1 );
		init_board( newboard );
		newboard->player1	= ch;
		newboard->turn		= ch;

		LINK( newboard, first_chessboard, last_chessboard, next, prev );
		top_chessboard++;

		send_to_char( ch, "Inizio una nuova partita di scacchi.\r\n" );
		return;
	}

	if ( is_name_prefix(arg, "partecipa partecipo join") )
	{
		CHESSBOARD_DATA *vboard = NULL;
		CHAR_DATA		*vch;
		char			 arg2[MIL];

		if ( board )
		{
			send_to_char( ch, "Sto già partecipando ad una partita di scacchi.\r\n" );
			return;
		}

		argument = one_argument( argument, arg2 );
		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Con chi devo partecipare ad una partita di scacchi?\r\n" );
			return;
		}

		vch = get_player_room( ch, arg2, TRUE );
		if ( !vch )
		{
			ch_printf( ch, "Non vedo nessun %s nella stanza.\r\n", arg2 );
			return;
		}

		vboard = get_chessboard( vch );
		if ( !vboard )
		{
			send_to_char( ch, "Non sta giocando a scacchi.\r\n" );
			return;
		}

		if ( vboard->player2 )
		{
			send_to_char( ch, "Questa scacchiera ha già due giocatori.\r\n" );
			return;
		}

		vboard->player2 = ch;
		vboard->turn	= vboard->player2;

		send_to_char( ch, "Mi unisco alla partita, è il mio turno.\r\n" );
		ch_printf( vboard->player1, "%s si unisce alla tua partita.\r\n", ch->name );
		return;
	}

	if ( is_name_prefix(arg, "pezzi pieces") )
	{
		int		x;	/* contatore dei colori */
		int		y;	/* contatore dei pezzi */

		for ( x = 0;  x < COLOR_NONE;  x++ )
		{
			if ( x == COLOR_BLACK )
				send_to_char( ch, "\r\n\r\nPezzi neri:\r\n" );
			else
				send_to_char( ch, "Pezzi bianchi:\r\n" );

			for ( y = PIECE_PAWN;  y < PIECE_NONE;  y++ )
				ch_printf( ch, "%-7s", table_pieces[y].name );

			send_to_char( ch, "\r\n" );
			for ( y = PIECE_PAWN;  y < PIECE_NONE;  y++ )
				send_to_char( ch, (x == COLOR_WHITE) ? table_pieces[y].wgraph1 : table_pieces[y].bgraph1 );

			send_to_char( ch, "\r\n" );
			for ( y = PIECE_PAWN;  y < PIECE_NONE;  y++ )
				send_to_char( ch, (x == COLOR_WHITE) ? table_pieces[y].wgraph2 : table_pieces[y].bgraph2 );
		}
		send_to_char( ch, "\r\n" );
		return;
	}

	if ( !board )
	{
		send_to_char( ch, "Non ho iniziato o non sto partecipando a nessuna partita di scacchi.\r\n" );
		return;
	}

	if ( is_name_prefix(arg, "smetto fermo stop") )
	{
		free_chessboard( board );
		return;
	}

	if ( is_name_prefix(arg, "forfeit") )
	{
		send_to_char( ch, "Dò forfeit così perdendo.\r\n" );
		free_chessboard( board );
		return;
	}

	if ( !board->player1 || !board->player2 )
	{
		send_to_char( ch, "C'è solo un giocatore.\r\n" );
		return;
	}

	if ( board->moves < 0 )
	{
		send_to_char( ch, "Il gioco non è ancora iniziato.\r\n" );
		return;
	}

	if ( is_name_prefix(arg, "promuovo promuovi promote") )
	{
		int		piece = board->piece[board->lastx][board->lasty];
		char	extra[MIL];

		if ( !((piece == PIECE_PAWN && board->player1 == ch)
		  ||   (piece == PIECE_PAWN && board->player2 == ch)) )
		{
			send_to_char( ch, "Non posso promuovere questo pezzo.\r\n" );
			return;
		}

		if ( (piece == PIECE_PAWN && board->lastx != 0)
		  || (piece == PIECE_PAWN && board->lastx != 7) )
		{
			send_to_char( ch, "Puoi promuovere solamente i pedoni che hanno raggiunto l'altro lato della scacchiera.\r\n" );
			return;
		}

		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Vorrei promuovere il pedone in che cosa?\r\n" );
			return;
		}

		if		( is_name_prefix(argument, "regina queen"  ) )	piece = PIECE_QUEEN;
		else if ( is_name_prefix(argument, "alfiere bishop") )	piece = PIECE_BISHOP;
		else if ( is_name_prefix(argument, "cavallo knight") )	piece = PIECE_KNIGHT;
		else if ( is_name_prefix(argument, "torre rook"	   ) )	piece = PIECE_ROOK;
		else
		{
			ch_printf( ch, "Non posso promuoverlo a %s.\r\n", argument );
			return;
		}

		board->piece[board->lastx][board->lasty] = piece;
		sprintf( extra, "%s (%c%d)", table_pieces[piece].name, board->lastx+'a', board->lasty+1 );
		board_move_messages( ch, MOVE_PROMOTE, extra );
		return;
	}

	if ( board->turn != ch )
	{
		send_to_char( ch, "Non è il mio turno.\r\n" );
		return;
	}

	if ( is_name_prefix(arg, "arrocco") )
	{
		int 	myx;
		int 	rooky;
		int 	kdy;
		int		rdy;
		bool	fRookShort;

		if ( king_in_check(board, PIECE_KING, (board->player1 == ch) ? COLOR_BLACK : COLOR_WHITE) > 0 )
		{
			send_to_char( ch, "Non posso eseguire un arrocco mentre sono sotto scacco.\r\n" );
			return;
		}

		if ( (board->player1 == ch && HAS_BIT(board->flags1, CHESSFLAG_MOVEDKING))
		  || (board->player2 == ch && HAS_BIT(board->flags2, CHESSFLAG_MOVEDKING)) )
		{
			send_to_char( ch, "Non posso effettuare un arrocco quando ho già mosso il mio re.\r\n" );
			return;
		}
		myx = (board->player1 == ch) ? 7 : 0;

		if ( !VALID_STR(argument) )
		{
			ch_printf( ch, "Utilizzo: %s arrocco corto|lungo\r\n", translate_command(ch, "chess") );
			return;
		}

		if ( is_name_prefix(argument, "corto short") )
			fRookShort = TRUE;
		else if ( is_name_prefix(argument, "lungo long") )
			fRookShort = FALSE;
		else
		{
			send_command( ch, "chess arrocco", CO );
			return;
		}

		if ( (board->player1 == ch && HAS_BIT(board->flags1, fRookShort ? CHESSFLAG_MOVEDKROOK : CHESSFLAG_MOVEDQROOK))
		  || (board->player2 == ch && HAS_BIT(board->flags2, fRookShort ? CHESSFLAG_MOVEDKROOK : CHESSFLAG_MOVEDQROOK)) )
		{
			ch_printf( ch, "Non posso effettuare l'arrocco %s perchè ho già mosso la torre prima.\r\n",
				fRookShort ? "corto" : "lungo" );
			return;
		}
		rooky = fRookShort ? 7 : 0;

		if ( ( fRookShort && (board->piece[myx][6] != PIECE_NONE || board->piece[myx][5] != PIECE_NONE))
		  || (!fRookShort && (board->piece[myx][1] != PIECE_NONE || board->piece[myx][2] != PIECE_NONE || board->piece[myx][3] != PIECE_NONE)) )
		{
			send_to_char( ch, "L'arrocco è bloccato dalla presenza di pezzi tra il re e la torre.\r\n" );
			return;
		}

		/* castling succeeded */
		if ( fRookShort )
		{
			kdy = 6;
			rdy = 5;
		}
		else
		{
			kdy = 2;
			rdy = 3;
		}

/* (FF) (TT) (RR) (bb) ricordo che una qualsiasi delle caselle, in cui avveniva l'arrocco
 *	non dovevano essere sotto scacco, mi sa che qui non è così, o forse ricordo sbagliato */

		/* check for 'move across check' rule */
		board->piece[myx][rdy] = board->piece[myx][4];
		board->piece[myx][4] = PIECE_NONE;

		if ( king_in_check(board, board->piece[myx][rdy], board->color[myx][rdy]) > 0 )
		{
			send_to_char( ch, "Il mio re si troverebbe sotto scacco dopo l'arrocco.\r\n" );

			board->piece[myx][4] = board->piece[myx][rdy];
			board->piece[myx][rdy] = PIECE_NONE;
			return;
		}

		board->piece[myx][kdy] = board->piece[myx][rdy];
		board->piece[myx][rdy] = board->piece[myx][rooky];
		board->piece[myx][rooky] = PIECE_NONE;

		/* check for 'check' after castled */
		if ( king_in_check(board, board->piece[myx][kdy], board->color[myx][kdy]) > 0 )
		{
			send_to_char( ch, "Il mio re si troverebbe sotto scacco dopo l'arrocco.\r\n" );

			board->piece[myx][4] = board->piece[myx][kdy];
			board->piece[myx][kdy] = PIECE_NONE;
			board->piece[myx][rooky] = board->piece[myx][rdy];
			board->piece[myx][rdy] = PIECE_NONE;
			return;
		}

		/* Basta indicare che è stato mosso il re per evitare un altro arrocco */
		if ( board->player1 == ch )
			SET_BIT( board->flags1, CHESSFLAG_MOVEDKING );
		else
			SET_BIT( board->flags2, CHESSFLAG_MOVEDKING );

		board_move_stuff( board );
		board_move_messages( ch, MOVE_CASTLE, rooky == 7 ? "corto" : "lungo" );
	}

	if ( is_name_prefix(arg, "muovo move") )
	{
		char	coord1[MIL];
		char	coord2[MIL];
		char	extra[MIL];
		int		x, y, dx, dy;
		int		ret;

		if ( !VALID_STR(argument) )
		{
			ch_printf( ch, "Utilizzo: %s muovo <sorgente> <destinazione>\r\n", translate_command(ch, "chess") );
			return;
		}

		argument = one_argument( argument, coord1 );
		argument = one_argument( argument, coord2 );

		if ( !VALID_STR(coord1) || !VALID_STR(coord2) )
		{
			ch_printf( ch, "Utilizzo: %s muovo <sorgente> <destinazione>\r\n", translate_command(ch, "chess") );
			return;
		}

		get_coord( coord1, &x, &y );
		get_coord( coord2, &dx, &dy );

		if ( x < 0 || x >= 8 || dx < 0 || dx >= 8
		  || y < 0 || y >= 8 || dy < 0 || dy >= 8 )
		{
			send_to_char( ch, "Mossa non valida, utilizza a-h e 1-8 (esempio: a4 b4).\r\n" );
			return;
		}

		extra[0] = '\0';

		ret = is_valid_move( ch, board, x, y, dx, dy );
		if ( ret == MOVE_OK || ret == MOVE_TAKEN )
		{
			int	piece;
			int	color;
			int	destpiece;
			int	destcolor;

			piece	  = board->piece[x][y];
			color	  = board->color[x][y];
			destpiece = board->piece[dx][dy];
			destcolor = board->color[dx][dy];

			board->piece[dx][dy] = piece;
			board->color[dx][dy] = color;
			board->piece[x][y] = PIECE_NONE;
			board->color[x][y] = COLOR_NONE;

			if ( king_in_check(board, PIECE_KING, board->color[dx][dy]) > 0 )
			{
				board->piece[dx][dy] = destpiece;
				board->color[dx][dy] = destcolor;
				board->piece[x][y]	 = piece;
				board->color[x][y]	 = color;
				ret = MOVE_INCHECK;
			}
			else
			{
				if ( destpiece == PIECE_NONE )
					sprintf( extra, "%c%d (%s) alle coordinate %c%d", x+'a', y+1, table_pieces[piece].name, y+'a', dy+1 );
				else
					sprintf( extra, "%c%d (%s) alle coordinate %c%d (%s)", x+'a', y+1, table_pieces[piece].name, y+'a', dy+1, table_pieces[destpiece].name );

				board_move_stuff( board );
				board->lastx = dx;
				board->lasty = dy;

				/* Imposta le flag per evitare gli arrocchi */
				if ( piece == PIECE_ROOK )
				{
					if ( color == COLOR_WHITE )
					{
						if ( y == 0 && x == 0 )
							SET_BIT( board->flags1, CHESSFLAG_MOVEDKROOK );
						else if ( y == 0 && x == 7 )
							SET_BIT( board->flags1, CHESSFLAG_MOVEDQROOK );
					}
					else
					{
						if ( y == 7 && x == 0 )
							SET_BIT( board->flags2, CHESSFLAG_MOVEDKROOK );
						else if ( y == 7 && x == 7 )
							SET_BIT( board->flags2, CHESSFLAG_MOVEDQROOK );
					}
				}
				else if ( piece == PIECE_KING )
				{
					if ( color == COLOR_WHITE )
						SET_BIT( board->flags1, CHESSFLAG_MOVEDKING );
					else
						SET_BIT( board->flags2, CHESSFLAG_MOVEDKING );
				}
			}

			board_move_messages( ch, ret, extra );
		}

		/* Così qui gestisce i comandi opzionali, come il promote */
		if ( VALID_STR(argument) )
		{
			do_chess( ch, argument );
			return;
		}
		return;
	}

	send_command( ch, "chess aiuto", CO );
}


#endif	/* T2_CHESS */
