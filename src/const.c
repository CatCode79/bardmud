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


/****************************************************************************\
 *						Modulo delle costanti del Mud						<
\****************************************************************************/


#include "mud.h"


/*
 * Tabella dei punti
 * Valore sit calcolato dividendo per due la sottrazione tra sleep e rest e sottraendo il risultato a rest
 */
const struct points_type table_points[] =
{
/*		code							attr1		attr2		base	div	sit		rest	sleep	starting */
	{ { POINTS_LIFE, "POINTS_LIFE",	"vita"		},	ATTR_CON,	ATTR_STR,	10,		1,	0.87,	1.25,	2.0,	 20	},
	{ { POINTS_MOVE, "POINTS_MOVE",	"movimento"	},	ATTR_RES,	ATTR_SPE,	20,		2,	1.50,	2.50,	4.5,	100	},
	{ { POINTS_MANA, "POINTS_MANA",	"mana"		},	ATTR_ABS,	ATTR_INT,	15,		1,	1.00,	1.75,	3.25,	100	},
	{ { POINTS_SOUL, "POINTS_SOUL",	"anima"		},	ATTR_SPI,	ATTR_MIG,	 5,		3,	0.25,	0.50,	0.75,	100	}
};
const int max_check_points = sizeof(table_points)/sizeof(struct points_type);


/*
 * Tabella delle unità di misura, divise per piani
 * (GR) togliere i plurali e ricavarli con le funzioni grammaticali
 * (FF) da inserirle in giro dopo aver creato bene le wild e i piani
 */
const struct measure_unit_type table_measureunit[] =
{
	{ "grammo",	"chilo",	"centimetro",	"metro",	"grammi",	"chili",	"centimetri",	"metri",	"gr",	"kg",	"mt",	"cm"	},
	{ "seme",	"pomo",		"dito",			"asta",		"semi",		"pomi",		"dita",			"aste",		"sm",	"pm",	"dt",	"as"	}
};


/*
 * Valori medi per i mob.
 * (RR) (CC) Per ora vengono caricati dinamicamente nei primi 50-65 livelli
 *	da metterli nei primi 50, poi spostarli nel suo doppio e calcolare
 *	quelli vuoti come una media
 */
struct mob_stats_type	table_mob_stats[LVL_LEGEND];


/*
 * Tabella dei bonus e malus relativi a:
 * A = tohit: (forza)
 * B = dodam: (forza)
 * C = defensive: (destrezza)
 */
const struct attr_bonus_type table_attr_bonus[MAX_LEARN_ATTR+1] =
{
/*	   A	 B	  C */
	{  -9,  -9,  100 },	/* 0, non dovrebbe mai accadere questo caso */
	{  -5,  -4,   58 },
	{  -5,  -4,   55 },
	{  -5,  -4,   53 },
	{  -5,  -4,   50 },
	{  -5,  -4,   50 },	/* 5 */
	{  -4,  -3,   50 },
	{  -4,  -3,   50 },
	{  -3,  -2,   50 },
	{  -3,  -2,   48 },
	{  -3,  -2,   45 },	/* 10 */
	{  -3,  -2,   43 },
	{  -3,  -1,   40 },
	{  -3,  -1,   38 },
	{  -3,  -1,   35 },
	{  -3,  -1,   33 },	/* 15 */
	{  -2,  -1,   30 },
	{  -2,  -1,   28 },
	{  -2,  -1,   25 },
	{  -2,  -1,   23 },
	{  -2,  -1,   20 },	/* 20 */
	{  -2,  -1,   18 },
	{  -2,  -1,   15 },
	{  -2,  -1,   13 },
	{  -1,   0,   10 },
	{  -1,   0,    8 },	/* 25 */
	{  -1,   0,    5 },
	{  -1,   0,    3 },
	{  -1,   0,    0 },
	{  -1,   0,    0 },
	{  -1,   0,    0 },	/* 30 */
	{  -1,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },	/* 35 */
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },	/* 40 */
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },	/* 45 */
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },	/* 50 */
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },
	{   0,   0,    0 },	/* 55 */
	{   0,   1,    0 },
	{   0,   1,   -2 },
	{   0,   1,   -5 },
	{   0,   1,   -7 },
	{   1,   1,  -10 },	/* 60 */
	{   1,   1,  -11 },
	{   1,   1,  -12 },
	{   1,   1,  -13 },
	{   1,   2,  -15 },
	{   1,   2,  -16 },	/* 65 */
	{   1,   2,  -17 },
	{   1,   2,  -18 },
	{   2,   3,  -20 },
	{   2,   3,  -22 },
	{   2,   3,  -25 },	/* 70 */
	{   2,   3,  -27 },
	{   2,   4,  -30 },
	{   2,   4,  -32 },
	{   2,   4,  -35 },
	{   2,   4,  -37 },	/* 75 */
	{   3,   5,  -40 },
	{   3,   5,  -32 },
	{   3,   5,  -35 },
	{   3,   5,  -37 },
	{   3,   6,  -50 },	/* 80 */
	{   3,   6,  -52 },
	{   3,   6,  -55 },
	{   3,   6,  -57 },
	{   4,   7,  -60 },
	{   4,   7,  -63 },	/* 85 */
	{   4,   7,  -67 },
	{   4,   7,  -71 },
	{   5,   7,  -75 },
	{   5,   7,  -78 },
	{   5,   7,  -82 },	/* 90 */
	{   5,   7,  -86 },
	{   6,   8,  -90 },
	{   6,   8,  -93 }, 
	{   7,   9,  -97 },
	{   7,   9, -101 },	/* 95 */
	{   8,  10, -105 },
	{   8,  10, -108 },
	{   9,  11, -112 },
	{   9,  11, -116 },
	{  10,  12, -120 }	/* 100 */
};
