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


/* Modulo riguardante il meteo */


#include "mud.h"
#include "build.h"
#include "db.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"
#include "nanny.h"
#include "room.h"


/*
 * File con le informazioni meteo
 */
#define METEO_FILE	TABLES_DIR "meteo.dat"


/*
 * Definizione del numero massimo di impostazioni di climate
 */
#define MAX_CLIMATE 5


/*
 * Variabile esterna delle informazioni meteo di base
 */
METEO_DATA		meteo;


/*
 * Costanti meteo
 */
char *const temp_settings[] =
{
	"freddo",
	"fresco",
	"temperato",
	"caldo",
	"torrido"
};

char *const precip_settings[] =
{
	"arido",
	"secco",
	"mite",
	"umido",
	"piovoso"
};

char *const wind_settings[] =
{
	"sereno",
	"bonaccia",
	"lieve brezza",
	"ventoso",
	"tempestoso"
};


char *const preciptemp_msg[6][6] =
{
	/* precip = 0 */
	{
		"Gelide temperature avvolgono ogni luogo",
		"Fa un po' freddo",
		"Il clima è decisamente arido",
		"Inizia a calare un dolce tepore",
		"Un caldo secco avvolge ogni luogo",
		"Il sole rovente sembra quasi far bruciare ogni luogo"
	 },
	 /* precip = 1 */
	 {
	 	"Violente tempeste trascinano cupe nubi",
	 	"La pioggia inizia lentamente a trasformarsi in ghiaccio",
	 	"Piogge occasionali bagnano il terreno",
	 	"Una lieve pioggerella cade da piccole nubi",
 	 	"Fa veramente caldo e il cielo è totalmente coperto",
	 	"Una opprimente umidità intensifica il caldo"
	 },
	 /* precip = 2 */
	 {
	 	"Una tempesta di neve ricopre ogni luogo",
	 	"Una debole nevicata ricopre ogni luogo",
	 	"Neve lieve cade dolcemente dal cielo",
	 	"Una lieve pioggerella rovina quella che sarebbe stata una bella giornata",
	 	"Alcune gocce di pioggia cadono sulle calde terre",
	 	"Una sottile pioggia cade in quest'aria soffocante"
	 },
	 /* precip = 3 */
	 {
	 	"La neve copre la gelida terra",
	 	"Soffice neve cade in ogni luogo",
	 	"Un forte acquazzone inumidisce quest'aria secca",
	 	"Una gradevole pioggia cade dal cielo",
	 	"Lo scirocco aumenta con la pioggia",
	 	"Un rigenerante acquazzone allievia il caldo opprimente"
	 },
	 /* precip = 4 */
	 {
	 	"L'aria gelida porta con se un'intensa grandinata",
	 	"La neve cade velocemente, ricoprendo la terra gelida",
	 	"La pioggia cade incessantemente in questa giornata secca",
	 	"La pioggia colpisce ritmicamente il suolo",
	 	"Una pioggia calda tamburella lievemente sul terreno",
	 	"Un acquazzone tropicale si abbatte violentemente sulle terre"
	 },
	 /* precip = 5 */
	 {
	 	"Un acquazzone di pioggia gelida tramuta la terra in ghiaccio",
	 	"Una bufera di neve ammanta ogni cosa di bianco",
	 	"Una pioggia torrenziale cade in questa fresca giornata",
	 	"Un tremendo acquazzone oscura questa mite giornata",
	 	"Pioggia scrosciante cade dal cielo",
	 	"Un diluvio inzuppa la terra bollente"
	 }
};

char *const windtemp_msg[6][6] =
{
	/* wind = 0 */
	{
		"L'aria è gelida e non si muove foglia",
		"La temperatura si abbassa notevolmente su tutta l'area",
		"L'aria è secca e non si avverte alcuna brezza",
		"L'aria e calda e non si muove foglia",
		"Nessuna fresca brezza allevia questa calda giornata",
		"Questo caldo opprimente ti soffoca"
	},
	/* wind = 1 */
	{
		"Una lieve brezza rende questa gelida giornata un po' più calda",
		"Il cambiamento d'aria intensifica il freddo",
		"Un po' di vento rende la giornata fresca",
		"E' una giornata mite, con una lieve brezza",
		"Fa veramente caldo e l'aria si muove leggermente",
		"Una debole brezza scuote quest'aria torrida"
	},
	/* wind = 2 */
	{
		"Una brezza rende l'aria gelida",
		"Una brezza porta con sè aria fresca",
		"Una lieve brezza rende l'aria fresca",
		"E' una giornata mite, con una piacevole brezza",
		"Correnti d'aria calda sferzano il luogo",
		"Una brezza rigenera un po' quest'aria soffocante"
	},
	/* wind = 3 */
	{
		"Violente raffiche di vento incrementano il gelo",
		"L'aria fresca è agitata da raffiche di vento",
		"Il vento soffia da nord, rinfrescando il luogo",
		"Folate di vento rinnovano l'aria mite",
		"Folate di vento secco accentuano la calura",
		"Il vento cerca di smorzare quest'aria torrida"
	},
	/* wind = 4 */
	{
		"L'aria gelida turbina in folate di vento",
		"Un forte e gelido vento soffia da nord",
		"Un forte vento rende l'aria fresca pungente",
		"E' una bella giornata, con lievi folate di vento",
		"Calde folate di vento soffiano su tutta l'area",
		"Bufere di vento smorzano l'aria torrida"
	},
	/* wind = 5 */
	{
		"Un vento fortissimo vento gelido ti scuote le ossa",
		"Raffiche di vento ululante sferzano l'aria fredda",
		"Un vento rabbioso sferza freneticamente l'aria",
		"Venti feroci si muovono velocemente urlando attraverso l'aria tiepida",
		"Fortissimi venti sferzano l'aria calda",
		"Venti monsonici lacerano l'aria torrida"
	}
};

char *const precip_msg[3] =
{
	"non c'è una nuvola in cielo",
	"candide nuovole si muovono nel cielo",
	"grige spesse nubi coprono il sole"
};

char *const wind_msg[6] =
{
	"non si avverte un alito di vento",
	"una lieve brezza scombina l'aria",
	"una brezza si stende su tutta la zona",
	"folate di vento sconvolgono la zona",
	"soffiano rabbiose raffiche di vento",
	"venti ululanti sferzano l'aria con frenesia"
};


/*
 * Carica le informazioni meteo di default
 */
void load_meteo_default( void )
{
	meteo.weath_unit		= 10;
	meteo.rand_factor		=  2;
	meteo.climate_factor	=  1;
	meteo.neigh_factor		=  3;
	meteo.max_vector		= meteo.weath_unit * 3;
}


/*
 * Load weather data from appropriate file in system dir
 */
void fread_meteo( MUD_FILE *fp )
{
	for ( ; ; )
	{
		char *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_meteo: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Climate"	) )		meteo.climate_factor =	fread_number( fp );
		else if ( !str_cmp(word, "End"		) )		break;
		else if ( !str_cmp(word, "Random"	) )		meteo.rand_factor =		fread_number( fp );
		else if ( !str_cmp(word, "Neighbor"	) )		meteo.neigh_factor =	fread_number( fp );
		else if ( !str_cmp(word, "Unit"		) )		meteo.weath_unit =		fread_number( fp );
		else if ( !str_cmp(word, "MaxVector") )		meteo.max_vector =		fread_number( fp );
		else
			send_log( fp, LOG_FREAD, "fread_meteo: etichetta non trovata: %s", word );
	}

	/*
	 * Controlla i valori letti (FF)
	 */
	if ( meteo.neigh_factor <= 0 )
	{
		send_log( NULL, LOG_FREAD, "fread_meteo: fattore neighbor errato: %d", meteo.neigh_factor );
		exit( EXIT_FAILURE );
	}
}


/*
 * Write data for global weather parameters
 */
void save_meteo( void )
{
	MUD_FILE	*fp;

	fp = mud_fopen( "", METEO_FILE, "w", TRUE );
	if ( !fp )
		return;

	fprintf( fp->file, "#METEO" );
	fprintf( fp->file, "Random       %d\n",	meteo.rand_factor		);
	fprintf( fp->file, "Climate      %d\n",	meteo.climate_factor	);
	fprintf( fp->file, "Neighbor     %d\n",	meteo.neigh_factor		);
	fprintf( fp->file, "Unit         %d\n",	meteo.weath_unit		);
	fprintf( fp->file, "MaxVector    %d\n",	meteo.max_vector		);
	fprintf( fp->file, "End\n\n" );
	fprintf( fp->file, "#END\n" );

	MUD_FCLOSE( fp );
}


/*
 * Initialize the weather for all the areas
 */
void init_area_weather( void )
{
	AREA_DATA	  *pArea;
	NEIGHBOR_DATA *neigh;
	NEIGHBOR_DATA *next_neigh;

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		int	cf;

		/* init temp and temp vector */
		cf = pArea->weather->climate_temp - 2;
		pArea->weather->temp		= number_range(-meteo.weath_unit, meteo.weath_unit) + cf * number_range( 0, meteo.weath_unit );
		pArea->weather->temp_vector = cf + number_range( -meteo.rand_factor, meteo.rand_factor );

		/* init precip and precip vector */
		cf = pArea->weather->climate_precip - 2;
		pArea->weather->precip		  = number_range(-meteo.weath_unit, meteo.weath_unit) + cf * number_range( 0, meteo.weath_unit );
		pArea->weather->precip_vector = cf + number_range( -meteo.rand_factor, meteo.rand_factor );

		/* init wind and wind vector */
		cf = pArea->weather->climate_wind - 2;
		pArea->weather->wind		= number_range(-meteo.weath_unit, meteo.weath_unit) + cf * number_range( 0, meteo.weath_unit );
		pArea->weather->wind_vector = cf + number_range( -meteo.rand_factor, meteo.rand_factor );

		/* check connections between neighbors */
		for ( neigh = pArea->weather->first_neighbor;  neigh;  neigh = next_neigh )
		{
			AREA_DATA	  *tarea;
			NEIGHBOR_DATA *tneigh;
			char		   filename[MIL];

			/* get the address if needed */
			if ( !neigh->address )
				neigh->address = get_area( neigh->name );

			/* area does not exist */
			if ( !neigh->address )
			{
				tneigh = neigh;
				next_neigh = tneigh->next;
				UNLINK( tneigh, pArea->weather->first_neighbor, pArea->weather->last_neighbor, next, prev );
				DISPOSE( tneigh->name );
				DISPOSE( tneigh );
				sprintf( filename, "%s%s", AREA_DIR, pArea->filename );
				fold_area( pArea, filename, FALSE );
				continue;
			}

			/* make sure neighbors both point to each other */
			tarea = neigh->address;
			for ( tneigh = tarea->weather->first_neighbor;  tneigh;  tneigh = tneigh->next )
			{
				if ( !strcmp(pArea->name, tneigh->name) )
					break;
			}

			if ( !tneigh )
			{
				CREATE ( tneigh, NEIGHBOR_DATA, 1 );
				tneigh->name = str_dup( pArea->name );
				LINK( tneigh, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );
				sprintf( filename, "%s%s", AREA_DIR, pArea->filename );
				fold_area( tarea, filename, FALSE );
			}
			tneigh->address = pArea;
			next_neigh = neigh->next;
		} /* chiude il for */
	} /* chiude il for */
}


/*
 * Carica tutte le informazioni meteo all'avvio
 */
void load_meteo( void )
{
	load_meteo_default( );
	fread_section( METEO_FILE, "METEO", fread_meteo, TRUE );
	init_area_weather( );
}


/*
 * Function to update weather vectors according to climate
 * settings, random effects, and neighboring areas.
 */
void adjust_vectors( WEATHER_DATA *weather )
{
	NEIGHBOR_DATA  *neigh;
	double			dT;
	double			dP;
	double			dW;

	if ( !weather )
	{
		send_log( NULL, LOG_BUG, "adjust_vectors: la variabile weather è NULL." );
		return;
	}

	dT = 0;
	dP = 0;
	dW = 0;

	/* Add in random effects */
	dT += number_range( -meteo.rand_factor, meteo.rand_factor );
	dP += number_range( -meteo.rand_factor, meteo.rand_factor );
	dW += number_range( -meteo.rand_factor, meteo.rand_factor );

	/* Add in climate effects*/
	dT += meteo.climate_factor * ( ((weather->climate_temp   - 2)*meteo.weath_unit) - (weather->temp  ) ) / meteo.weath_unit;
	dP += meteo.climate_factor * ( ((weather->climate_precip - 2)*meteo.weath_unit) - (weather->precip) ) / meteo.weath_unit;
	dW += meteo.climate_factor * ( ((weather->climate_wind   - 2)*meteo.weath_unit) - (weather->wind  ) ) / meteo.weath_unit;

	/* Add in effects from neighboring areas */
	for ( neigh = weather->first_neighbor;  neigh;  neigh = neigh->next )
	{
		/* see if we have the area cache'd already */
		if ( !neigh->address )
		{
			/* try and find address for area */
			neigh->address = get_area( neigh->name );

			/* if couldn't find area ditch the neigh */
			if ( !neigh->address )
			{
				NEIGHBOR_DATA *temp;
				send_log( NULL, LOG_BUG, "adjust_weather: nome area non valido." );
				temp = neigh->prev;
				UNLINK( neigh, weather->first_neighbor, weather->last_neighbor, next, prev );
				DISPOSE( neigh->name );
				DISPOSE( neigh );
				neigh = temp;
				continue;
			}
		}

		dT += ( neigh->address->weather->temp   - weather->temp   ) / meteo.neigh_factor;
		dP += ( neigh->address->weather->precip - weather->precip ) / meteo.neigh_factor;
		dW += ( neigh->address->weather->wind   - weather->wind   ) / meteo.neigh_factor;
	}

	/* now apply the effects to the vectors */
	weather->temp_vector   += (int)dT;
	weather->precip_vector += (int)dP;
	weather->wind_vector   += (int)dW;

	/* Make sure they are within the right range */
	weather->temp_vector   = URANGE( -meteo.max_vector, weather->temp_vector,	meteo.max_vector );
	weather->precip_vector = URANGE( -meteo.max_vector, weather->precip_vector, meteo.max_vector );
	weather->wind_vector   = URANGE( -meteo.max_vector, weather->wind_vector,	meteo.max_vector );
}


/*
 * Controlla se dalla stanza NON si possa vedere il tempo atmosferico
 * (FF) usare la tabella table_sector per questo
 */
bool no_weather_sect( const int sect )
{
	if ( sect < 0 || sect >= MAX_SECTOR )
	{
		send_log( NULL, LOG_BUG, "no_weather_sect: tipo di settore passato errato: %d", sect );
		return TRUE;	/* Ritorna true evitando di inviare gli echo del tempo atmosferico */
	}

	if ( sect == SECTOR_INSIDE		)	return TRUE;
	if ( sect == SECTOR_UNDERWATER	)	return TRUE;
	if ( sect == SECTOR_OCEANFLOOR	)	return TRUE;
	if ( sect == SECTOR_UNDERGROUND	)	return TRUE;

	return FALSE;
}


/*
 * Get weather echo messages according to area weather...
 * stores echo message in weath_data.... must be called before
 * the vectors are adjusted
 */
void send_weather_echo( AREA_DATA *pArea )
{
	DESCRIPTOR_DATA *d;
	WEATHER_DATA	*weath;
	char			 echo[MIL];
	int				 color;

	int		n;
            	  
	int		temp;
	int		precip;
	int		wind;
            
	int		dT;
	int		dP;
	int		dW;
            
	int		tindex;
	int		pindex;
	int		windex;

	if ( !pArea )
	{
		send_log( NULL, LOG_BUG, "send_weather_echo: pArea è NULL" );
		return;
	}

	weath = pArea->weather;

	/* set echo to be nothing */
	echo[0] = '\0';
	color	= AT_GREY;

	/* get the random number */
	n = number_bits(2);

	/* variables for convenience */
	temp   = weath->temp;
	precip = weath->precip;
	wind   = weath->wind;

	dT = weath->temp_vector;
	dP = weath->precip_vector;
	dW = weath->wind_vector;

	tindex = ( temp   + meteo.weath_unit*3 - 1 ) / meteo.weath_unit;
	pindex = ( precip + meteo.weath_unit*3 - 1 ) / meteo.weath_unit;
	windex = ( wind   + meteo.weath_unit*3 - 1 ) / meteo.weath_unit;

	/* get the echo string... mainly based on precip */
	switch ( pindex )
	{
	  default:
		send_log( NULL, LOG_BUG, "echo_weather: indice di precipitazione non valido" );
		weath->precip = 0;
		break;

	  case 0:
		if ( precip - dP > meteo.weath_unit*-2 )
		{
			char *echo_strings[4] =
			{
				"Le nubi cominciano a disperdersi lentamente.\r\n",
				"Le nuvole si dissolvono adagio, allargando il cielo.\r\n",
				"Il cielo si fa strada attraverso il varco delle nuvole.\r\n",
				"Le nubi evaporano fino a dipanarsi completamente.\r\n"
			};
			strcpy( echo, echo_strings[n] );
			color = AT_WHITE;
			weath->sky = SKY_CLOUDLESS;
		}
		break;

	  case 1:
		if ( precip - dP <= meteo.weath_unit*-2 )
		{
			char *echo_strings[4] =
			{
				"Qualche nube offusca il cielo.\r\n",
				"Il cielo è coperto di nuvole.\r\n",
				"C'è foschia.. Il cielo è coperto da piccole nubi luminose.\r\n",
				"Grosse nuvole grigie espanse s'ammassano coprendo il cielo.\r\n"
			};
			strcpy( echo, echo_strings[n] );
			color = AT_GREY;
			weath->sky = SKY_CLOUDY;
		}
		break;

	  case 2:
		if ( precip - dP > 0 )
		{
			if ( tindex > 1 )
			{
				char *echo_strings[4] =
				{
					"La pioggia sta diminuendo.\r\n",
					"Smette di piovere.\r\n",
					"Il temporale si affievolisce.\r\n",
					"La pioggia fitta s'è arrestata: sono solo gocce, ora.\r\n"
				};
				strcpy( echo, echo_strings[n] );
				color = AT_CYAN;
				weath->sky = SKY_CLOUDY;
			}
			else
			{
				char *echo_strings[4] =
				{
					"La neve cade più lentamente.\r\n",
					"Ha smesso di nevicare.\r\n",
					"La neve si dirada pian piano..\r\n",
					"I fiocchi di neve stanno lentamente scomparendo.\r\n"
				};
				strcpy( echo, echo_strings[n] );
				color = AT_WHITE;
				weath->sky = SKY_CLOUDY;
			}
		}
		break;

	  case 3:
		if ( precip - dP <= 0 )
		{
			if ( tindex > 1 )
			{
				char *echo_strings[4] =
				{
					"Lievi gocce di pioggia cadono lente.\r\n",
					"Scroscia una pioggia battente.\r\n",
					"Gronda una leggera pioggerellina.\r\n",
					"Pioviggina.\r\n"
				};

				strcpy( echo, echo_strings[n] );
				color = AT_CYAN;
				weath->sky = SKY_RAINING;
			}
			else
			{
				char *echo_strings[4] =
				{
					"Comincia a nevicare.\r\n",
					"Cade un'abbondante neve spessa.\r\n",
					"Leggeri cristalli di neve scendono dalle nubi.\r\n",
					"Soffici fiocchi di neve turbinano nell'aria.\r\n"
				};
				strcpy( echo, echo_strings[n] );
				color = AT_WHITE;
				weath->sky = SKY_CLOUDY;
			}
		}
		else if ( tindex < 2 && temp - dT > -meteo.weath_unit )
		{
			char *echo_strings[4] =
			{
				"La temperatura bassa veste la pioggia di candida neve.\r\n",
				"Le gocce di pioggia si ghiacciano durante il loro scrosciare.\r\n",
				"La pioggia battente gela regalando grossi fiocchi di neve.\r\n",
				"La neve perde peso durante la sua danza nell'aria, divenendo pioggia sottile.\r\n"
			};
			strcpy( echo, echo_strings[n] );
			color = AT_WHITE;
			weath->sky = SKY_SNOW;
		}
		else if ( tindex > 1 && temp - dT <= -meteo.weath_unit )
		{
			char *echo_strings[4] =
			{
				"La temperatura sale e tramuta la soffice neve in pioggia.\r\n",
				"La neve si scioglie lentamente, lasciando il posto ad una leggera pioggia.\r\n",
				"I fiocchi di neve fluttuano rapidi, ne prende il posto una fitta pioggia fredda.\r\n",
				"L'aria si scalda: la neve si dirada e comincia a piovere.\r\n"
			};
			strcpy( echo, echo_strings[n] );
			color = AT_CYAN;
			weath->sky = SKY_RAINING;
		}
		break;

	  case 4:
		if ( precip - dP > meteo.weath_unit*2 )
		{
			if ( tindex > 1 )
			{
				char *echo_strings[4] =
				{
					"Il cielo si rischiara, i fulmini sono cessati.\r\n",
					"I lampi si attenuano man mano, risolvendo la loro corsa luminosa.\r\n",
					"Il rullo dei tuoni si placa lentamente.. Il tempo si ristabilisce.\r\n",
					"La tempesta si sta affievolendo, i lampi scoppiano più radi, allontanandosi.\r\n"
				};
				strcpy( echo, echo_strings[n] );
				color = AT_GREY;
				weath->sky = SKY_LIGHTNING;
			}
		}
		else if ( tindex < 2 && temp - dT > -meteo.weath_unit )
		{
			char *echo_strings[4] =
			{
				"La fredda pioggia muta la sua caduta diventando neve.\r\n",
				"Pesanti gocce di pioggia fitta terminano la loro discesa gelando.. Nevicherà a breve.\r\n",
				"Nevica. E il candore dei fiocchi lenti è accompagnato da una leggera pioggia.\r\n",
				"La pioggia battente comincia a ghiacciare.\r\n"
			};

			strcpy( echo, echo_strings[n] );
			color = AT_WHITE;
			weath->sky = SKY_SNOW;
		}
		else if ( tindex > 1 && temp - dT <= -meteo.weath_unit )
		{
			char *echo_strings[4] =
			{
				"La neve si scioglie cadendo e una gelida pioggia l'accompagna.\r\n",
				"Acqua ghiacciata piove dal cielo: è neve disciolta.\r\n",
				"Diluvia.. Un'improvvisa pioggia gelata mista a neve.\r\n",
				"La neve cede rapidamente il posto ad una pesante pioggia fredda.\r\n"
			};

			strcpy( echo, echo_strings[n] );
			color = AT_CYAN;
			weath->sky = SKY_RAINING;
		}
		break;

	  case 5:
		if ( precip - dP <= meteo.weath_unit*2 )
		{
			if ( tindex > 1 )
			{
				char *echo_strings[4] =
				{
					"Un lampo balena improvviso, rischiarando il cielo per un attimo.\r\n",
					"Folgori di lampi s'incrociano nel cielo.\r\n",
					"L'improvviso bagliore di un fulmine spacca in due il cielo.\r\n",
					"Il cielo ripete del rombo dei tuoni, un lampo l'attraversa, rapido e accecante.\r\n"
				};

				strcpy( echo, echo_strings[n] );
				color = AT_YELLOW;
				weath->sky = SKY_LIGHTNING;
			}
		}
		else if ( tindex > 1 && temp - dT <= -meteo.weath_unit )
		{
			char *echo_strings[4] =
			{
				"Il cielo grida di tuoni possenti, mentre la neve si trasforma in pioggia violenta.\r\n",
				"Il fragore dei tuoni esplode nel cielo, una pioggia irruente prende il posto della candida neve.\r\n",
				"La pioggia battente gela toccando il suolo, sfrecciando rapida tra le folgori dei lampi.\r\n",
				"La neve cadente si scioglie rapidamente, i tuoni ruggiscono dall'alto.\r\n"
			};
			strcpy( echo, echo_strings[n] );
			color = AT_WHITE;
			weath->sky = SKY_LIGHTNING;
		}
		else if ( tindex < 2 && temp - dT > -meteo.weath_unit )
		{
			char *echo_strings[4] =
			{
				"I lampi sopiscono lasciando il posto ad una bufera di neve accecante.\r\n",
				"La tempesta di neve turbina in un vortice abbagliante.. I lampi sono cessati.\r\n",
				"Nel boato di un tuono potente la pioggia si dilegua lasciando il posto ad una bufera di neve.\r\n",
				"Il tuono termina il suo grido e tace, la gelida pioggia ghiaccia scendendo, è neve pesante adesso.\r\n"
			};
			strcpy( echo, echo_strings[n] );
			color = AT_CYAN;
			weath->sky = SKY_SNOW;
		}
		break;
	}

	/* display the echo strings to the appropriate players */
	for ( d = first_descriptor;  d;  d = d->next )
	{
		if ( d->connected != CON_PLAYING )						continue;
		if ( !is_outside(d->character) )						continue;
		if ( !is_awake(d->character) )							continue;
		if ( !d->character->in_room )							continue;
		if ( no_weather_sect(d->character->in_room->sector) )	continue;
		if ( d->character->in_room->area != pArea )				continue;

		if ( VALID_STR(echo) )
		{
			set_char_color( color, d->character );
			ch_printf( d->character, "%s&w", echo );
		}
	}
}


/*
 * Function updates weather for each area
 */
void update_weather( void )
{
	AREA_DATA	*pArea;
	int			 limit;

	limit = meteo.weath_unit * 3;

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		/* Apply vectors to fields */
		pArea->weather->temp   += pArea->weather->temp_vector;
		pArea->weather->precip += pArea->weather->precip_vector;
		pArea->weather->wind   += pArea->weather->wind_vector;

		/* Make sure they are within the proper range */
		pArea->weather->temp   = URANGE( -limit, pArea->weather->temp,	 limit );
		pArea->weather->precip = URANGE( -limit, pArea->weather->precip, limit );
		pArea->weather->wind   = URANGE( -limit, pArea->weather->wind,	 limit );

		/* get an appropriate echo for the area */
		send_weather_echo( pArea );
	}

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
		adjust_vectors( pArea->weather );
}


/*
 * Incantesimo di Control Weather
 */
SPELL_RET spell_control_weather( int sn, int level, CHAR_DATA *ch, void *vo )
{
	SKILL_DATA	 *skill = get_skilltype( sn );
	WEATHER_DATA *weath;
	int			  change;

	weath = ch->in_room->area->weather;
	change = number_range( -meteo.rand_factor, meteo.rand_factor )
				+ ( (get_level(ch)*1.5) / (meteo.max_vector*2) );	/* (TT) qui andrà bene level al posto della get_level? se sì moltiplicare poi per 3*/

	if		( is_name(target_name, "caldo warmer"	) )	weath->temp_vector	 += change;
	else if	( is_name(target_name, "freddo colder"	) )	weath->temp_vector	 -= change;
	else if	( is_name(target_name, "umido wetter"	) )	weath->precip_vector += change;
	else if	( is_name(target_name, "secco drier"	) )	weath->precip_vector -= change;
	else if	( is_name(target_name, "ventoso windier") )	weath->wind_vector	 += change;
	else if	( is_name(target_name, "sereno calmer"	) )	weath->wind_vector	 -= change;
	else
	{
		send_to_char( ch, "Rendere il tempo più caldo, freddo, umido, secco, ventoso o sereno?\r\n" );
		return rSPELL_FAILED;
	}

	weath->temp_vector	 = URANGE( -meteo.max_vector,	weath->temp_vector,		meteo.max_vector );
	weath->precip_vector = URANGE( -meteo.max_vector,	weath->precip_vector,	meteo.max_vector );
	weath->wind_vector	 = URANGE( -meteo.max_vector,	weath->wind_vector,		meteo.max_vector );

	successful_casting( skill, ch, NULL, NULL );
	return rNONE;
}


/*
 * Command to display the weather status of all the areas
 */
DO_RET do_showweather( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	char	   arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_showweather: char è NULL" );
		return;
	}

	argument = one_argument( argument, arg );

	set_char_color( AT_BLUE, ch );
	ch_printf( ch, "%-40s%-8s %-8s %-8s\r\n", "Area Name:", "Temp:", "Precip:", "Wind:" );

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		if ( !VALID_STR(arg) || nifty_is_name_prefix(arg, pArea->name) )
		{
			set_char_color( AT_BLUE, ch	 );		ch_printf	( ch, "%-40s", pArea->name );
			set_char_color( AT_WHITE, ch );		ch_printf	( ch, "%3d", pArea->weather->temp );
			set_char_color( AT_BLUE, ch	 );		send_to_char( ch, "(" );
			set_char_color( AT_LCYAN, ch );		ch_printf	( ch, "%3d", pArea->weather->temp_vector );
			set_char_color( AT_BLUE,ch	 );		send_to_char( ch, ") " );
			set_char_color( AT_WHITE,ch	 );		ch_printf	( ch, "%3d", pArea->weather->precip );
			set_char_color( AT_BLUE, ch	 );		send_to_char( ch, "(" );
			set_char_color( AT_LCYAN, ch );		ch_printf	( ch, "%3d", pArea->weather->precip_vector );
			set_char_color( AT_BLUE, ch	 );		send_to_char( ch, ") " );
			set_char_color( AT_WHITE, ch );		ch_printf	( ch, "%3d", pArea->weather->wind );
			set_char_color( AT_BLUE, ch  );		send_to_char( ch, "(" );
			set_char_color( AT_LCYAN, ch );		ch_printf	( ch, "%3d", pArea->weather->wind_vector );
			set_char_color( AT_BLUE, ch  );		send_to_char( ch, ")\r\n" );
		}
	} /* chiude il for */
}


/*
 * Command to control global weather variables and to reset weather
 */
DO_RET do_setmeteo(CHAR_DATA *ch, char *argument)
{
	char arg[MIL];

	set_char_color(AT_BLUE, ch);

	argument = one_argument(argument, arg);

	if ( !VALID_STR(arg) )
	{
		ch_printf( ch, "%-15s%-6s\r\n",	"Parameters:", "Value:" );
		ch_printf( ch, "%-15s%-6d\r\n",	"random", meteo.rand_factor );
		ch_printf( ch, "%-15s%-6d\r\n",	"climate", meteo.climate_factor );
		ch_printf( ch, "%-15s%-6d\r\n",	"neighbor", meteo.neigh_factor );
		ch_printf( ch, "%-15s%-6d\r\n",	"unit", meteo.weath_unit );
		ch_printf( ch, "%-15s%-6d\r\n",	"maxvector", meteo.max_vector );

		ch_printf( ch, "\r\nResulting values:\r\n");
		ch_printf( ch, "Weather variables range from %d to %d.\r\n", meteo.weath_unit*-3, meteo.weath_unit*3 );
		ch_printf( ch, "Weather vectors range from %d to %d.\r\n", -meteo.max_vector, meteo.max_vector );
		ch_printf( ch, "The maximum a vector can change in one update is %d.\r\n",
			meteo.rand_factor + meteo.climate_factor*2 + (meteo.weath_unit*6/meteo.neigh_factor) );
	}
	else if ( !str_cmp(arg, "casuale") || !str_cmp(arg, "random") )
	{
		if ( !is_number(argument) )
		{
			send_to_char( ch, "Set maximum random change in vectors to what?\r\n" );
		}
		else
		{
			meteo.rand_factor = atoi(argument);
			ch_printf( ch, "Maximum random change in vectors now equals %d.\r\n", meteo.rand_factor );
			save_meteo( );
		}
	}
	else if ( !str_cmp(arg, "clima") || !str_cmp(arg, "climate") )
	{
		if ( !is_number(argument) )
		{
			send_to_char( ch, "Set climate effect coefficient to what?\r\n" );
		}
		else
		{
			meteo.climate_factor = atoi( argument );
			ch_printf( ch, "Climate effect coefficient now equals %d.\r\n", meteo.climate_factor );
			save_meteo();
		}
	}
	else if ( !str_cmp(arg, "confine") || !str_cmp(arg, "neighbor") )
	{
		if ( !is_number(argument) )
		{
			send_to_char( ch, "Set neighbor effect divisor to what?\r\n" );
		}
		else
		{
			meteo.neigh_factor = atoi(argument);

			if ( meteo.neigh_factor <= 0 )
				meteo.neigh_factor = 1;

			ch_printf( ch, "Neighbor effect coefficient now equals 1/%d.\r\n", meteo.neigh_factor );
			save_meteo( );
		}
	}
	else if ( !str_cmp_acc(arg, "unità") || !str_cmp(arg, "unit") )
	{
		if ( !is_number(argument) )
		{
			send_to_char( ch, "Set weather unit size to what?\r\n" );
		}
		else
		{
			meteo.weath_unit = atoi( argument );
			ch_printf( ch, "Weather unit size now equals %d.\r\n", meteo.weath_unit );
			save_meteo( );
		}
	}
	else if ( !str_cmp(arg, "vettoremassimo") || !str_cmp(arg, "maxvector") )
	{
		if ( !is_number(argument) )
		{
			send_to_char( ch, "Set maximum vector size to what?\r\n" );
		}
		else
		{
			meteo.max_vector = atoi( argument );
			ch_printf( ch, "Maximum vector size now equals %d.\r\n", meteo.max_vector );
			save_meteo( );
		}
	}
	else if ( !str_cmp(arg, "resetta") || !str_cmp(arg, "reset") )
	{
		init_area_weather( );
		send_to_char(ch, "Weather system reinitialized.\r\n");
	}
	else if ( !str_cmp(arg, "aggiorna") || !str_cmp(arg, "update") )
	{
		int x;
		int	number;

		number = atoi( argument );

		if ( number < 1 )
			number = 1;

		for ( x = 0;  x < number;  x++ )
			update_weather( );

		send_to_char( ch, "Weather system updated.\r\n" );
	}
	else
	{
		send_to_char( ch, "You may only use one of the following fields:\r\n" );
		send_to_char( ch, "    random\r\n"		);
		send_to_char( ch, "    climate\r\n"		);
		send_to_char( ch, "    neighbor\r\n"	);
		send_to_char( ch, "    unit\r\n"		);
		send_to_char( ch, "    maxvector\r\n"	);
		send_to_char( ch, "You may also reset or update the system using the fields " );
		send_to_char( ch, "'reset' and 'update' respectively.\r\n" );
	}
}


/*
 * Produce a description of the weather based on area weather using
 * the following sentence format:
 *		<combo-phrase> and <single-phrase>.
 * Where the combo-phrase describes either the precipitation and
 * temperature or the wind and temperature. The single-phrase
 * describes either the wind or precipitation depending upon the
 * combo-phrase.
 * (Under Construction)
 */
DO_RET do_weather( CHAR_DATA *ch, char *argument )
{
	char	*combo;
	char	*single;
	int		 temp;
	int		 precip;
	int		 wind;

	if ( !is_outside(ch) )
	{
		send_to_char( ch, "Al chiuso non posso vedere il cielo.\r\n" );
		return;
	}

	temp   = ( ch->in_room->area->weather->temp	  + meteo.weath_unit*3 - 1 ) / meteo.weath_unit;
	precip = ( ch->in_room->area->weather->precip + meteo.weath_unit*3 - 1 ) / meteo.weath_unit;
	wind   = ( ch->in_room->area->weather->wind	  + meteo.weath_unit*3 - 1 ) / meteo.weath_unit;

	if ( precip >= 3 )
	{
		combo = preciptemp_msg[precip][temp];
		single = wind_msg[wind];
	}
	else
	{
		combo = windtemp_msg[wind][temp];
		single = precip_msg[precip];
	}

	set_char_color( AT_BLUE, ch );
	ch_printf( ch, "%s e %s.\r\n", combo, single );
}


/*
 * Function to allow modification of an area's climate
 */
DO_RET do_climate( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *area;
	char	   arg[MIL];
	int		   x;

	/* Little error checking */
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_climate: ch è NULL" );
		return;
	}
	else if ( !ch->in_room )
	{
		send_log( NULL, LOG_BUG, "do_climate: ch non è in una stanza" );
		return;
	}
	else if ( !ch->in_room->area )
	{
		send_log( NULL, LOG_BUG, "do_climate: ch non è nell'area" );
		return;
	}
	else if ( !ch->in_room->area->weather )
	{
		send_log( NULL, LOG_BUG, "do_climate: area con dati weather NULL" );
		return;
	}

	set_char_color( AT_BLUE, ch );

	area = ch->in_room->area;

	argument = strlower( argument );
	argument = one_argument( argument, arg );

	/* Display current climate settings */
	if ( !VALID_STR(arg) )
	{
		NEIGHBOR_DATA *neigh;

		ch_printf( ch, "%s:\r\n", area->name );
		ch_printf( ch, "\tTemperature:\t%s\r\n",	temp_settings[area->weather->climate_temp]	 );
		ch_printf( ch, "\tPrecipitation:\t%s\r\n",	precip_settings[area->weather->climate_precip] );
		ch_printf( ch, "\tWind:\t\t%s\r\n",			wind_settings[area->weather->climate_wind]	 );

		if ( area->weather->first_neighbor )
			ch_printf( ch, "\r\nNeighboring weather systems:\r\n" );

		for ( neigh = area->weather->first_neighbor;  neigh;  neigh = neigh->next )
			ch_printf( ch, "\t%s\r\n", neigh->name );

		return;
	}
	/* set climate temperature */
	else if ( is_name(arg, "temperatura temp") )
	{
		argument = one_argument( argument, arg );

		for ( x = 0;  x < MAX_CLIMATE;  x++ )
		{
			if ( str_cmp(arg, temp_settings[x]) )
				continue;

			area->weather->climate_temp = x;
			ch_printf( ch, "The climate temperature for %s is now %s.\r\n", area->name, temp_settings[x] );

			break;
		}

		if ( x == MAX_CLIMATE )
		{
			send_to_char( ch, "Possible temperature settings:\r\n" );
			for ( x = 0;  x < MAX_CLIMATE;  x++ )
				ch_printf( ch,"\t%s\r\n", temp_settings[x] );
		}

		return;
	}
	/* set climate precipitation */
	else if ( is_name(arg, "precipitazione precip") )
	{
		argument = one_argument(argument, arg);

		for ( x = 0;  x < MAX_CLIMATE;  x++ )
		{
			if ( str_cmp(arg, precip_settings[x]) )
				continue;

			area->weather->climate_precip = x;
			ch_printf( ch, "The climate precipitation for %s is now %s.\r\n",
				area->name, precip_settings[x] );

			break;
		}

		if ( x == MAX_CLIMATE )
		{
			send_to_char( ch, "Possible precipitation settings:\r\n" );
			for ( x = 0;  x < MAX_CLIMATE;  x++ )
				ch_printf( ch, "\t%s\r\n", precip_settings[x] );
		}
		return;
	}
	/* set climate wind */
	else if ( is_name(arg, "vento wind") )
	{
		argument = one_argument(argument, arg);

		for ( x = 0;  x < MAX_CLIMATE;  x++ )
		{
			if ( str_cmp(arg, wind_settings[x]) )
				continue;

			area->weather->climate_wind = x;
			ch_printf( ch, "The climate wind for %s is now %s.\r\n", area->name, wind_settings[x] );

			break;
		}

		if ( x == MAX_CLIMATE )
		{
			send_to_char( ch, "Possible wind settings:\r\n" );
			for ( x = 0;  x < MAX_CLIMATE;  x++ )
				ch_printf( ch, "\t%s\r\n", wind_settings[x] );
		}

		return;
	}
	/* add or remove neighboring weather systems */
	else if ( is_name(arg, "confine neighbor") )
	{
		NEIGHBOR_DATA *neigh;
		AREA_DATA	  *tarea;

		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Add or remove which area?\r\n" );
			return;
		}

		/* look for a matching list item */
		for ( neigh = area->weather->first_neighbor;  neigh;  neigh = neigh->next )
		{
			if ( nifty_is_name(argument, neigh->name) )
				break;
		}

		/* if the a matching list entry is found, remove it */
		if ( neigh )
		{
			/* look for the neighbor area in question */
			if ( !(tarea = neigh->address) )
				tarea = get_area(neigh->name);

			/* if there is an actual neighbor area remove its entry to this area */
			if ( tarea )
			{
				NEIGHBOR_DATA *tneigh;

				tarea = neigh->address;
				for ( tneigh = tarea->weather->first_neighbor;  tneigh;  tneigh = tneigh->next)
				{
					if ( !strcmp(area->name, tneigh->name) )
						break;
				}

				UNLINK(tneigh,
					tarea->weather->first_neighbor,
					tarea->weather->last_neighbor,
					next, prev);
				DISPOSE(tneigh->name);
				DISPOSE(tneigh);
			}

			UNLINK(neigh,
				area->weather->first_neighbor,
				area->weather->last_neighbor,
				next, prev);

			ch_printf( ch,"The weather in %s and %s no longer affect each other.\r\n",
				neigh->name, area->name);

			DISPOSE(neigh->name);
			DISPOSE(neigh);
		}
		/* otherwise add an entry */
		else
		{
			tarea = get_area( argument );

			if ( !tarea )
			{
				send_to_char( ch, "No such area exists.\r\n" );
				return;
			}
			else if ( tarea == area )
			{
				ch_printf( ch, "%s already affects its own weather.\r\n", area->name );
				return;
			}

			/* add the entry */
			CREATE( neigh, NEIGHBOR_DATA, 1 );
			neigh->name = str_dup( tarea->name );
			neigh->address = tarea;
			LINK( neigh,
				area->weather->first_neighbor,
				area->weather->last_neighbor,
				next, prev );

			/* add an entry to the neighbor's list */
			CREATE( neigh, NEIGHBOR_DATA, 1 );
			neigh->name = str_dup( area->name );
			neigh->address = area;
			LINK( neigh,
				tarea->weather->first_neighbor,
				tarea->weather->last_neighbor,
				next, prev );

			ch_printf( ch, "The weather in %s and %s now affect one another.\r\n",
				tarea->name, area->name );
		}
		return;
	}
	else
	{
		send_to_char( ch, "Climate may only be followed by one of the following fields:\r\n" );
		send_to_char( ch, "    temp\r\n"	 );
		send_to_char( ch, "    precip\r\n"	 );
		send_to_char( ch, "    wind\r\n"	 );
		send_to_char( ch, "    neighbor\r\n" );

		return;
	}
}
