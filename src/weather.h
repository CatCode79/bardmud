#ifndef WEATHER_H
#define WEATHER_H


/*
 * Definizioni della struttura meteo
 */
typedef struct	meteo_data		METEO_DATA;


/*
 * Time and weather stuff
 */
typedef enum
{
	SUN_DARK,	/* inizio della notte */
	SUN_RISE,	/* aurora */
	SUN_LIGHT,	/* alba */
	SUN_SET		/* tramonto */
} sun_positions;


/*
 * Enumerazione dei tipi di cielo
 * (FF) non son convinto che bastino per fare i sound, ma per ora ci si accontenta
 */
typedef enum
{
	SKY_CLOUDLESS,		/* Senza nubi */
	SKY_CLOUDY,			/* Nuvoloso */
	SKY_RAINING,		/* Pioggia */
	SKY_SNOW,			/* Neve */
	SKY_LIGHTNING		/* Fulmini */
} sky_conditions;


/*
 * Struttura delle informazioni meteo di base
 */
struct meteo_data
{
	int		weath_unit;
	int		rand_factor;
	int		climate_factor;
	int		neigh_factor;
	int		max_vector;
};


struct neighbor_data
{
	NEIGHBOR_DATA *next;
	NEIGHBOR_DATA *prev;
	char		  *name;
	AREA_DATA	  *address;
};


/*
 * Struttura weather di un'area
 */
struct weather_data
{
	NEIGHBOR_DATA *first_neighbor;		/* areas which affect weather sys	*/
	NEIGHBOR_DATA *last_neighbor;
	int			   sky;					/* Tipo di cielo					*/
	int 		   temp;				/* temperature						*/
	int			   precip;				/* precipitation					*/
	int			   wind;				/* umm... wind						*/
	int			   temp_vector;			/* vectors controlling				*/
	int			   precip_vector;		/* rate of change					*/
	int			   wind_vector;
	int			   climate_temp;		/* climate of the area				*/
	int			   climate_precip;
	int			   climate_wind;
};


/*
 * Variabile delle informazioni meteo di base
 */
extern	METEO_DATA		meteo;


/*
 * Funzioni
 */
void	load_meteo		( void );
void	update_weather	( void );


#endif	/* WEATHER_H */
