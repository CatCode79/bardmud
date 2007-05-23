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


#include "mud.h"
#include "archetype.h"
#include "gramm.h"
#include "interpret.h"


/*
 * Struttura per la tabella degli Archetipi
 * (FF) aggiungerci i colori
 */
typedef struct archetype_data
{
	char	*name_male;		/* Nome maschile dell'archetipo, cui fa riferimento anche una keyword tra gli help */
	char	*name_fema;		/* Nome femminile dell'archetipo, anche questa keyword di help */
	char	*descr;			/* Breve descrizione dell'archetipo */
} ARCHETYPE_DATA;


/*
 * Tabella degli Archetipi
 * (GR) in futuro gestire i femminili con le funzioni gramm
 * (FF) Scegliere i colori per gli archetipi
 */
const ARCHETYPE_DATA table_archetype[] =
{
	{ "Altruista",			"Altruista",			"Tutti hanno bisogno di un mondo migliore."					},
	{ "Architetto",			"Architetto",			"Voglio costruire qualcosa o un futuro migliore."			},
	{ "Autocrate",			"Autocrate",			"Ho bisogno del potere!"									},
	{ "Bambino",			"Bambina",			    "Ci sarà qualcuno a prendersi cura di me?"					},
	{ "Buffone",			"Buffone",			    "II riso attenua il dolore."								},
	{ "Bullo",				"Bulla",				"La forza è tutto ciò che conta."							},
	{ "Burbero",			"Burbera",			    "Nulla ha valore."											},
	{ "Competitore",		"Competitrice",		    "Devo essere sempre il migliore."							},
	{ "Conformista",		"Conformista",		    "Seguo e assisto."											},
	{ "Deviato",			"Deviata",			    "Esisto solo per soddisfare il mio bisogno.."				},
	{ "Entusiasta",			"Entusiasta",			"E' la passione che mi da la vita!"							},
	{ "Fanatico",			"Fanatica",			    "La Causa è tutto ciò che conta."							},
	{ "Furfante",			"Furfante",			    "Chi può, vince. Chi non può, perde. Io posso."				},
	{ "Galante",			"Galante",			    "Non sono il guastafeste, sono la festa!"					},
	{ "Gaudente",			"Gaudente",			    "La vita è un piacere."										},
	{ "Giudice",			"Giudice",			    "La verità è là fuori."										},
	{ "Martire",			"Martire",			    "Soffro ma per un bene più grande."							},
	{ "Masochista",			"Masochista",			"Mi metto alla prova ogni volta che posso."					},
	{ "Mostro",				"Mostro",				"Sono Dannato! E mi comporto come tale!"					},
	{ "Maestro",			"Maestra",			    "Salvo gli altri attraverso il sapere."						},
	{ "Penitente",			"Penitente",			"La vita è una maledizione da espiare!"						},
	{ "Perfezionista",		"Perfezionista",		"Nulla è sufficientemente buono."							},
	{ "Pianificatore",		"Pianificatrice",		"Sovrintendo ciò che deve essere fatto."					},
	{ "Ribelle",			"Ribelle",			    "Non seguo alcuna regola!"									},
	{ "Sfruttatore",		"Sfruttatrice",		    "Gli altri esistono per il mio bene."						},
	{ "Solitario",			"Solitaria",			"Vado avanti per conto mio."								},
	{ "Sopravvissuto",		"Sopravvissuta",		"Nulla può abbattermi."										},
	{ "Temerario",			"Temeraria",			"Lo slancio è tutto ciò che conta."							},
	{ "Tradizionalista",	"Tradizionalista",	    "Ciò che è sempre stato, sarà in eterno."					},
	{ "Visionario",			"Visionaria",			"C'è qualcosa al di là di tutto questo."					},
	{ "Egoista",			"Egoista",				"La vita è il mio terreno di razzia."						},
	{ "Innovatore",			"Innovatrice",			"C'è sempre un modo migliore per fare le cose."				},
	{ "Difensore",			"Difenditrice",			"I forti devono proteggere i deboli dai malvagi"			},
	{ "Tiranno",			"Tiranno",				"L'unico como per fare le cose è farle alla tua maniera"	},
};


/*
 * Ritorna il nome dell'archetipo di un pg
 */
char *get_archetype( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_archetype: ch è NULL" );
		return NULL;
	}

	if ( IS_MOB(ch) )
		return NULL;


	if ( ch->pg->archetype < -1 && ch->pg->archetype>= MAX_ARCHETYPE )
	{
		send_log( NULL, LOG_BUG, "get_archetype: archetipo del pg %s errato: %d",
			ch->name, ch->pg->archetype );
		return NULL;
	}

	if ( ch->pg->archetype == -1 )
		return "Normale";

	if ( ch->sex == SEX_FEMALE )
		return table_archetype[ch->pg->archetype].name_fema;
	else
		return table_archetype[ch->pg->archetype].name_male;
}


/*
 * Ritorna il numero di archetipo passata una stringa
 * Se non lo trova ritorna -1
 */
int	get_archetype_num( const char *argument )
{
	int		x;

	if ( !VALID_STR(argument) )
		return -1;

	for ( x = 0;  x < MAX_ARCHETYPE;  x++ )
	{
		if ( !str_cmp(argument, table_archetype[x].name_male) )
			return x;
		if ( !str_cmp(argument, table_archetype[x].name_fema) )
			return x;
	}

	for ( x = 0;  x < MAX_ARCHETYPE;  x++ )
	{
		if ( !str_prefix(argument, table_archetype[x].name_male) )
			return x;
		if ( !str_prefix(argument, table_archetype[x].name_fema) )
			return x;
	}

	return -1;
}

/*
 * Ritorna la grandezza sizeof degli archetipi in memoria
 */
int get_sizeof_archetype( void )
{
	return MAX_ARCHETYPE * sizeof( ARCHETYPE_DATA );
}


/*
 * Visualizza tutta la lista degli archetipi
 */
void show_archetype_list( CHAR_DATA *ch )
{
	int		x;

	send_to_char( ch, "Digita aiuto <archetipo> per saperne di più un archetipo in particolare.\r\n" );
	send_to_char( ch, "Esempio:  aiuto giudice\r\n" );
	send_to_char( ch, "Sceglilo con ponderazione, lo potrai modificare solo attraverso l'onirismo.\r\n" );
	for ( x = 0;  x < MAX_ARCHETYPE;  x++ )
	{
		ch_printf( ch, "&W%2d&w] %-15s \"%s\"\r\n",
			x+1,
			(ch->sex == SEX_FEMALE) ? table_archetype[x].name_fema : table_archetype[x].name_male,
			table_archetype[x].descr );
	}
}


/*
 * Sceglie l'archetipo che il pg vuole
 */
DO_RET do_archetype( CHAR_DATA *ch, char *argument )
{
	char	buf[MIL];
	int		type;		/* Numero di archetipo ricavato dall'argomento */

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono scegliere un proprio archetipo.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Archetipi possibili:\r\n" );
		show_archetype_list( ch );
		return;
	}

	if ( is_number(argument) )
	{
		type = atoi( argument );
		type--;		/* Diminuisce di uno perché la lista conta da 1 e non da 0 */
	}
	else
	{
		type = get_archetype_num( argument );
		if ( type == -1 )
		{
			send_to_char( ch, "Archetipo inesistente.\r\n" );
			return;
		}
	}

	if ( type < 0 || type >= MAX_ARCHETYPE )
	{
		send_to_char( ch, "Tipo di archetipo scelto errato.\r\n" );
		return;
	}

	/* Se il pg ha già scelto l'archetipo non lo può modificare con questo comando */
	if ( ch->pg->archetype != -1 && !IS_ADMIN(ch) )
	{
		send_to_char( ch, "Sono già caratterizzato, non posso cambiare il mio carattere in questa maniera.\r\n" );
		return;
	}

	ch->pg->archetype = type;

	strcpy( buf, get_archetype(ch) );
	strcpy( buf, get_article(buf, ARTICLE_INDETERMINATE, (ch->sex == SEX_FEMALE) ? ARTICLE_FEMININE : -1, -1) );

	ch_printf( ch, "Ora sono %s\r\n", buf );
}
