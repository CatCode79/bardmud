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


/*******************************************************************************
 * Descrizione:
 *	Codice di controllo grammaticale in italiano.
 *
 * Note:
 *	I nomi delle variabili sono in inglese come al solito, ho preferito però
 *	utilizzare nomi delle funzioni in italiano per la difficoltà nella ricerca
 *	del corrispettivo in inglese.
 ******************************************************************************/

#include <ctype.h>
#include <stdarg.h>

#include "mud.h"
#include "gramm.h"


/*
 * Controlla se il carattere passato è una vocale o meno
 * Se sì ritorna vero.
 */
bool is_vowel( char letter )
{
	switch ( letter )
	{
	  case 'a':	  case 'A':
	  case 'e':	  case 'E':
	  case 'i':	  case 'I':
	  case 'o':	  case 'O':
	  case 'u':	  case 'U':
		return TRUE;
	}

	return FALSE;
}


/*
 * Ritorna vero se il carattere è un accento tra quelli comuni in italiano.
 * Serve per il controllo sui caratteri digitabili.
 */
bool is_accent( const char accent )
{
	if ( accent == 'à'
	  || accent == 'è'
	  || accent == 'é'
	  || accent == 'ì'
	  || accent == 'ò'
	  || accent == 'ù' )
	{
		return TRUE;
	}

	return FALSE;
}


char gramm_ao( CHAR_DATA *ch )
{
	if ( ch->sex == SEX_FEMALE )
		return 'a';

	return 'o';
}


/*
 * Controlla che la parola inizi con semiconsonante (vocale seguita da altra vocale)
 */
bool semi_consonante( const char *word, char letter )
{
	if ( tolower(word[0]) == tolower(letter) )
	{
		if ( is_vowel(word[1]) && word[1] != letter )
			return TRUE;
	}

	return FALSE;
}


/*
 * Controlla se la parola inizia per esse impura
 */
bool esse_impura( const char *word )
{
	if ( tolower(word[0]) == 's')
	{
		if ( !is_vowel(word[1]) )
			return TRUE;
	}

	return FALSE;
}


/*
 * Controlla che la parola inizi con il gruppo o diagramma passato
 */
bool gruppo_diagramma( const char *word, const char first_letter, const char second_letter )
{
	if ( word[0] == first_letter )
	{
		if ( word[1] == second_letter )
			return TRUE;
	}

	return FALSE;
}


/*
 * Ritorna vero se la parola è considerata al singolare.
 */
bool is_singural( const char *word )
{
	return TRUE;
}


/*
 * Ritorna vero se la parola è considerata maschile.
 * Sono commentate le eccezioni non in uso in ambito rpg-medievale.
 */
bool is_masculine( const char *word )
{
	int length;

	length = strlen(word);
	length--;

	switch ( word[length] )
	{
	  /* Generalmente femminile con 'a' finale */
	  case 'a':
	  case 'A':
		/* Eccezioni, tipologie di derivazione greca che finiscono in "ma" */
		if ( !str_cmp(word, "diploma" ) )		return TRUE;
		if ( !str_cmp(word, "dramma"  ) )		return TRUE;
		if ( !str_cmp(word, "poema"	  ) )		return TRUE;
		if ( !str_cmp(word, "teorema" ) )		return TRUE;
		if ( !str_cmp(word, "problema") )		return TRUE;
		/* altri... */
		if ( !str_cmp(word, "duca"	  ) )		return TRUE;
		if ( !str_cmp(word, "nulla"	  ) )		return TRUE;
		if ( !str_cmp(word, "papa"	  ) )		return TRUE;
		if ( !str_cmp(word, "pigiama" ) )		return TRUE;
		if ( !str_cmp(word, "poeta"	  ) )		return TRUE;
		if ( !str_cmp(word, "profeta" ) )		return TRUE;
	/*	if ( !str_cmp(word, "vaglia"  ) )		return TRUE; */

		/* parole terminanti in "essa" sono femminili. */
		if ( !str_suffix("essa", word) )
		{
			return FALSE;
		}

		return FALSE;
		break;

	  case 'e':
	  case 'E':
		/* Se finisce con "tore" o "sore" allora maschile */
		if ( !str_suffix("ore", word) )
			return TRUE;
		/* Al contrario se finisce in "trice" e in "itrice" sono femminili. */
		if ( !str_suffix("trice", word) )
			return FALSE;

		/* Il resto dei casi è difficile da tracciare */
		return number_range(0, 1);
		break;

	  /* Generalmente femminile con 'i' finale */
	  case 'i':
	  case 'I':
		/* Eccezioni */
		if ( !str_cmp(word, "brindisi") )	return TRUE;

		return FALSE;
		break;

	  /* Generalmente maschile con 'o' finale */
	  case 'o':
	  case 'O':
		/* Eccezioni */
		if ( !str_cmp(word, "mano") )		return FALSE;
/*		if ( !str_cmp(word, "radio") )		return FALSE; */
/*		if ( !str_cmp(word, "dinamo") )		return FALSE; */
/*		if ( !str_cmp(word, "moto") )		return FALSE; */
/*		if ( !str_cmp(word, "auto") )		return FALSE; */
/*		if ( !str_cmp(word, "foto") )		return FALSE; */
/*		if ( !str_cmp(word, "virago") )		return FALSE; */
/*		if ( !str_cmp(word, "biro") )		return FALSE; */
		if ( !str_cmp(word, "eco") )		return number_range( 0,1 );	/* Viene usato sia come maschile che come femminile al singolare. */

		return TRUE;
		break;

	  /* Generalmente femminili se la parola finisce con "ta'" e "tu'" */
	  case '\'':
	  case 'à':
	  case 'ù':
		if ( (word[length] == '\'' && (word[length-1] == 'a' || word[length-1] == 'u') && word[length-1] == 't')
		  || word[length-1] == 't' )	/* questo ultimo controllo solo per accentate */
		{
			return FALSE;
		}
		break;
	}

	/* Generalmente maschile se il nome termina in consonante, di solito di origine straniera */
	if ( !is_vowel(word[length]) )
	{
		/* Eccezioni */
		if ( !str_cmp(word, "gang") )		return FALSE;
	/*	if ( !str_cmp(word, "holding") )	return FALSE; */

		return TRUE;
	}

	/* Se non trova nessuna corrispondenza utile ritorna maschile e segnala la parola */
	send_log( NULL, LOG_BUG, "is_masculine: indeterminabile il genere della parola %s.", word );
	return TRUE;
}


/*
 * Decide quale articolo utilizzare per la parola passata o
 *	la prima parola di una stringa passata.
 * Ritorna una stringa con l'articolo più la parola o la stringa passata
 * La ricerca dell'articolo viene eseguita sulla
 * (FF) aggiungere ai bits anche quello che fa ritornare solo l'articolo magari
 * Rifarla come funzione ... che accetta i vari article, poi lo farà internamente il bitvector
 */
char *get_article( char *string, int bit, ... )
{
	BITVECTOR	*bits;			/* Bitvector per gestire le tipologie di articoli */
	va_list		 args;
	int			 argument;
	static char	 article[MIL];	/* Conterrà sia l'articolo che la word passata */
	char		 arg[MIL];
	char		 word[MIL];		/* Copia qui la word e toglie i colori per essere sicuri che venga elaborata correttamente */

	if ( !VALID_STR(string) )
	{
		send_log( NULL, LOG_BUG, "get_article: string è NULL" );
		return "un errore";		/* giusto per ritornare qualcosa :P */
	}

#ifdef DEBUG_MEMORYLEAK
	createbv( bits, __FILE__, __LINE__ );
#else
	createbv( bits );
#endif
	if ( bit != -1 )
	{
		SET_BIT( bits, bit );
		va_start( args, bit );
		for ( argument = va_arg(args, int);  argument != -1;  argument = va_arg(args, int) )
			SET_BIT( bits, argument );
		va_end( args );
	}

	/* Se si vuole la stringa con un articolo e un possessivo allora lo ritorna subito */
	if ( HAS_BIT(bits, ARTICLE_POSSESSIVE) )
	{
		static char poss[MIL];

		poss[0] = '\0';
		if ( HAS_BIT(bits, ARTICLE_FEMININE) )
		{
			if ( HAS_BIT(bits, ARTICLE_PLURAL) )
				strcat( poss, HAS_BIT(bits, ARTICLE_INDETERMINATE) ? "delle tue " : "le tue " );
			else
				strcat( poss, HAS_BIT(bits, ARTICLE_INDETERMINATE) ? "una tua " : "la tua " );
		}
		else
		{
			if ( HAS_BIT(bits, ARTICLE_PLURAL) )
				strcat( poss, HAS_BIT(bits, ARTICLE_INDETERMINATE) ? "dei tuoi " : "i tuoi " );
			else
				strcat( poss, HAS_BIT(bits, ARTICLE_INDETERMINATE) ? "un tuo " : "il tuo " );
		}

		destroybv( bits );
		strcat( poss, string );
		return poss;
	}

	/* Toglie i colori alla prima parola per lavorare su di essa senza errori */
	one_argument( string, arg );
	strcpy( word, strip_color(arg) );

	article[0] = '\0';
	if ( HAS_BIT(bits, ARTICLE_FEMININE) )
	{
		if ( HAS_BIT(bits, ARTICLE_PLURAL) )
		{
			if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
				strcpy( article, "delle " );
			else
				strcpy( article, "le " );		/* raramente anche "l'" davanti a vocale ma solo in testi poetici. */
		}
		else
		{
			/* Se la prima lettera è una vocale e non è una semiconsonante allora elide
			 *	l'articolo con l'apostrofo */
			if ( is_vowel(word[0]) && !semi_consonante(word, 'i') )
			{
				if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
					strcpy( article, "un'" );
				else
					strcpy( article, "l'" );
			}
			else
			{
				if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
					strcpy( article, "una " );
				else
					strcpy( article, "la " );	/* davanti a 'h', al contrario del maschile, non c'è la forma con l'apostrofo, un problema in meno :P */
			}
		}
	}
	else
	{
		if ( esse_impura(word)					/* la parola inizia con una s e una consonante dopo */
		  || tolower(word[0]) == 'z'			/* la parola inizia con 'z' */
		  || tolower(word[0]) == 'x'			/* la parola inizia con 'x' */
		  || gruppo_diagramma(word, 'p', 'n')	/* se la parola inizia con il gruppo 'pn' */
		  || gruppo_diagramma(word, 'p', 's')	/* se la parola inizia con il gruppo 'ps' */
		  || gruppo_diagramma(word, 'g', 'n')	/* se inizia con un diagramma 'gn' o 'sc' (sc viene controllato prima dalla esse_impura) */
		  || semi_consonante(word, 'i') )		/* se inizia per vocale ma non quella semiconsontantica i/j/ */
		{
			if ( HAS_BIT(bits, ARTICLE_PLURAL) )
			{
				if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
					strcpy( article, "degli " );
				else
					strcpy( article, "gli " );
			}
			else
			{
				if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
					strcpy( article, "uno " );
				else
					strcpy( article, "lo " );
			}
		}
		else
		{
			if ( HAS_BIT(bits, ARTICLE_PLURAL) )
			{
				if ( is_vowel(word[0]) )
				{
					if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
						strcpy( article, "degli " );
					else
						strcpy( article, "gli " );
				}
				else
				{
					if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
						strcpy( article, "dei " );
					else
						strcpy( article, "i " );
				}
			}
			else
			{
				if ( is_vowel(word[0]) )
				{
					if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
						strcpy( article, "un " );
					else
						strcpy( article, "l'" );
				}
				else
				{
					if ( HAS_BIT(bits, ARTICLE_INDETERMINATE) )
						strcpy( article, "un " );
					else
						strcpy( article, "il " );	/* Ci sarebbe da effettuare un controllo sulle 'h' aspirate e non per l'apostrofo... ma la cosa non è fattibile senza, per esempio, una funzione che controlli un elenco di sostantivi scelti per essere apostrofati, poichè vi possono essere nomi fantasy del contesto background del mud che richiedono questo stile... fate vobis :P */
				}
			}
		}
	}

	destroybv( bits );
	strcat( article, string );
	return ( article );
}


/*
 * Shove either "a " or "an " onto the beginning of a string
 */
char *aoran( const char *str )
{
	static char temp[MSL];

	if ( !str )
	{
		send_log( NULL, LOG_BUG, "aoran: NULL str" );
		return "";
	}

	if ( is_vowel(str[0]) || ( strlen(str) > 1 && tolower(str[0]) == 'y' && !is_vowel(str[1])) )
		strcpy( temp, "an " );
	else
		strcpy( temp, "a " );

	strcat( temp, str );
	return temp;
}


/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name isn't always appropriate
 * since it contains keywords that may not look proper.
 */
char *myobj( OBJ_DATA *obj )
{
	if ( !str_prefix("a ",	  obj->short_descr) )	return obj->short_descr + 2;
	if ( !str_prefix("an ",	  obj->short_descr) )	return obj->short_descr + 3;
	if ( !str_prefix("the ",  obj->short_descr) )	return obj->short_descr + 4;
	if ( !str_prefix("some ", obj->short_descr) )	return obj->short_descr + 5;

	return obj->short_descr;
}


/* (FF) Ritornano sempre parole o lettere minuscole, dovrei controllare che la parola
 *	non sia tutta maiuscola ed adeguarmi allo stile.
 * Forma il numero di un sostantivo, se si passa TRUE singolare.
 */
char *get_gramm_number( const char *word, bool singular )
{
	int		length;
	static char	temp[MSL];

	strcpy( temp, word );

	length = strlen(word);
	length--;

	/* Se si vuole la parola in singolare (FF) non operativa per ora. */
	if ( singular )
	{
		switch ( word[length] )
		{
		  case 'a':
		  case 'e':
		  case 'i':
		  case 'o':
		  return temp;
		  break;
		}
	}
	else	/* Se si vuole la parola in plurale */
	{
		switch ( word[length] )
		{
		  case 'a':
		  case 'A':
			/* Eccezioni */
			if ( !str_cmp(word, "ala" ) )	return "ali";		/* ale e arme è di uso arcaico */
			if ( !str_cmp(word, "arma") )	return "armi";

			/* Nomi in 'a' che rimangono inalterati. */
			/* (RR) */

			/* Nomi che conservano al plurale le consonanti velari /k/ e /g/ */
			if ( !str_suffix("ca", word) || !str_suffix("ga", word) )
			{
				/* Eccezione giusto per essere precisi: Belga, in plurale maschile perde il suono velare (Belgi) in plurale femminile è normale (Belghe) */

				temp[length] = '\0';
				if ( is_masculine(word) )
					strcat(temp, "hi");
				else
					strcat(temp, "he");

				return temp;
			}

			/* Nomi in "cia" e "gia" formano plurale con "cie" e "gie".
			 * Il problema è che con la i atona invece conservano la 'i' se precedute da vocale,
			 *	la perdono se precedute da consonante. */
			if ( !str_suffix("cia", word) || !str_suffix("gia", word) )
			{
				if ( number_range(0, 1) == 0 )
					temp[length] = 'e';
				else
				{
					if ( is_vowel(temp[length-3]) )
						temp[length] = 'e';
					else
					{
						temp[length-1] = 'e';
						temp[length] = '\0';
					}
				}
				return temp;
			}

			/* Altrimenti si invia la parola formata con la regola base: */
			if ( is_masculine(word) )
				temp[length] = 'i';
			else
				temp[length] = 'e';

			return temp;
			break;

		  case 'o':
		  case 'O':
			/* Eccezioni: */
			if ( str_cmp(word, "uomo") )	return "uomini";

			/* Se la parola finisce con "co" o "go" in linea di massima, se sono piani, conservano
			 *	leconsonanti velari /k/ e /g/ ed escono "chi" e "ghi", se sono sdrucciole invece
			 *	le perdono, formando parole che finiscono con "ci" e "gi" */
			if ( !str_suffix("co", word) || !str_suffix("go", word) )
			{
				/* Gestisce i nomi piani */
				if ( !str_cmp(word, "nemico") )		return "nemici";
				if ( !str_cmp(word, "amico") )		return "amici";
				if ( !str_cmp(word, "greco") )		return "greci";
				if ( !str_cmp(word, "porco") )		return "porci";

				/* Ci sarebbero da gestire le doppie forme. almeno indicando quella più usata */

				temp[length] = '\0';
				strcat( temp, "hi" );
				return temp;
			}

			/* I nomi in "io" con 'i' tonica formano plurale in "ii", con i atona escono 'i' */
			if ( !str_suffix("io", word) )
			{
				/* Eccezioni: */
				if ( !str_cmp(word, "tempio") )		return "templi";

				if ( number_range(0,1) == 0 )
					temp[length] = 'i';
				else
					temp[length] = '\0';

				return temp;
			}

			/* Eccezioni di alcuni nomi in 'o' che prendono la 'a' al plurale. */
			if ( !str_cmp(word, "centinaio") )	return "centinaia";
			if ( !str_cmp(word, "migliaio") )	return "migliaia";
			if ( !str_cmp(word, "miglio") )		return "miglia";
			if ( !str_cmp(word, "paio") )		return "paia";
			if ( !str_cmp(word, "uovo") )		return "uova";
			if ( !str_cmp(word, "riso") )		return "risa";

			/* Altrimenti si invia la parola formata con la regola base: */
			temp[length] = 'i';
			return temp;
			break;

		  case 'e':
		  case 'E':
			/* I nomi in "ie" sono invariati, sono queste eccezioni cambiano: */
			if ( !str_suffix("ie", word) )
			{
				if ( !str_cmp(word, "moglie") )		return "mogli";
				if ( !str_cmp(word, "superficie") )	return "superfici";	/* Esiste anche la forma invariata, meno comune */
				if ( !str_cmp(word, "l'effige") )	return "effigi";	/* Esiste anche la forma invariata, meno comune */

				return temp;
			}

			/* Eccezioni: */
			if ( !str_cmp(word, "bue") )			return "buoi";

			/* Altrimenti si invia la parola formata con la regola base: */
			temp[length] = 'i';
			return temp;
			break;
		}
	}

	send_log( NULL, LOG_BUG, "get_gramm_number: non trovata parola o regola per il %s di %s.",
		(singular)  ?  "singolare"  :  "plurale", word );
	return temp;
}


/*
 * (FF) anche qui devo mantenere le case della parola originale.
 * Forma il genere di un sostantivo, se si passa TRUE è il maschile.
 */
char *get_gramm_gender( const char *word, bool masculine )
{
	static char temp[MSL];

	strcpy( temp, word );

	/* */
	if ( masculine )
	{

	}
	else
	{

	}
	return temp;
}


#if 0
char *syllabify( char *text, int column )
{
	/* Se la prima lettera della parola è seguita da una consonante allora è sillaba */
	if (  )

	/* Le consonanti semplice seguite da vocale formano sillaba */
	if ( )

	/* */
}
#endif
