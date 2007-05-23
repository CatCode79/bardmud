#ifndef COLOR_H
#define COLOR_H


/*****************************************************************************
 *               Color Module -- Allow user customizable Colors.            *
 *                                   --Matthew                              *
 *                      Enhanced ANSI parser by Samson                      *
 ****************************************************************************/


/*
 * Other miscelaneous ANSI tags that can be used
 */
#define ANSI_RESET		"\033[0m"		/* Setta i colori di default del client */
#define ANSI_BOLD		"\033[1m"		/* For bright color stuff */
#define ANSI_ITALIC		"\033[3m"		/* Italic text			  */
#define ANSI_UNDERLINE	"\033[4m"		/* Underline text		  */
#define ANSI_BLINK		"\033[5m"		/* Blinking text		  */
#define ANSI_REVERSE	"\033[7m"		/* Reverse colors		  */
#define ANSI_STRIKEOUT	"\033[9m"		/* Overstrike line		  */
#define ANSI_CLEAR		"\033[2J"		/* Cancella lo schermo	  */


/*
 * Tipo di invio di colore, utilizzati per la set_color_char e set_color_pager
 */
#define	CLR_FORE	0
#define	CLR_BACK	1
#define	CLR_BLINK	2


/*
 * Enumerazione dei colori di base per il Mud.
 */
typedef enum
{
	AT_BLACK,
	AT_DRED,
	AT_DGREEN,
	AT_ORANGE,
	AT_DBLUE,
	AT_MAGENTA,
	AT_CYAN,
	AT_GREY,
	AT_DGREY,
	AT_RED,
	AT_GREEN,
	AT_YELLOW,
	AT_BLUE,
	AT_PINK,
	AT_LCYAN,
	AT_WHITE,
	AT_NONE		/* 17esimo, è il NONE per gestire alcune cose, tipo che non vi è tinta nei capelli */
	/* Non serve mettere un MAX_ suvvia, tanto si sa che sono 16 e che rimangono tali */
} base_colors_types;

#define MAX_COLOR	16	/* Massimo dei colori telnet di foreground, non conta AT_NONE */


/*
 * Enumerazione dei colori AT utilizzati nel set di default dei colori del mud.
 * (FF) alcuni di questi colori non hanno riferimento nel codice, bisognerebbe fare un
 *	controllo e colorare un po' oppure toglierli.
 */
typedef enum
{
	AT_BLINK = 16,
	AT_PLAIN,
	AT_ACTION,
	AT_MURMUR,
	AT_WHISPER,
	AT_SAY,
	AT_YELL,
	AT_SHOUT,
	AT_TELL,
	AT_HIT,
	AT_HITME,
	AT_ADMIN,
	AT_HURT,
	AT_FALLING,
	AT_DANGER,
	AT_MAGIC,
	AT_CONSIDER,
	AT_POISON,
	AT_SOCIAL,
	AT_DYING,
	AT_DEAD,
	AT_SKILL,
	AT_CARNAGE,
	AT_DAMAGE,
	AT_FLEE,
	AT_RMNAME,
	AT_RMDESC,
	AT_OBJECT,
	AT_PERSON,
	AT_LIST,
	AT_BYE,
	AT_GOLD,
	AT_GTELL,
	AT_NOTE,
	AT_HUNGRY,
	AT_THIRSTY,
	AT_FIRE,
	AT_SOBER,
	AT_WEAROFF,
	AT_EXITS,
	AT_SCORE,
	AT_RESET,
	AT_LOG,
	AT_DIEMSG,
	AT_ARENA,
	AT_THINK,
	AT_AFLAGS,		/* for area flag display line */
	AT_WHO,			/* for wholist */
	AT_DIVIDER,		/* for version 1.4 code */
	AT_MORPH,		/* for version 1.4 code */
	AT_RFLAGS,		/* for room flag display line */
	AT_STYPE,		/* for sector display line */
	AT_ANAME,		/* for filename display line */
	AT_SCORE2,
	AT_WHO2,
	AT_HELP,		/* for helpfiles */
	AT_PRAC,
	AT_PRAC2,
	AT_COMMAND,		/* 57 */
	AT_SING,
	MAX_AT_COLORS
} at_colors_types;


/*
 * Struttura per la tabella dei colori.
 */
struct color_type
{
	char		 clr;		/* Carattere per il colore formato da & */
	CODE_DATA	 code;		/* Codici per i colori */
	CODE_DATA	 hair;		/* Codici per i colori dei capelli */
	char		*ansi[3];	/* Codici ansi per l'invio dei colori: fore, back e blink */
	char		*html;		/* Codici di colore web */
};


/*
 * Struttura per il set di default dei colori
 */
struct color_set_type
{
	char	*display;
	int		 code;
};


/*
 * Costanti esterne
 */
extern	const struct color_type		table_color[];
extern	const struct color_set_type	color_def_set[];
extern	const int					max_check_color;


/*
 * Funzioni globali
 */
char		get_clr			( const int category );
void		reset_colors	( CHAR_DATA *ch );
void		set_char_color	( int AType, CHAR_DATA *ch );
char	   *color_str		( int AType, CHAR_DATA *ch );
int			strlen_nocolor	( char *argument );
int			colorcode		( const char *col, char *code, CHAR_DATA *ch );
char	   *strip_color		( char *text );
char		random_color	( void );


#endif	/* COLOR_H */
