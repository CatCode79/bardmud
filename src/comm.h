#ifndef COMM_H
#define COMM_H


/*
 * Tipi di TO_ per la funzione act
 * E anche per le funzioni echo
 */
typedef enum
{
	TO_ROOM,		/* Messaggio inviato a tutti quelli della stanza tranne al pg */
	TO_NOVICT,		/* Messaggio inviato a tutti quelli della stanza tranne alla vittima */
	TO_VICT,		/* Messaggio inviato alla vittima */
	TO_CHAR,		/* Messaggio inviato al pg */
	TO_AREA,		/* Messaggio inviato all'area */
	TO_MUD,			/* Messaggio inviato a tutto il Mud	*/
	TO_ADMIN,		/* Messaggio inviato solo agli amministratori della stanza */
	TO_ARENA,		/* Messaggio a tutti quelli in arena */
#ifdef T2_ALFA
	TO_AROUND,
	TO_CAPTAIN,		/* Messaggio al capitano di una ship */
#endif
	MAX_TO
} to_types;


/*
 * Tipologie per la act_string
 */
#define STRING_NONE		0
#define STRING_ADMIN	1


/*
 * Struttura di un descrittore
 */
struct descriptor_data
{
	DESCRIPTOR_DATA *next;
	DESCRIPTOR_DATA *prev;
	DESCRIPTOR_DATA *snoop_by;
	CHAR_DATA 		*character;
	CHAR_DATA 		*original;
	char 			*host_old;
	char 			*host;
	char			*name;
	char			*password;
	int				 tempnum;
#ifdef T2_DETCLIENT
	char			*client;
#endif
	int				 port;
	int				 descriptor;
	int				 connected;
	int				 idle;
	int				 lines;
	bool			 fcommand;
	char			 inbuf [MAX_INBUF_SIZE];
	char			 incomm[MIL];
	char			 inlast[MIL];
	int				 repeat;
	char 	 		*outbuf;
	int		 		 outsize;
	int				 outtop;
	char 	 		*pagebuf;
	unsigned long	 pagesize;
	char			 pagecmd;
	char			 pagecolor;
	OLC_DATA 		*olc;
	int				 total_output;
	int				 reroll;
#ifdef T2_MCCP
	MCCP_DATA		*mccp;
#endif
#ifdef T2_MSP
	bool			 msp;
#endif
#ifdef T2_MXP
	bool			 mxp;
#endif
	int				 animation;				/* Quale animazione è inviata (se -1 nessuna) */
	int				 frame;					/* A quale frame è arrivato ad inviare (-1 se nessuno) */
};


/*
 * Variabili
 */
extern	DESCRIPTOR_DATA		   *first_descriptor;
extern	DESCRIPTOR_DATA		   *last_descriptor;
extern	int						num_descriptors;

extern	const /*unsigned*/ char		echo_off_str	[];		/* nanny.c */
extern	const /*unsigned*/ char		echo_on_str		[];		/* nanny.c */
extern	const /*unsigned*/ char		do_termtype_str	[];		/* nanny.c */
extern	const unsigned char			eor_on_str		[];		/* nanny.c, comm.c */


/*
 * Funzioni
 */
void	init_descriptor				( DESCRIPTOR_DATA *dnew, int desc );
void	free_desc					( DESCRIPTOR_DATA *d );
char   *get_host					( CHAR_DATA *ch );
void	write_to_pager				( DESCRIPTOR_DATA *d, const char *txt, int length );
bool	is_idle						( CHAR_DATA *ch );
void	close_socket				( DESCRIPTOR_DATA *dclose, bool force );
bool	write_to_descriptor			( DESCRIPTOR_DATA *d, char *txt, int length );
void	write_to_buffer				( DESCRIPTOR_DATA *d, const char *txt, int length );
void	write_to_buffer_private		( DESCRIPTOR_DATA *d, const char *txt, int length );
void	send_to_descriptor			( DESCRIPTOR_DATA *d, const char *txt );
void	send_to_char				( CHAR_DATA *ch, const char *txt );
void	send_to_pager				( CHAR_DATA *ch, const char *txt );
void	ch_printf					( CHAR_DATA *ch, const char *fmt, ... )	__attribute__ ( (format(printf, 2, 3)) );
void	pager_printf				( CHAR_DATA *ch, const char *fmt, ... )	__attribute__ ( (format(printf, 2, 3)) );
void	send_to_descriptor_nocolor	( DESCRIPTOR_DATA *d, const char *txt );
void	send_to_char_nocolor		( CHAR_DATA *ch, const char *txt );
void	send_to_pager_nocolor		( CHAR_DATA *ch, const char *txt );
void	ch_printf_nocolor			( CHAR_DATA *ch, const char *fmt, ... )	__attribute__ ( (format(printf, 2, 3)) );
void	pager_printf_nocolor		( CHAR_DATA *ch, const char *fmt, ... )	__attribute__ ( (format(printf, 2, 3)) );
char   *obj_short					( OBJ_DATA *obj );
void	act							( int AType, const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type );


#endif	/* COMM_H */
