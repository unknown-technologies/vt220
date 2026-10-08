#include <string.h>

#include "types.h"
#include "vt.h"

#define	VT220_SETUP_MOVE_NONE			0
#define	VT220_SETUP_MOVE_UP			1
#define	VT220_SETUP_MOVE_DOWN			2
#define	VT220_SETUP_MOVE_LEFT			3
#define	VT220_SETUP_MOVE_RIGHT			4
#define	VT220_SETUP_MOVE_LEFT_MARGIN		5

#define	SETUP_SCREEN_DIRECTORY	0
#define	SETUP_SCREEN_DISPLAY	1
#define	SETUP_SCREEN_GENERAL	2
#define	SETUP_SCREEN_COMM	3
#define	SETUP_SCREEN_PRINTER	4
#define	SETUP_SCREEN_KEYBOARD	5
#define	SETUP_SCREEN_TAB	6
#define	SETUP_SCREEN_COUNT	7

static const char* vt220_setup_screen_names[SETUP_SCREEN_COUNT] = {
	"Set-Up Directory",
	"Display Set-Up",
	"General Set-Up",
	"Communications Set-Up",
	"Printer Set-Up",
	"Keyboard Set-Up",
	"Tab Set-Up"
};

static const char* vt220_setup_screen_names_french[SETUP_SCREEN_COUNT] = {
	"R\xE9pertoire des modes de fonct.",
	"Mode de fonct. de l'\xE9""cran",
	"Mode de fonct. g\xE9n\xE9ral",
	"Mode de fonct. communications",
	"Mode de fonct. de l'imprimante",
	"Mode de fonct. du clavier",
	"Mode de fonct. des tab."
};

static const char* vt220_setup_screen_names_german[SETUP_SCREEN_COUNT] = {
	"Auswahlbild \xDC""BERSICHT",
	"Auswahlbild ANZEIGE",
	"Auswahlbild ALLGEMEINES",
	"Auswahlbild KOMMUNIKATION",
	"Auswahlbild DRUCKER",
	"Auswahlbild TASTATUR",
	"Auswahlbild TAB"
};

static const char* vt220_keyboard_languages_english[16] = {
	"Unknown Keyboard",
	"North American Keyboard",
	"British Keyboard",
	"Flemish Keyboard",
	"Canadian(French) Keyboard",
	"Danish Keyboard",
	"Finnish Keyboard",
	"German Keyboard",
	"Dutch Keyboard",
	"Italian Keyboard",
	"Swiss(French) Keyboard",
	"Swiss(German) Keyboard",
	"Swedish Keyboard",
	"Norwegian Keyboard",
	"French/Belgian Keyboard",
	"Spanish Keyboard"
};

static const char* vt220_keyboard_languages_french[16] = {
	"Clavier inconnu",
	"Clavier nord am\xE9ricain",
	"Clavier britannique",
	"Clavier flamand",
	"Clavier canadien fran\xE7""ais",
	"Clavier danois",
	"Clavier finnois",
	"Clavier allemand",
	"Clavier hollandais",
	"Clavier italien",
	"Clavier suiesse fran\xE7""ais",
	"Clavier suisse allemand",
	"Clavier su\xE9""dois",
	"Clavier norv\xE9gien",
	"Clavier fran\xE7""ais/belge",
	"Clavier espagnol"
};

static const char* vt220_keyboard_languages_german[16] = {
	"unbekannte Tastatur",
	"nordamerikanische Tastatur",
	"britische Tastatur",
	"fl\xE4mische Tastatur",
	"kanadische(franz) Tastatur",
	"d\xE4nische Tastatur",
	"finnische Tastatur",
	"deutsche Tastatur",
	"holl\xE4ndische Tastatur",
	"italienische Tastatur",
	"schweizerische(franz) Tastatur",
	"schweizerische(deutsch) Tastatur",
	"schwedische Tastatur",
	"norwegische Tastatur",
	"franz/belgische Tastatur",
	"spanische Tastatur"
};

typedef struct {
	unsigned char	width;
	const char*	label;
} FIELD;

/******************************************************************************/
#define	DIRECTORY_DISPLAY		0
#define	DIRECTORY_GENERAL		1
#define	DIRECTORY_COMM			2
#define	DIRECTORY_PRINTER		3
#define	DIRECTORY_KEYBOARD		4
#define DIRECTORY_TAB			5
#define	DIRECTORY_ON_LINE		6
#define	DIRECTORY_LOCAL			7
#define	DIRECTORY_CLEAR_DISPLAY		8
#define	DIRECTORY_CLEAR_COMM		9
#define	DIRECTORY_RESET_TERMINAL	10
#define	DIRECTORY_RECALL		11
#define	DIRECTORY_SAVE			12
#define	DIRECTORY_SET_UP		13
#define	DIRECTORY_DEFAULT		14
#define	DIRECTORY_EXIT			15

static const FIELD setup_directory_field_names_english[] = {
	/* 00 */ { .width =  7, .label = "Display" },
	/* 01 */ { .width =  7, .label = "General" },
	/* 02 */ { .width =  4, .label = "Comm" },
	/* 03 */ { .width =  7, .label = "Printer" },
	/* 04 */ { .width =  8, .label = "Keyboard" },
	/* 05 */ { .width =  3, .label = "Tab" },
	/* 06 */ { .width =  7, .label = "On Line" },
	/* 07 */ { .width =  7, .label = "Local" },
	/* 08 */ { .width = 13, .label = "Clear Display" },
	/* 09 */ { .width = 10, .label = "Clear Comm" },
	/* 10 */ { .width = 14, .label = "Reset Terminal" },
	/* 11 */ { .width =  6, .label = "Recall" },
	/* 12 */ { .width =  4, .label = "Save" },
	/* 13 */ { .width = 14, .label = "Set-Up=English" },
	/* 14 */ { .width =  7, .label = "Default" },
	/* 15 */ { .width =  4, .label = "Exit" }
};

static const FIELD setup_directory_field_names_french[] = {
	/* 00 */ { .width =  5, .label = "Ecran" },
	/* 01 */ { .width = 11, .label = "G\xE9n\xE9ralit\xE9s" },
	/* 02 */ { .width =  5, .label = "Comm." },
	/* 03 */ { .width = 10, .label = "Imprimante" },
	/* 04 */ { .width =  7, .label = "Clavier" },
	/* 05 */ { .width =  3, .label = "Tab" },
	/* 06 */ { .width =  8, .label = "En ligne" },
	/* 07 */ { .width =  8, .label = "Local" },
	/* 08 */ { .width = 10, .label = "Eff. \xE9""cran" },
	/* 09 */ { .width = 11, .label = "Init. comm." },
	/* 10 */ { .width = 10, .label = "Remise \xE0 0" },
	/* 11 */ { .width =  6, .label = "Rappel" },
	/* 12 */ { .width = 10, .label = "Sauvegarde" },
	/* 13 */ { .width = 23, .label = "Mode de fonct.=Fran\xE7""ais" },
	/* 14 */ { .width =  6, .label = "D\xE9""faut" },
	/* 15 */ { .width =  6, .label = "Sortie" }
};

static const FIELD setup_directory_field_names_german[] = {
	/* 00 */ { .width =  7, .label = "ANZEIGE" },
	/* 01 */ { .width = 11, .label = "ALLGEMEINES" },
	/* 02 */ { .width =  4, .label = "KOMM" },
	/* 03 */ { .width =  7, .label = "DRUCKER" },
	/* 04 */ { .width =  8, .label = "TASTATUR" },
	/* 05 */ { .width =  3, .label = "TAB" },
	/* 06 */ { .width =  6, .label = "online" },
	/* 07 */ { .width =  6, .label = "lokal" },
	/* 08 */ { .width = 15, .label = "Anzeige l\xF6schen" },
	/* 09 */ { .width = 11, .label = "Komm r\xFC""cks." },
	/* 10 */ { .width = 15, .label = "Terminal r\xFC""cks." },
	/* 11 */ { .width =  7, .label = "abrufen" },
	/* 12 */ { .width =  9, .label = "speichern" },
	/* 13 */ { .width = 19, .label = "Auswahlbild=Deutsch" },
	/* 14 */ { .width =  8, .label = "Standard" },
	/* 15 */ { .width =  6, .label = "FERTIG" }
};

static const FIELD* setup_directory_field_names[] = {
	setup_directory_field_names_english,
	setup_directory_field_names_french,
	setup_directory_field_names_german
};

/******************************************************************************/
#define	DISPLAY_TO_NEXT_SET_UP		0
#define	DISPLAY_TO_DIRECTORY		1
#define	DISPLAY_80_COLUMNS		2
#define	DISPLAY_132_COLUMNS		3
#define	DISPLAY_INTERPRET_CONTROLS	4
#define	DISPLAY_DISPLAY_CONTROLS	5
#define	DISPLAY_NO_AUTO_WRAP		6
#define	DISPLAY_AUTO_WRAP		7
#define	DISPLAY_SMOOTH_SCROLL		8
#define	DISPLAY_JUMP_SCROLL		9
#define	DISPLAY_LIGHT_TEXT_DARK_SCREEN	10
#define	DISPLAY_DARK_TEXT_LIGHT_SCREEN	11
#define	DISPLAY_CURSOR			12
#define	DISPLAY_NO_CURSOR		13
#define	DISPLAY_BLOCK_CURSOR_STYLE	14
#define	DISPLAY_UNDERLINE_CURSOR_STYLE	15

static const FIELD display_field_names_english[] = {
	/* 00 */ { .width = 14, .label = "To Next Set-Up" },
	/* 01 */ { .width = 12, .label = "To Directory" },
	/* 02 */ { .width = 11, .label = "80 Columns" },
	/* 03 */ { .width = 11, .label = "132 Columns" },
	/* 04 */ { .width = 18, .label = "Interpret Controls" },
	/* 05 */ { .width = 18, .label = "Display Controls" },
	/* 06 */ { .width = 12, .label = "No Auto Wrap" },
	/* 07 */ { .width = 12, .label = "Auto Wrap" },
	/* 08 */ { .width = 13, .label = "Smooth Scroll" },
	/* 09 */ { .width = 13, .label = "Jump Scroll" },
	/* 10 */ { .width = 23, .label = "Light Text, Dark Screen" },
	/* 11 */ { .width = 23, .label = "Dark Text, Light Screen" },
	/* 12 */ { .width =  9, .label = "Cursor" },
	/* 13 */ { .width =  9, .label = "No Cursor" },
	/* 14 */ { .width = 22, .label = "Block Cursor Style" },
	/* 15 */ { .width = 22, .label = "Underline Cursor Style" }
};

static const FIELD display_field_names_french[] = {
	/* 00 */ { .width = 16, .label = "Mode de f. suiv." },
	/* 01 */ { .width = 15, .label = "Vers r\xE9pertoire" },
	/* 02 */ { .width =  8, .label = "80 Col." },
	/* 03 */ { .width =  8, .label = "132 Col." },
	/* 04 */ { .width = 23, .label = "Sans visu. caract. ctr." },
	/* 05 */ { .width = 23, .label = "Visu. caract. ctr." },
	/* 06 */ { .width = 16, .label = "Sans retour auto" },
	/* 07 */ { .width = 16, .label = "Retour auto" },
	/* 08 */ { .width = 13, .label = "D\xE9""fil\xE9 lent" },
	/* 09 */ { .width = 13, .label = "D\xE9""fil\xE9 rapide" },
	/* 10 */ { .width = 17, .label = "Affichage normal" },
	/* 11 */ { .width = 17, .label = "Affichage invers\xE9" },
	/* 12 */ { .width = 12, .label = "Curseur" },
	/* 13 */ { .width = 12, .label = "Sans curseur" },
	/* 14 */ { .width =  8, .label = "Bloc" },
	/* 15 */ { .width =  8, .label = "Soulign\xE9" }
};

static const FIELD display_field_names_german[] = {
	/* 00 */ { .width = 15, .label = "N\xC4""CHSTE AUSWAHL" },
	/* 01 */ { .width =  9, .label = "\xDC""BERSICHT" },
	/* 02 */ { .width = 11, .label = "80 Zeichen" },
	/* 03 */ { .width = 11, .label = "132 Zeichen" },
	/* 04 */ { .width = 24, .label = "Steuerzeichen unsichtbar" },
	/* 05 */ { .width = 24, .label = "Steuerzeichen sichtbar" },
	/* 06 */ { .width = 17, .label = "kein Auto-Umbruch" },
	/* 07 */ { .width = 17, .label = "Auto-Umbruch" },
	/* 08 */ { .width = 16, .label = "weich Abrollen" },
	/* 09 */ { .width = 16, .label = "schnell Abrollen" },
	/* 10 */ { .width = 18, .label = "Normal-Darstellung" },
	/* 11 */ { .width = 18, .label = "Invers-Darstellung" },
	/* 12 */ { .width = 18, .label = "Schreibmarke" },
	/* 13 */ { .width = 18, .label = "keine Schreibmarke" },
	/* 14 */ { .width = 19, .label = "Block-Schreibmarke" },
	/* 15 */ { .width = 19, .label = "Strich-Schreibmarke" }
};

static const FIELD* display_field_names[] = {
	display_field_names_english,
	display_field_names_french,
	display_field_names_german
};

/******************************************************************************/
#define	GENERAL_TO_NEXT_SET_UP		0
#define	GENERAL_TO_DIRECTORY		1
#define	GENERAL_VT200_MODE_7BIT_CTRLS	2
#define	GENERAL_VT200_MODE_8BIT_CTRLS	3
#define	GENERAL_VT52_MODE		4
#define	GENERAL_VT100_MODE		5
#define	GENERAL_VT220_ID		6
#define	GENERAL_VT100_ID		7
#define	GENERAL_VT101_ID		8
#define	GENERAL_VT102_ID		9
#define	GENERAL_UDK_UNLOCKED		10
#define	GENERAL_UDK_LOCKED		11
#define	GENERAL_FEATURES_UNLOCKED	12
#define	GENERAL_FEATURES_LOCKED		13
#define	GENERAL_MULTINATIONAL		14
#define	GENERAL_NATIONAL		15
#define	GENERAL_NUMERIC_KEYPAD		16
#define	GENERAL_APPLICATION_KEYPAD	17
#define	GENERAL_NORMAL_CURSOR_KEYS	18
#define	GENERAL_APPLICATION_CURSOR_KEYS	19
#define	GENERAL_NO_NEW_LINE		20
#define	GENERAL_NEW_LINE		21

static const FIELD general_field_names_english[] = {
	/* 00 */ { .width = 14, .label = "To Next Set-Up" },
	/* 01 */ { .width = 12, .label = "To Directory" },
	/* 02 */ { .width = 26, .label = "VT200 Mode, 7 Bit Controls" },
	/* 03 */ { .width = 26, .label = "VT200 Mode, 8 Bit Controls" },
	/* 04 */ { .width = 26, .label = "VT52 Mode" },
	/* 05 */ { .width = 26, .label = "VT100 Mode" },
	/* 06 */ { .width =  8, .label = "VT220 ID" },
	/* 07 */ { .width =  8, .label = "VT100 ID" },
	/* 08 */ { .width =  8, .label = "VT101 ID" },
	/* 09 */ { .width =  8, .label = "VT102 ID" },
	/* 10 */ { .width = 26, .label = "User Defined Keys Unlocked" },
	/* 11 */ { .width = 26, .label = "User Defined Keys Locked" },
	/* 12 */ { .width = 22, .label = "User Features Unlocked" },
	/* 13 */ { .width = 22, .label = "User Features Locked" },
	/* 14 */ { .width = 13, .label = "Multinational" },
	/* 15 */ { .width = 13, .label = "National" },
	/* 16 */ { .width = 18, .label = "Numeric Keypad" },
	/* 17 */ { .width = 18, .label = "Application Keypad" },
	/* 18 */ { .width = 23, .label = "Normal Cursor Keys" },
	/* 19 */ { .width = 23, .label = "Application Cursor Keys" },
	/* 20 */ { .width = 11, .label = "No New Line" },
	/* 21 */ { .width = 11, .label = "New Line" }
};

static const FIELD general_field_names_french[] = {
	/* 00 */ { .width = 16, .label = "Mode de f. suiv." },
	/* 01 */ { .width = 15, .label = "Vers r\xE9pertoire" },
	/* 02 */ { .width = 26, .label = "VT200, Car. de ctrl 7 bits" },
	/* 03 */ { .width = 26, .label = "VT200, Car. de ctrl 8 bits" },
	/* 04 */ { .width = 26, .label = "VT52" },
	/* 05 */ { .width = 26, .label = "VT100" },
	/* 06 */ { .width =  8, .label = "ID VT220" },
	/* 07 */ { .width =  8, .label = "ID VT100" },
	/* 08 */ { .width =  8, .label = "ID VT101" },
	/* 09 */ { .width =  8, .label = "ID VT102" },
	/* 10 */ { .width = 22, .label = "T.D.U. modifiables" },
	/* 11 */ { .width = 22, .label = "T.D.U. non modifiables" },
	/* 12 */ { .width = 22, .label = "F.D.U. modifiables" },
	/* 13 */ { .width = 22, .label = "F.D.U. non modifiables" },
	/* 14 */ { .width = 13, .label = "Multinational" },
	/* 15 */ { .width = 13, .label = "National" },
	/* 16 */ { .width = 16, .label = "Mode num\xE9rique" },
	/* 17 */ { .width = 16, .label = "Mode application" },
	/* 18 */ { .width = 21, .label = "Mode d\xE9placement" },
	/* 19 */ { .width = 21, .label = "Mode application sec." },
	/* 20 */ { .width = 11, .label = "M\xEAme ligne" },
	/* 21 */ { .width = 11, .label = "Nelle ligne" }
};

static const FIELD general_field_names_german[] = {
	/* 00 */ { .width = 15, .label = "N\xC4""CHSTE AUSWAHL" },
	/* 01 */ { .width =  9, .label = "\xDC""BERSICHT" },
	/* 02 */ { .width = 27, .label = "VT200-Modus, 7-Bit-Steuerng" },
	/* 03 */ { .width = 27, .label = "VT200-Modus, 8-Bit-Steuerng" },
	/* 04 */ { .width = 27, .label = "VT52-Modus" },
	/* 05 */ { .width = 27, .label = "VT100-Modus" },
	/* 06 */ { .width =  9, .label = "ID. VT220" },
	/* 07 */ { .width =  9, .label = "ID. VT100" },
	/* 08 */ { .width =  9, .label = "ID. VT101" },
	/* 09 */ { .width =  9, .label = "ID. VT102" },
	/* 10 */ { .width = 17, .label = "F-Tasten frei" },
	/* 11 */ { .width = 17, .label = "F-Tasten gesperrt" },
	/* 12 */ { .width = 25, .label = "Benutzer Auswahl frei" },
	/* 13 */ { .width = 25, .label = "Benutzer Auswahl gesperrt" },
	/* 14 */ { .width = 13, .label = "multinational" },
	/* 15 */ { .width = 13, .label = "national" },
	/* 16 */ { .width = 22, .label = "Numerik-Tastenblock" },
	/* 17 */ { .width = 22, .label = "Anwendungs-Tastenblock" },
	/* 18 */ { .width = 22, .label = "Normal-Pfeiltasten" },
	/* 19 */ { .width = 22, .label = "Anwendungs-Pfeiltasten" },
	/* 20 */ { .width = 13, .label = "gleiche Zeile" },
	/* 21 */ { .width = 13, .label = "neue Zeile" }
};

static const FIELD* general_field_names[] = {
	general_field_names_english,
	general_field_names_french,
	general_field_names_german
};

/******************************************************************************/
#define	COMM_TO_NEXT_SET_UP		0
#define	COMM_TO_DIRECTORY		1
#define	COMM_TRANSMIT			2
#define	COMM_RECEIVE			3
#define	COMM_RECEIVE_TRANSMIT		4
#define	COMM_XOFF_AT_64			5
#define	COMM_XOFF_AT_128		6
#define	COMM_NO_XOFF			7
#define	COMM_8_BITS_NO_PARITY		8
#define	COMM_8_BITS_EVEN_PARITY		9
#define	COMM_8_BITS_ODD_PARITY		10
#define	COMM_8_BITS_EVEN_PARITY_NO_CHK	11
#define	COMM_8_BITS_ODD_PARITY_NO_CHK	12
#define	COMM_7_BITS_EVEN_PARITY_NO_CHK	13
#define	COMM_7_BITS_ODD_PARITY_NO_CHK	14
#define	COMM_7_BITS_NO_PARITY		15
#define	COMM_7_BITS_EVEN_PARITY		16
#define	COMM_7_BITS_ODD_PARITY		17
#define	COMM_7_BITS_MARK_PARITY		18
#define	COMM_7_BITS_SPACE_PARITY	19
#define	COMM_1_STOP_BIT			20
#define	COMM_2_STOP_BITS		21
#define	COMM_NO_LOCAL_ECHO		22
#define	COMM_LOCAL_ECHO			23
#define	COMM_EIA_PORT_DATA_LEADS_ONLY	24
#define	COMM_EIA_PORT_MODEM_CONTROL	25
#define	COMM_20_MA_PORT			26
#define	COMM_DISCONNECT_2_S_DELAY	27
#define	COMM_DISCONNECT_60_MS_DELAY	28
#define	COMM_LIMITED_TRANSMIT		29
#define	COMM_UNLIMITED_TRANSMIT		30

static const FIELD comm_field_names_english[] = {
	/* 00 */ { .width = 14, .label = "To Next Set-Up" },
	/* 01 */ { .width = 12, .label = "To Directory" },
	/* 02 */ { .width =  9, .label = "Transmit=" },
	/* 03 */ { .width =  8, .label = "Receive=" },
	/* 04 */ { .width =  8, .label = "Transmit" },
	/* 05 */ { .width = 11, .label = "XOFF at 64" },
	/* 06 */ { .width = 11, .label = "XOFF at 128" },
	/* 07 */ { .width = 11, .label = "No XOFF" },
	/* 08 */ { .width = 29, .label = "8 Bits, No Parity" },
	/* 09 */ { .width = 29, .label = "8 Bits, Even Parity" },
	/* 10 */ { .width = 29, .label = "8 Bits, Odd Parity" },
	/* 11 */ { .width = 29, .label = "8 Bits, Even Parity, No Check" },
	/* 12 */ { .width = 29, .label = "8 Bits, Odd Parity, No Check" },
	/* 13 */ { .width = 29, .label = "7 Bits, Even Parity, No Check" },
	/* 14 */ { .width = 29, .label = "7 Bits, Odd Parity, No Check" },
	/* 15 */ { .width = 29, .label = "7 Bits, No Parity" },
	/* 16 */ { .width = 29, .label = "7 Bits, Even Parity" },
	/* 17 */ { .width = 29, .label = "7 Bits, Odd Parity" },
	/* 18 */ { .width = 29, .label = "7 Bits, Mark Parity" },
	/* 19 */ { .width = 29, .label = "7 Bits, Space Parity" },
	/* 20 */ { .width = 11, .label = "1 Stop Bit" },
	/* 21 */ { .width = 11, .label = "2 Stop Bits" },
	/* 22 */ { .width = 13, .label = "No Local Echo" },
	/* 23 */ { .width = 13, .label = "Local Echo" },
	/* 24 */ { .width = 25, .label = "EIA Port, Data Leads Only" },
	/* 25 */ { .width = 25, .label = "EIA Port, Modem Control" },
	/* 26 */ { .width = 25, .label = "20 mA Port" },
	/* 27 */ { .width = 23, .label = "Disconnect, 2 s Delay" },
	/* 28 */ { .width = 23, .label = "Disconnect, 60 ms Delay" },
	/* 29 */ { .width = 18, .label = "Limited Transmit" },
	/* 30 */ { .width = 18, .label = "Unlimited Transmit" }
};

static const FIELD comm_field_names_french[] = {
	/* 00 */ { .width = 16, .label = "Mode de f. suiv." },
	/* 01 */ { .width = 15, .label = "Vers r\xE9pertoire" },
	/* 02 */ { .width =  7, .label = "Emiss.=" },
	/* 03 */ { .width =  7, .label = "R\xE9""cep.=" },
	/* 04 */ { .width =  6, .label = "Emiss." },
	/* 05 */ { .width = 10, .label = "XOFF \xE0 64" },
	/* 06 */ { .width = 10, .label = "XOFF \xE0 128" },
	/* 07 */ { .width = 10, .label = "Sans XOFF" },
	/* 08 */ { .width = 30, .label = "8 bits, pas de parit\xE9" },
	/* 09 */ { .width = 30, .label = "8 bits, parit\xE9 paire" },
	/* 10 */ { .width = 30, .label = "8 bits, parit\xE9 impaire" },
	/* 11 */ { .width = 30, .label = "8 bits, parit\xE9 paire, 0 v\xE9rif" },
	/* 12 */ { .width = 30, .label = "8 bits, parit\xE9 impaire,0 v\xE9rif" },
	/* 13 */ { .width = 30, .label = "7 bits, parit\xE9 paire, 0 v\xE9rif" },
	/* 14 */ { .width = 30, .label = "7 bits, parit\xE9 impaire,0 v\xE9rif" },
	/* 15 */ { .width = 30, .label = "7 bits, pas de parit\xE9" },
	/* 16 */ { .width = 30, .label = "7 bits, parit\xE9 paire" },
	/* 17 */ { .width = 30, .label = "7 bits, parit\xE9 impaire" },
	/* 18 */ { .width = 30, .label = "7 Bits, repos" },
	/* 19 */ { .width = 30, .label = "7 Bits, travail" },
	/* 20 */ { .width = 14, .label = "1 bit d'arr\xEAt" },
	/* 21 */ { .width = 14, .label = "2 bits d'arr\xEAt" },
	/* 22 */ { .width = 15, .label = "Sans \xE9""cho local" },
	/* 23 */ { .width = 15, .label = "Echo local" },
	/* 24 */ { .width = 28, .label = "Interface EIA, donn\xE9""es seul." },
	/* 25 */ { .width = 28, .label = "Interface EIA, ctrl de modem" },
	/* 26 */ { .width = 28, .label = "Interface 20 mA" },
	/* 27 */ { .width = 22, .label = "D\xE9""connect\xE9 apr\xE8s 2 s" },
	/* 28 */ { .width = 22, .label = "D\xE9""connect\xE9 apr\xE8s 60 ms" },
	/* 29 */ { .width = 16, .label = "Vit. limit\xE9""e" },
	/* 30 */ { .width = 16, .label = "Vit. non limit\xE9""e" }
};

static const FIELD comm_field_names_german[] = {
	/* 00 */ { .width = 15, .label = "N\xC4""CHSTE AUSWAHL" },
	/* 01 */ { .width =  9, .label = "\xDC""BERSICHT" },
	/* 02 */ { .width =  7, .label = "Senden=" },
	/* 03 */ { .width = 10, .label = "Empfangen=" },
	/* 04 */ { .width =  6, .label = "Senden" },
	/* 05 */ { .width = 12, .label = "XOFF bei 64" },
	/* 06 */ { .width = 12, .label = "XOFF bei 128" },
	/* 07 */ { .width = 12, .label = "kein XOFF" },
	/* 08 */ { .width = 37, .label = "8 Bits, keine Parit\xE4t" },
	/* 09 */ { .width = 37, .label = "8 Bits, gerade Parit\xE4t" },
	/* 10 */ { .width = 37, .label = "8 Bits, ungerade Parit\xE4t" },
	/* 11 */ { .width = 37, .label = "8 Bits, gerade Parit\xE4t, keine Pr\xFC""fung" },
	/* 12 */ { .width = 37, .label = "8 Bits, unger. Parit\xE4t, keine Pr\xFC""fung" },
	/* 13 */ { .width = 37, .label = "7 Bits, gerade Parit\xE4t, keine Pr\xFC""fung" },
	/* 14 */ { .width = 37, .label = "7 Bits, unger. Parit\xE4t, keine Pr\xFC""fung" },
	/* 15 */ { .width = 37, .label = "7 Bits, keine Parit\xE4t" },
	/* 16 */ { .width = 37, .label = "7 Bits, gerade Parit\xE4t" },
	/* 17 */ { .width = 37, .label = "7 Bits, ungerade Parit\xE4t" },
	/* 18 */ { .width = 37, .label = "7 Bits, Stoppolarit\xE4t" },
	/* 19 */ { .width = 37, .label = "7 Bits, Startpolarit\xE4t" },
	/* 20 */ { .width = 10, .label = "1 Stopbit" },
	/* 21 */ { .width = 10, .label = "2 Stopbits" },
	/* 22 */ { .width =  9, .label = "Fernecho" },
	/* 23 */ { .width =  9, .label = "Lokalecho" },
	/* 24 */ { .width = 12, .label = "Null-Modem" },
	/* 25 */ { .width = 12, .label = "Extern-Modem" },
	/* 26 */ { .width = 12, .label = "20 mA" },
	/* 27 */ { .width = 28, .label = "Auto-Trenn, 2 s Abfallzeit" },
	/* 28 */ { .width = 28, .label = "Auto-Trenn, 60 ms Abfallzeit" },
	/* 29 */ { .width = 17, .label = "begrenzt Senden" },
	/* 30 */ { .width = 17, .label = "unbegrenzt Senden" }
};

static const FIELD* comm_field_names[] = {
	comm_field_names_english,
	comm_field_names_french,
	comm_field_names_german
};

/******************************************************************************/
#define	PRINTER_TO_NEXT_SET_UP		0
#define	PRINTER_TO_DIRECTORY		1
#define	PRINTER_SPEED			2
#define	PRINTER_NORMAL_PRINT_MODE	3
#define	PRINTER_AUTO_PRINT_MODE		4
#define	PRINTER_CONTROLLER_MODE		5
#define	PRINTER_8_BITS_NO_PARITY	6
#define	PRINTER_8_BITS_EVEN_PARITY	7
#define	PRINTER_8_BITS_ODD_PARITY	8
#define	PRINTER_7_BITS_NO_PARITY	9
#define	PRINTER_7_BITS_EVEN_PARITY	10
#define	PRINTER_7_BITS_ODD_PARITY	11
#define	PRINTER_7_BITS_MARK_PARITY	12
#define	PRINTER_7_BITS_SPACE_PARITY	13
#define	PRINTER_1_STOP_BIT		14
#define	PRINTER_2_STOP_BITS		15
#define	PRINTER_PRINT_FULL_PAGE		16
#define	PRINTER_PRINT_SCROLL_REGION	17
#define	PRINTER_PRINT_NATIONAL_ONLY	18
#define	PRINTER_NATIONAL_AND_LINE_DRAW	19
#define	PRINTER_PRINT_MULTINATIONAL	20
#define	PRINTER_NO_TERMINATOR		21
#define	PRINTER_TERMINATOR_FF		22

static const FIELD printer_field_names_english[] = {
	/* 00 */ { .width = 14, .label = "To Next Set-Up" },
	/* 01 */ { .width = 12, .label = "To Directory" },
	/* 02 */ { .width =  6, .label = "Speed=" },
	/* 03 */ { .width = 17, .label = "Normal Print Mode" },
	/* 04 */ { .width = 17, .label = "Auto Print Mode" },
	/* 05 */ { .width = 17, .label = "Controller Mode" },
	/* 06 */ { .width = 20, .label = "8 Bits, No Parity" },
	/* 07 */ { .width = 20, .label = "8 Bits, Even Parity" },
	/* 08 */ { .width = 20, .label = "8 Bits, Odd Parity" },
	/* 09 */ { .width = 20, .label = "7 Bits, No Parity" },
	/* 10 */ { .width = 20, .label = "7 Bits, Even Parity" },
	/* 11 */ { .width = 20, .label = "7 Bits, Odd Parity" },
	/* 12 */ { .width = 20, .label = "7 Bits, Mark Parity" },
	/* 13 */ { .width = 20, .label = "7 Bits, Space Parity" },
	/* 14 */ { .width = 11, .label = "1 Stop Bit" },
	/* 15 */ { .width = 11, .label = "2 Stop Bits" },
	/* 16 */ { .width = 19, .label = "Print Full Page" },
	/* 17 */ { .width = 19, .label = "Print Scroll Region" },
	/* 18 */ { .width = 25, .label = "Print National Only" },
	/* 19 */ { .width = 25, .label = "National and Line Drawing" },
	/* 20 */ { .width = 25, .label = "Print Multinational" },
	/* 21 */ { .width = 15, .label = "No Terminator" },
	/* 22 */ { .width = 15, .label = "Terminator = FF" }
};

static const FIELD printer_field_names_french[] = {
	/* 00 */ { .width = 16, .label = "Mode de f. suiv." },
	/* 01 */ { .width = 15, .label = "Vers r\xE9pertoire" },
	/* 02 */ { .width =  5, .label = "Vit.=" },
	/* 03 */ { .width = 15, .label = "Normal" },
	/* 04 */ { .width = 15, .label = "Impression auto" },
	/* 05 */ { .width = 15, .label = "Contr\xF4leur" },
	/* 06 */ { .width = 22, .label = "8 bits, pas de parit\xE9" },
	/* 07 */ { .width = 22, .label = "8 bits, parit\xE9 paire" },
	/* 08 */ { .width = 22, .label = "8 bits, parit\xE9 impaire" },
	/* 09 */ { .width = 22, .label = "7 bits, pas de parit\xE9" },
	/* 10 */ { .width = 22, .label = "7 bits, parit\xE9 paire" },
	/* 11 */ { .width = 22, .label = "7 bits, parit\xE9 impaire" },
	/* 12 */ { .width = 22, .label = "7 bits, repos" },
	/* 13 */ { .width = 22, .label = "7 bits, travail" },
	/* 14 */ { .width = 14, .label = "1 bit d'arr\xEAt" },
	/* 15 */ { .width = 14, .label = "2 bits d'arr\xEAt" },
	/* 16 */ { .width = 18, .label = "Pleine page" },
	/* 17 */ { .width = 18, .label = "Zone de d\xE9""filement" },
	/* 18 */ { .width = 27, .label = "Imprimer National Seulement" },
	/* 19 */ { .width = 27, .label = "National et Graphique" },
	/* 20 */ { .width = 27, .label = "Imprimer Multinational" },
	/* 21 */ { .width = 24, .label = "Sans terminateur" },
	/* 22 */ { .width = 24, .label = "Terminateur = Av. 1 page" }
};

static const FIELD printer_field_names_german[] = {
	/* 00 */ { .width = 15, .label = "N\xC4""CHSTE AUSWAHL" },
	/* 01 */ { .width =  9, .label = "\xDC""BERSICHT" },
	/* 02 */ { .width = 10, .label = "Geschwind=" },
	/* 03 */ { .width = 14, .label = "Normal-Betrieb" },
	/* 04 */ { .width = 14, .label = "Auto-Betrieb" },
	/* 05 */ { .width = 14, .label = "Fern-Betrieb" },
	/* 06 */ { .width = 24, .label = "8 Bits, keine Parit\xE4t" },
	/* 07 */ { .width = 24, .label = "8 Bits, gerade Parit\xE4t" },
	/* 08 */ { .width = 24, .label = "8 Bits, ungerade Parit\xE4t" },
	/* 09 */ { .width = 24, .label = "7 Bits, keine Parit\xE4t" },
	/* 10 */ { .width = 24, .label = "7 Bits, gerade Parit\xE4t" },
	/* 11 */ { .width = 24, .label = "7 Bits, ungerade Parit\xE4t" },
	/* 12 */ { .width = 24, .label = "7 Bits, Stoppolarit\xE4t" },
	/* 13 */ { .width = 24, .label = "7 Bits, Startpolarit\xE4t" },
	/* 14 */ { .width = 10, .label = "1 Stopbit" },
	/* 15 */ { .width = 10, .label = "2 Stopbits" },
	/* 16 */ { .width = 20, .label = "drucke volle Seite" },
	/* 17 */ { .width = 20, .label = "drucke Abrollbereich" },
	/* 18 */ { .width = 21, .label = "nur national drucken" },
	/* 19 */ { .width = 21, .label = "national mit Grafik" },
	/* 20 */ { .width = 21, .label = "multinational drucken" },
	/* 21 */ { .width = 10, .label = "neue Zeile" },
	/* 22 */ { .width = 10, .label = "neue Seite" }
};

static const FIELD* printer_field_names[] = {
	printer_field_names_english,
	printer_field_names_french,
	printer_field_names_german
};

/******************************************************************************/
#define	KEYBOARD_TO_NEXT_SET_UP		0
#define	KEYBOARD_TO_DIRECTORY		1
#define	KEYBOARD_TYPEWRITER_KEYS	2
#define	KEYBOARD_DATA_PROCESSING_KEYS	3
#define	KEYBOARD_CAPS_LOCK		4
#define	KEYBOARD_SHIFT_LOCK		5
#define	KEYBOARD_AUTO_REPEAT		6
#define	KEYBOARD_NO_AUTO_REPEAT		7
#define	KEYBOARD_KEYCLICK		8
#define	KEYBOARD_NO_KEYCLICK		9
#define	KEYBOARD_MARGIN_BELL		10
#define	KEYBOARD_NO_MARGIN_BELL		11
#define	KEYBOARD_WARNING_BELL		12
#define	KEYBOARD_NO_WARNING_BELL	13
#define	KEYBOARD_BREAK			14
#define	KEYBOARD_NO_BREAK		15
#define	KEYBOARD_AUTO_ANSWERBACK	16
#define	KEYBOARD_NO_AUTO_ANSWERBACK	17
#define	KEYBOARD_ANSWERBACK		18
#define	KEYBOARD_ANSWERBACK_CONCEALED	19
#define	KEYBOARD_NOT_CONCEALED		20
#define	KEYBOARD_CONCEALED		21

static const FIELD keyboard_field_names_english[] = {
	/* 00 */ { .width = 14, .label = "To Next Set-Up" },
	/* 01 */ { .width = 12, .label = "To Directory" },
	/* 02 */ { .width = 20, .label = "Typewriter Keys" },
	/* 03 */ { .width = 20, .label = "Data Processing Keys" },
	/* 04 */ { .width = 10, .label = "Caps Lock" },
	/* 05 */ { .width = 10, .label = "Shift Lock" },
	/* 06 */ { .width = 14, .label = "Auto Repeat" },
	/* 07 */ { .width = 14, .label = "No Auto Repeat" },
	/* 08 */ { .width = 11, .label = "Keyclick" },
	/* 09 */ { .width = 11, .label = "No Keyclick" },
	/* 10 */ { .width = 14, .label = "Margin Bell" },
	/* 11 */ { .width = 14, .label = "No Margin Bell" },
	/* 12 */ { .width = 15, .label = "Warning Bell" },
	/* 13 */ { .width = 15, .label = "No Warning Bell" },
	/* 14 */ { .width =  8, .label = "Break" },
	/* 15 */ { .width =  8, .label = "No Break" },
	/* 16 */ { .width = 18, .label = "Auto Answerback" },
	/* 17 */ { .width = 18, .label = "No Auto Answerback" },
	/* 18 */ { .width = 11, .label = "Answerback=" },
	/* 19 */ { .width = 41, .label = "Answerback=<Concealed>" },
	/* 20 */ { .width = 11, .label = "Not Concealed" },
	/* 21 */ { .width = 11, .label = "Concealed" }
};

static const FIELD keyboard_field_names_french[] = {
	/* 00 */ { .width = 16, .label = "Mode de f. suiv." },
	/* 01 */ { .width = 15, .label = "Vers r\xE9pertoire" },
	/* 02 */ { .width = 20, .label = "Touches bureautique" },
	/* 03 */ { .width = 20, .label = "Touches informatique" },
	/* 04 */ { .width = 12, .label = "Caract. sup." },
	/* 05 */ { .width = 12, .label = "MAJ./min." },
	/* 06 */ { .width = 11, .label = "R\xE9p\xE9t. auto" },
	/* 07 */ { .width = 11, .label = "0 R\xE9p. auto" },
	/* 08 */ { .width = 15, .label = "Bip de touche" },
	/* 09 */ { .width = 15, .label = "0 Bip de touche" },
	/* 10 */ { .width = 17, .label = "Alarme de marge" },
	/* 11 */ { .width = 17, .label = "0 Alarme de marge" },
	/* 12 */ { .width =  8, .label = "Alarme" },
	/* 13 */ { .width =  8, .label = "0 Alarme" },
	/* 14 */ { .width = 12, .label = "Interrupt." },
	/* 15 */ { .width = 12, .label = "0 Interrupt." },
	/* 16 */ { .width = 16, .label = "Msg de r\xE9ponse" },
	/* 17 */ { .width = 16, .label = "0 Msg de r\xE9ponse" },
	/* 18 */ { .width = 11, .label = "Msg de r\xE9p=" },
	/* 19 */ { .width = 41, .label = "Msg de r\xE9p=<Cach\xE9>" },
	/* 20 */ { .width =  7, .label = "Affich\xE9" },
	/* 21 */ { .width =  7, .label = "Cach\xE9" }
};

static const FIELD keyboard_field_names_german[] = {
	/* 00 */ { .width = 15, .label = "N\xC4""CHSTE AUSWAHL" },
	/* 01 */ { .width =  9, .label = "\xDC""BERSICHT" },
	/* 02 */ { .width = 21, .label = "Schreibmasch Tastatur" },
	/* 03 */ { .width = 21, .label = "EDV Tastatur" },
	/* 04 */ { .width = 11, .label = "feststellen" },
	/* 05 */ { .width = 11, .label = "umschalten" },
	/* 06 */ { .width = 15, .label = "Typamatik" },
	/* 07 */ { .width = 15, .label = "keine Typamatik" },
	/* 08 */ { .width = 14, .label = "Tastklick" },
	/* 09 */ { .width = 14, .label = "kein Tastklick" },
	/* 10 */ { .width = 15, .label = "Randsignal" },
	/* 11 */ { .width = 15, .label = "kein Randsignal" },
	/* 12 */ { .width = 11, .label = "Signal" },
	/* 13 */ { .width = 11, .label = "kein Signal" },
	/* 14 */ { .width = 11, .label = "VA frei" },
	/* 15 */ { .width = 11, .label = "VA gesperrt" },
	/* 16 */ { .width = 18, .label = "Auto-Antwort" },
	/* 17 */ { .width = 18, .label = "keine Auto-Antwort" },
	/* 18 */ { .width =  8, .label = "Antwort=" },
	/* 19 */ { .width = 38, .label = "Antwort=<unsichtbar>" },
	/* 20 */ { .width = 10, .label = "sichtbar" },
	/* 21 */ { .width = 10, .label = "unsichtbar" }
};

static const FIELD* keyboard_field_names[] = {
	keyboard_field_names_english,
	keyboard_field_names_french,
	keyboard_field_names_german
};

/******************************************************************************/
#define	TAB_TO_NEXT_SET_UP		0
#define	TAB_TO_DIRECTORY		1
#define	TAB_CLEAR_ALL_TABS		2
#define	TAB_SET_8_COLUMN_TABS		3

static const FIELD tab_field_names_english[] = {
	/* 00 */ { .width = 14, .label = "To Next Set-Up" },
	/* 01 */ { .width = 12, .label = "To Directory" },
	/* 02 */ { .width = 14, .label = "Clear All Tabs" },
	/* 03 */ { .width = 17, .label = "Set 8 Column Tabs" }
};

static const FIELD tab_field_names_french[] = {
	/* 00 */ { .width = 16, .label = "Mode de f. suiv." },
	/* 01 */ { .width = 15, .label = "Vers r\xE9pertoire" },
	/* 02 */ { .width = 19, .label = "Annulation des tab." },
	/* 03 */ { .width = 11, .label = "Tab. 8 col." }
};

static const FIELD tab_field_names_german[] = {
	/* 00 */ { .width = 15, .label = "N\xC4""CHSTE AUSWAHL" },
	/* 01 */ { .width =  9, .label = "\xDC""BERSICHT" },
	/* 02 */ { .width = 16, .label = "l\xF6sche Tab-Stops" },
	/* 03 */ { .width = 15, .label = "setze Tab-Stops" }
};

static const FIELD* tab_field_names[] = {
	tab_field_names_english,
	tab_field_names_french,
	tab_field_names_german
};

static unsigned int VT220GetNextBaudRate(unsigned int baud, int allow_zero)
{
	switch(baud) {
		case 0:
			return 75;
		case 75:
			return 110;
		case 110:
			return 150;
		case 150:
			return 300;
		case 300:
			return 600;
		case 600:
			return 1200;
		case 1200:
			return 2400;
		case 2400:
		default:
			return 4800;
		case 4800:
			return 9600;
		case 9600:
			return 19200;
		/* the following rates are for modern use, a real VT220
		 * only supports up to 19200 */
		case 19200:
			return 38400;
		case 38400:
			return 57600;
		case 57600:
			return 115200;
		case 115200:
			if(allow_zero) {
				return 0;
			} else {
				return 75;
			}
		/* Technically various UARTs support higher rates, like 230400
		 * baud, but others do not. It would therefore be more work to
		 * figure out what is/isn't supported on the local hardware. */
	}
}

const char* VT220SetupGetTitle(VT220* vt)
{
	if(vt->setup.screen < 0 || vt->setup.screen >= SETUP_SCREEN_COUNT) {
		return "???";
	} else {
		switch(vt->config.language) {
			default:
			case VT220_LANGUAGE_ENGLISH:
				return vt220_setup_screen_names[vt->setup.screen];
			case VT220_LANGUAGE_FRANCAIS:
				return vt220_setup_screen_names_french[vt->setup.screen];
			case VT220_LANGUAGE_DEUTSCH:
				return vt220_setup_screen_names_german[vt->setup.screen];
		}
	}
}

void VT220SetupEraseDisplay(VT220* vt)
{
	memset(vt->setup.text, 0, 8 * 132 * sizeof(VT220CELL));
	memset(vt->setup.line_attributes, 0, 8);
}

void VT220SetupEraseLine(VT220* vt)
{
	memset(&vt->setup.text[vt->setup.write_y * vt->columns], 0, vt->columns * sizeof(VT220CELL));
}

void VT220SetupWrite(VT220* vt, const u16 c, const int sgr, int display_controls)
{
	u16 glyph = c;
	if(display_controls) {
		if(glyph == 0x20) {
			glyph = 0;
		} else if(glyph < 0x20) {
			glyph += 0x100;
		}
	} else if(c == 0x20) {
		glyph = 0;
	}

	int idx = vt->setup.write_y * vt->columns + vt->setup.write_x;
	vt->setup.text[idx].text = glyph;
	vt->setup.text[idx].attr = sgr;

	vt->setup.write_x++;
	if(vt->setup.write_x >= vt->columns) {
		vt->setup.write_x = 0;
		vt->setup.write_y++;
		if(vt->setup.write_y >= 8) {
			vt->setup.write_y = 7;
		}
	}
}

void VT220SetupSetLineAttribute(VT220* vt, int attr)
{
	vt->setup.line_attributes[vt->setup.write_y] = attr;
}

void VT220SetupGoto(VT220* vt, const int line, const int column)
{
	vt->setup.write_x = column - 1;
	vt->setup.write_y = line - 1;
}

void VT220SetupCursorRight(VT220* vt)
{
	vt->setup.write_x++;
	if(vt->setup.write_x >= vt->columns) {
		vt->setup.write_x = vt->columns - 1;
	}
}

void VT220SetupCursorRightN(VT220* vt, int n)
{
	vt->setup.write_x += n;
	if(vt->setup.write_x >= vt->columns) {
		vt->setup.write_x = vt->columns - 1;
	}
}

void VT220SetupCursorSave(VT220* vt)
{
	vt->setup.write_x_save = vt->setup.write_x;
	vt->setup.write_y_save = vt->setup.write_y;
}

void VT220SetupCursorRestore(VT220* vt)
{
	vt->setup.write_x = vt->setup.write_x_save;
	vt->setup.write_y = vt->setup.write_y_save;
}

void VT220SetupWriteString(VT220* vt, const char* s, const int sgr)
{
	for(; *s; s++) {
		VT220SetupWrite(vt, (unsigned char) *s, sgr, 0);
	}
}

void VT220SetupFill(VT220* vt, const u16 c, unsigned int count, const int sgr)
{
	for(; count; count--) {
		VT220SetupWrite(vt, c, sgr, 0);
	}
}

void VT220SetupWriteNumber(VT220* vt, int val, const int width, const int align, const int sgr)
{
	char buf[8];
	int i, j;
	if(val == 0) {
		buf[7] = 0;
		i = 7;
	} else {
		for(i = 7; i > 0 && val > 0; i--) {
			buf[i] = val % 10;
			val /= 10;
		}
		i++;
	}
	j = (7 - i);

	if(align) {
		for(; j < width - 1; j++) {
			VT220SetupWrite(vt, ' ', sgr, 0);
		}
	}

	for(; i < 8; i++) {
		VT220SetupWrite(vt, buf[i] + '0', sgr, 0);
	}

	for(; j < width; j++) {
		VT220SetupWrite(vt, ' ', sgr, 0);
	}
}

void VT220iSetupWriteField(VT220* vt, const FIELD* field, const int sgr, const int pad)
{
	if(pad != 2) {
		VT220SetupWriteString(vt, " ", sgr);
	}
	size_t len = strlen(field->label);
	int fill = field->width - len;
	if(fill < 0) {
		fill = 0;
	}
	if(pad != 1) {
		fill++;
	}
	VT220SetupWriteString(vt, field->label, sgr);
	VT220SetupFill(vt, ' ', fill, sgr);
}

void VT220SetupWriteField(VT220* vt, const FIELD** table, const int id, const int sgr)
{
	VT220iSetupWriteField(vt, &table[vt->config.language][id], sgr, 0);
}

void VT220SetupWriteFieldNoRightPad(VT220* vt, const FIELD** table, const int id, const int sgr)
{
	VT220iSetupWriteField(vt, &table[vt->config.language][id], sgr, 1);
}

void VT220SetupWriteFieldNoLeftPad(VT220* vt, const FIELD** table, const int id, const int sgr)
{
	VT220iSetupWriteField(vt, &table[vt->config.language][id], sgr, 2);
}

int VT220SetupGetFieldWidth(VT220* vt, const FIELD** table, const int id)
{
	return table[vt->config.language][id].width;
}

void VT220SetupShowTitle(VT220* vt)
{
	const char* title = VT220SetupGetTitle(vt);

	VT220SetupGoto(vt, 1, 1);
	VT220SetupEraseLine(vt);
	VT220SetupSetLineAttribute(vt, DECDWL);
	VT220SetupWriteString(vt, title, SGR_BOLD | SGR_BLINKING);

	VT220SetupGoto(vt, 1, 31);
	VT220SetupWriteString(vt, "VT220 V2.3", SGR_UNDERSCORE);
}

void VT220SetupShowStatus(VT220* vt)
{
	VT220SetupGoto(vt, 7, 1);
	VT220SetupFill(vt, '-', 132, 0);
	VT220SetupGoto(vt, 8, 5);
	VT220SetupEraseLine(vt);
	if(vt->setup.in_enq >= 0) {
		switch(vt->config.language) {
			case VT220_LANGUAGE_ENGLISH:
				VT220SetupGoto(vt, 8, 15);
				VT220SetupWriteString(vt, "Enter Answerback=", 0);
				break;
			case VT220_LANGUAGE_FRANCAIS:
				VT220SetupGoto(vt, 8, 1);
				VT220SetupWriteString(vt, "Indiquez le Message de r\xE9ponse=", 0);
				break;
			case VT220_LANGUAGE_DEUTSCH:
				VT220SetupGoto(vt, 8, 9);
				VT220SetupWriteString(vt, "Bitte Antwort eingeben=", 0);
				break;
		}
		for(int i = 0; i < 30; i++) {
			if(vt->setup.enq[i]) {
				int bold = i == vt->setup.in_enq || (i == 29 && vt->setup.in_enq == 30);
				int sgr = bold ? (SGR_REVERSE | SGR_BOLD) : SGR_REVERSE;
				VT220SetupWrite(vt, (unsigned char) vt->setup.enq[i], sgr, 1);
			} else {
				for(; i < 30; i++) {
					int bold = i == vt->setup.in_enq || (i == 29 && vt->setup.in_enq == 30);
					int sgr = bold ? (SGR_REVERSE | SGR_BOLD) : SGR_REVERSE;
					VT220SetupWrite(vt, ' ', sgr, 1);
				}
				break;
			}
		}
	} else {
		if(vt->mode & IRM) {
			switch(vt->config.language) {
				default:
				case VT220_LANGUAGE_ENGLISH:
					VT220SetupWriteString(vt, "Insert Mode", 0);
					break;
				case VT220_LANGUAGE_FRANCAIS:
					VT220SetupWriteString(vt, "Mode d'insert", 0);
					break;
				case VT220_LANGUAGE_DEUTSCH:
					VT220SetupWriteString(vt, "Einf\xFCg-Modus", 0);
					break;
			}
		} else {
			switch(vt->config.language) {
				default:
				case VT220_LANGUAGE_ENGLISH:
					VT220SetupWriteString(vt, "Replace Mode", 0);
					break;
				case VT220_LANGUAGE_FRANCAIS:
					VT220SetupWriteString(vt, "Mode de remplacement", 0);
					break;
				case VT220_LANGUAGE_DEUTSCH:
					VT220SetupWriteString(vt, "Ersetz-Modus", 0);
					break;
			}
		}

		VT220SetupGoto(vt, 8, 28);
		switch(vt->config.language) {
			default:
			case VT220_LANGUAGE_ENGLISH:
				VT220SetupWriteString(vt, "Printer: ", 0);
				if(vt->print_rx) {
					if(vt->printer_controller) {
						VT220SetupWriteString(vt, "Controller", 0);
					} else if(vt->auto_print_mode) {
						VT220SetupWriteString(vt, "Auto", 0);
					} else {
						VT220SetupWriteString(vt, "Ready", 0);
					}
				} else {
					VT220SetupWriteString(vt, "None", 0);
				}
				break;
			case VT220_LANGUAGE_FRANCAIS:
				VT220SetupWriteString(vt, "Imprimante: ", 0);
				if(vt->print_rx) {
					if(vt->printer_controller) {
						VT220SetupWriteString(vt, "Contr\xF4leur", 0);
					} else if(vt->auto_print_mode) {
						VT220SetupWriteString(vt, "Auto", 0);
					} else {
						/* TODO: figure out the correct string here */
						VT220SetupWriteString(vt, "Ready", 0);
					}
				} else {
					VT220SetupWriteString(vt, "Aucune", 0);
				}
				break;
			case VT220_LANGUAGE_DEUTSCH:
				VT220SetupWriteString(vt, "Drucker: ", 0);
				if(vt->print_rx) {
					if(vt->printer_controller) {
						VT220SetupWriteString(vt, "Fern-Betrieb", 0);
					} else if(vt->auto_print_mode) {
						VT220SetupWriteString(vt, "Auto-Betrieb", 0);
					} else {
						VT220SetupWriteString(vt, "ok", 0);
					}
				} else {
					VT220SetupWriteString(vt, "keiner", 0);
				}
				break;
				break;
		}
	}
}

void VT220SetupShowHintAction(VT220* vt)
{
	VT220SetupGoto(vt, 8, 1);
	VT220SetupEraseLine(vt);
	switch(vt->config.language) {
		default:
		case VT220_LANGUAGE_ENGLISH:
			VT220SetupWriteString(vt, "Press ENTER to take this action - Press Cursor Keys to move", 0);
			break;
		case VT220_LANGUAGE_FRANCAIS:
			VT220SetupWriteString(vt, "<VALIDER> pour faire ce que vous avez choisi - <FLECHE> pour vous d\xE9placer.", 0);
			break;
		case VT220_LANGUAGE_DEUTSCH:
			VT220SetupWriteString(vt, "Zum Durchf\xFChren dieser Aktion EINGABE dr\xFC""cken - Weiter mit Pfeiltasten", 0);
			break;
	}
	VT220Bell(vt);
}

void VT220SetupShowHintValue(VT220* vt)
{
	VT220SetupGoto(vt, 8, 1);
	VT220SetupEraseLine(vt);
	switch(vt->config.language) {
		default:
		case VT220_LANGUAGE_ENGLISH:
			VT220SetupWriteString(vt, "Press ENTER to change this field - Press Cursor Keys to move", 0);
			break;
		case VT220_LANGUAGE_FRANCAIS:
			VT220SetupWriteString(vt, "<VALIDER> pour changer cette zone - <FLECHE> pour vous d\xE9placer.", 0);
			break;
		case VT220_LANGUAGE_DEUTSCH:
			VT220SetupWriteString(vt, "Zum \xC4ndern dieses Feldes EINGABE dr\xFC""cken - Weiter mit Pfeiltasten", 0);
			break;
	}
	VT220Bell(vt);
}

void VT220SetupShowHint(VT220* vt)
{
	switch(vt->setup.screen) {
		case SETUP_SCREEN_DIRECTORY:
			if((vt->setup.cursor_x == 0 && (vt->setup.cursor_y == 1 || vt->setup.cursor_y == 2))
					|| (vt->setup.cursor_x == 1 && vt->setup.cursor_y == 2)) {
				VT220SetupShowHintValue(vt);
			} else {
				VT220SetupShowHintAction(vt);
			}
			break;
		case SETUP_SCREEN_DISPLAY:
		case SETUP_SCREEN_GENERAL:
		case SETUP_SCREEN_COMM:
		case SETUP_SCREEN_PRINTER:
		case SETUP_SCREEN_KEYBOARD:
			if((vt->setup.cursor_y == 0 && vt->setup.cursor_x < 2)
					|| (vt->setup.cursor_y == 2 && vt->setup.cursor_x == 1)) {
				VT220SetupShowHintAction(vt);
			} else {
				VT220SetupShowHintValue(vt);
			}
			break;
		case SETUP_SCREEN_TAB:
			VT220SetupShowHintAction(vt);
			break;
	}
}

void VT220SetupShowDone(VT220* vt)
{
	VT220SetupGoto(vt, 8, 37);
	VT220SetupEraseLine(vt);
	switch(vt->config.language) {
		default:
		case VT220_LANGUAGE_ENGLISH:
			VT220SetupWriteString(vt, "Done", 0);
			break;
		case VT220_LANGUAGE_FRANCAIS:
			VT220SetupWriteString(vt, "Effectu\xE9", 0);
			break;
		case VT220_LANGUAGE_DEUTSCH:
			VT220SetupWriteString(vt, "AUSGEF\xDCHRT", 0);
			break;
	}
}

static inline int VT220SetupGetSGR(VT220* vt, int x, int y, int cursor_x, int cursor_y)
{
	if(x == cursor_x && y == cursor_y) {
		if(vt->mode & DECSCNM) {
			return SGR_BOLD;
		} else {
			return SGR_REVERSE | SGR_BOLD;
		}
	} else {
		return SGR_REVERSE;
	}
}

#define GET_SGR(y, x)	VT220SetupGetSGR(vt, x, y, vt->setup.cursor_x, vt->setup.cursor_y)

void VT220SetupGetKeyboardLanguage(VT220* vt, FIELD* field)
{
	switch(vt->config.language) {
		default:
		case VT220_LANGUAGE_ENGLISH:
			field->width = 25;
			field->label = vt220_keyboard_languages_english[(int) vt->config.keyboard];
			break;
		case VT220_LANGUAGE_FRANCAIS:
			field->width = 25;
			field->label = vt220_keyboard_languages_french[(int) vt->config.keyboard];
			break;
		case VT220_LANGUAGE_DEUTSCH:
			field->width = 32;
			field->label = vt220_keyboard_languages_german[(int) vt->config.keyboard];
			break;
	}
}

void VT220SetupShowDirectory(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			if(vt->setup.cursor_x > 5) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 1;
				} else {
					vt->setup.cursor_x = 5;
				}
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 5) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 2;
				} else {
					vt->setup.cursor_x = 5;
				}
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 5;
				vt->setup.cursor_y = 0;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 3) {
				vt->setup.cursor_x = 3;
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 5;
				vt->setup.cursor_y = 1;
			}
			break;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_DISPLAY, GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_GENERAL, GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_COMM, GET_SGR(0, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_PRINTER, GET_SGR(0, 3));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_KEYBOARD, GET_SGR(0, 4));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_TAB, GET_SGR(0, 5));

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->config.local) {
		VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_LOCAL, GET_SGR(1, 0));
	} else {
		VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_ON_LINE, GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_CLEAR_DISPLAY, GET_SGR(1, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_CLEAR_COMM, GET_SGR(1, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_RESET_TERMINAL, GET_SGR(1, 3));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_RECALL, GET_SGR(1, 4));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_SAVE, GET_SGR(1, 5));

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_SET_UP, GET_SGR(2, 0));
	VT220SetupCursorRight(vt);

	FIELD kblang;
	VT220SetupGetKeyboardLanguage(vt, &kblang);
	VT220iSetupWriteField(vt, &kblang, GET_SGR(2, 1), 0);

	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_DEFAULT, GET_SGR(2, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, setup_directory_field_names, DIRECTORY_EXIT, GET_SGR(2, 3));
}

void VT220SetupShowDisplay(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			if(vt->setup.cursor_x > 3) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 1;
				} else {
					vt->setup.cursor_x = 3;
				}
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 2) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 2;
				} else {
					vt->setup.cursor_x = 2;
				}
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 3;
				vt->setup.cursor_y = 0;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 1) {
				vt->setup.cursor_x = 1;
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 2;
				vt->setup.cursor_y = 1;
			}
			break;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, display_field_names, DISPLAY_TO_NEXT_SET_UP, GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, display_field_names, DISPLAY_TO_DIRECTORY, GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	if(vt->mode & DECCOLM) {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_132_COLUMNS, GET_SGR(0, 2));
	} else {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_80_COLUMNS, GET_SGR(0, 2));
	}
	VT220SetupCursorRight(vt);
	switch(vt->config.controls) {
		case VT220_CONTROLS_INTERPRET_CONTROLS:
			VT220SetupWriteField(vt, display_field_names, DISPLAY_INTERPRET_CONTROLS, GET_SGR(0, 3));
			break;
		case VT220_CONTROLS_DISPLAY_CONTROLS:
			VT220SetupWriteField(vt, display_field_names, DISPLAY_DISPLAY_CONTROLS, GET_SGR(0, 3));
			break;
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & DECAWM) {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_AUTO_WRAP, GET_SGR(1, 0));
	} else {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_NO_AUTO_WRAP, GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & DECSCLM) {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_SMOOTH_SCROLL, GET_SGR(1, 1));
	} else {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_JUMP_SCROLL, GET_SGR(1, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & DECSCNM) {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_DARK_TEXT_LIGHT_SCREEN, GET_SGR(1, 2));
	} else {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_LIGHT_TEXT_DARK_SCREEN, GET_SGR(1, 2));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & DECTCEM) {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_CURSOR, GET_SGR(2, 0));
	} else {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_NO_CURSOR, GET_SGR(2, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.cursor_style == VT220_CURSOR_STYLE_BLOCK_CURSOR) {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_BLOCK_CURSOR_STYLE, GET_SGR(2, 1));
	} else {
		VT220SetupWriteField(vt, display_field_names, DISPLAY_UNDERLINE_CURSOR_STYLE, GET_SGR(2, 1));
	}
}

void VT220SetupShowGeneral(VT220* vt)
{
	if(vt->vt100_mode) {
		if(vt->setup.cursor_y == 0 && vt->setup.cursor_x > 3) {
			if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
				vt->setup.cursor_x = 0;
				vt->setup.cursor_y = 1;
			} else {
				vt->setup.cursor_x = 3;
			}
		} else if(vt->setup.cursor_y > 0 && vt->setup.cursor_x > 2) {
			if(vt->setup.cursor_y < 2 && vt->setup.cursor_x > 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
				vt->setup.cursor_x = 0;
				vt->setup.cursor_y++;
			} else {
				vt->setup.cursor_x = 2;
			}
		} else if(vt->setup.cursor_y > 0 && vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
			vt->setup.cursor_y--;
			if(vt->setup.cursor_y == 0) {
				vt->setup.cursor_x = 3;
			} else {
				vt->setup.cursor_x = 2;
			}
		}
	} else if(vt->setup.cursor_x > 2) {
		if(vt->setup.cursor_y < 2 && vt->setup.cursor_x > 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y++;
		} else {
			vt->setup.cursor_x = 2;
		}
	} else if(vt->setup.cursor_y > 0 && vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
		vt->setup.cursor_x = 2;
		vt->setup.cursor_y--;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, general_field_names, GENERAL_TO_NEXT_SET_UP, GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, general_field_names, GENERAL_TO_DIRECTORY, GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	if(!(vt->mode & DECANM)) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_VT52_MODE, GET_SGR(0, 2));
	} else if(vt->vt100_mode) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_VT100_MODE, GET_SGR(0, 2));
	} else if(vt->ct_7bit) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_VT200_MODE_7BIT_CTRLS, GET_SGR(0, 2));
	} else {
		VT220SetupWriteField(vt, general_field_names, GENERAL_VT200_MODE_8BIT_CTRLS, GET_SGR(0, 2));
	}

	if(vt->vt100_mode) {
		VT220SetupCursorRight(vt);
		switch(vt->config.vt100_terminal_id) {
			case VT220_VT100_TERMINAL_ID_VT220:
				VT220SetupWriteField(vt, general_field_names, GENERAL_VT220_ID, GET_SGR(0, 3));
				break;
			case VT220_VT100_TERMINAL_ID_VT100:
				VT220SetupWriteField(vt, general_field_names, GENERAL_VT100_ID, GET_SGR(0, 3));
				break;
			case VT220_VT100_TERMINAL_ID_VT101:
				VT220SetupWriteField(vt, general_field_names, GENERAL_VT101_ID, GET_SGR(0, 3));
				break;
			case VT220_VT100_TERMINAL_ID_VT102:
				VT220SetupWriteField(vt, general_field_names, GENERAL_VT102_ID, GET_SGR(0, 3));
				break;
		}
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->udk_locked) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_UDK_LOCKED, GET_SGR(1, 0));
	} else {
		VT220SetupWriteField(vt, general_field_names, GENERAL_UDK_UNLOCKED, GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_FEATURES_UNLOCKED, GET_SGR(1, 1));
	} else {
		VT220SetupWriteField(vt, general_field_names, GENERAL_FEATURES_LOCKED, GET_SGR(1, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & DECNRCM) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_NATIONAL, GET_SGR(1, 2));
	} else {
		VT220SetupWriteField(vt, general_field_names, GENERAL_MULTINATIONAL, GET_SGR(1, 2));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & KAM) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_APPLICATION_KEYPAD, GET_SGR(2, 0));
	} else {
		VT220SetupWriteField(vt, general_field_names, GENERAL_NUMERIC_KEYPAD, GET_SGR(2, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & DECCKM) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_APPLICATION_CURSOR_KEYS, GET_SGR(2, 1));
	} else {
		VT220SetupWriteField(vt, general_field_names, GENERAL_NORMAL_CURSOR_KEYS, GET_SGR(2, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & LNM) {
		VT220SetupWriteField(vt, general_field_names, GENERAL_NEW_LINE, GET_SGR(2, 2));
	} else {
		VT220SetupWriteField(vt, general_field_names, GENERAL_NO_NEW_LINE, GET_SGR(2, 2));
	}
}

void VT220SetupShowComm(VT220* vt)
{
	if(vt->setup.cursor_x > 3) {
		if(vt->setup.cursor_y < 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y++;
		} else {
			vt->setup.cursor_x = 3;
		}
	} else if(vt->setup.cursor_y > 0  && vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
		vt->setup.cursor_x = 3;
		vt->setup.cursor_y--;
	}
	if(vt->setup.cursor_y == 2 && vt->setup.cursor_x > 2) {
		vt->setup.cursor_x = 2;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, comm_field_names, COMM_TO_NEXT_SET_UP, GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, comm_field_names, COMM_TO_DIRECTORY, GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteFieldNoRightPad(vt, comm_field_names, COMM_TRANSMIT, GET_SGR(0, 2));
	VT220SetupWriteNumber(vt, vt->config.tx_baud_rate, 6, 0, GET_SGR(0, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteFieldNoRightPad(vt, comm_field_names, COMM_RECEIVE, GET_SGR(0, 3));
	if(vt->config.rx_baud_rate == 0) {
		VT220SetupWriteFieldNoLeftPad(vt, comm_field_names, COMM_RECEIVE_TRANSMIT, GET_SGR(0, 3));
	} else {
		int width = VT220SetupGetFieldWidth(vt, comm_field_names, COMM_RECEIVE_TRANSMIT);
		VT220SetupWriteNumber(vt, vt->config.rx_baud_rate, width, 0, GET_SGR(0, 3));
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->use_xoff) {
		switch(vt->xoff_point) {
			case 64:
				VT220SetupWriteField(vt, comm_field_names, COMM_XOFF_AT_64, GET_SGR(1, 0));
				break;
			case 128:
				VT220SetupWriteField(vt, comm_field_names, COMM_XOFF_AT_128, GET_SGR(1, 0));
				break;
		}
	} else {
		VT220SetupWriteField(vt, comm_field_names, COMM_NO_XOFF, GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	switch(vt->config.format) {
		default:
		case VT220_COMM_8BIT_NO_PARITY:
			VT220SetupWriteField(vt, comm_field_names, COMM_8_BITS_NO_PARITY, GET_SGR(1, 1));
			break;
		case VT220_COMM_8BIT_EVEN_PARITY:
			VT220SetupWriteField(vt, comm_field_names, COMM_8_BITS_EVEN_PARITY, GET_SGR(1, 1));
			break;
		case VT220_COMM_8BIT_ODD_PARITY:
			VT220SetupWriteField(vt, comm_field_names, COMM_8_BITS_ODD_PARITY, GET_SGR(1, 1));
			break;
		case VT220_COMM_8BIT_EVEN_PARITY_NO_CHECK:
			VT220SetupWriteField(vt, comm_field_names, COMM_8_BITS_EVEN_PARITY_NO_CHK, GET_SGR(1, 1));
			break;
		case VT220_COMM_8BIT_ODD_PARITY_NO_CHECK:
			VT220SetupWriteField(vt, comm_field_names, COMM_8_BITS_ODD_PARITY_NO_CHK, GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_EVEN_PARITY_NO_CHECK:
			VT220SetupWriteField(vt, comm_field_names, COMM_7_BITS_EVEN_PARITY_NO_CHK, GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_ODD_PARITY_NO_CHECK:
			VT220SetupWriteField(vt, comm_field_names, COMM_7_BITS_ODD_PARITY_NO_CHK, GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_NO_PARITY:
			VT220SetupWriteField(vt, comm_field_names, COMM_7_BITS_NO_PARITY, GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_EVEN_PARITY:
			VT220SetupWriteField(vt, comm_field_names, COMM_7_BITS_EVEN_PARITY, GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_ODD_PARITY:
			VT220SetupWriteField(vt, comm_field_names, COMM_7_BITS_ODD_PARITY, GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_MARK_PARITY:
			VT220SetupWriteField(vt, comm_field_names, COMM_7_BITS_MARK_PARITY, GET_SGR(1, 1));
			break;
		case VT220_COMM_7BIT_SPACE_PARITY:
			VT220SetupWriteField(vt, comm_field_names, COMM_7_BITS_SPACE_PARITY, GET_SGR(1, 1));
			break;
	}
	VT220SetupCursorRight(vt);
	if(vt->config.stop_bits == VT220_COMM_1_STOP_BIT) {
		VT220SetupWriteField(vt, comm_field_names, COMM_1_STOP_BIT, GET_SGR(1, 2));
	} else {
		VT220SetupWriteField(vt, comm_field_names, COMM_2_STOP_BITS, GET_SGR(1, 2));
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & SRM) {
		VT220SetupWriteField(vt, comm_field_names, COMM_NO_LOCAL_ECHO, GET_SGR(1, 3));
	} else {
		VT220SetupWriteField(vt, comm_field_names, COMM_LOCAL_ECHO, GET_SGR(1, 3));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	switch(vt->config.port) {
		default:
		case VT220_COMM_EIA_PORT_DATA_LEADS_ONLY:
			VT220SetupWriteField(vt, comm_field_names, COMM_EIA_PORT_DATA_LEADS_ONLY, GET_SGR(2, 0));
			break;
		case VT220_COMM_EIA_PORT_MODEM_CONTROL:
			VT220SetupWriteField(vt, comm_field_names, COMM_EIA_PORT_MODEM_CONTROL, GET_SGR(2, 0));
			break;
		case VT220_COMM_20MA_PORT:
			VT220SetupWriteField(vt, comm_field_names, COMM_20_MA_PORT, GET_SGR(2, 0));
			break;
	}
	VT220SetupCursorRight(vt);
	if(vt->config.delay == VT220_COMM_2S_DELAY) {
		VT220SetupWriteField(vt, comm_field_names, COMM_DISCONNECT_2_S_DELAY, GET_SGR(2, 1));
	} else {
		VT220SetupWriteField(vt, comm_field_names, COMM_DISCONNECT_60_MS_DELAY, GET_SGR(2, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.transmit == VT220_TRANSMIT_LIMITED) {
		VT220SetupWriteField(vt, comm_field_names, COMM_LIMITED_TRANSMIT, GET_SGR(2, 2));
	} else {
		VT220SetupWriteField(vt, comm_field_names, COMM_UNLIMITED_TRANSMIT, GET_SGR(2, 2));
	}
}

void VT220SetupShowPrinter(VT220* vt)
{
	if(vt->setup.cursor_x > 2) {
		if(vt->setup.cursor_y < 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y++;
		} else {
			vt->setup.cursor_x = 2;
		}
	} else if(vt->setup.cursor_y > 0 && vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
		vt->setup.cursor_x = 2;
		vt->setup.cursor_y--;
	}
	if((vt->setup.cursor_y == 0 || vt->setup.cursor_y == 2) && vt->setup.cursor_x > 2) {
		if(vt->setup.cursor_y < 2 && vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y = 1;
		} else {
			vt->setup.cursor_x = 2;
		}
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, printer_field_names, PRINTER_TO_NEXT_SET_UP, GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, printer_field_names, PRINTER_TO_DIRECTORY, GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteFieldNoRightPad(vt, printer_field_names, PRINTER_SPEED, GET_SGR(0, 2));
	VT220SetupWriteNumber(vt, 4800, 5, 0, GET_SGR(0, 2)); /* TODO */

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->printer_controller) {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_CONTROLLER_MODE, GET_SGR(1, 0));
	} else if(vt->auto_print_mode) {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_AUTO_PRINT_MODE, GET_SGR(1, 0));
	} else {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_NORMAL_PRINT_MODE, GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	switch(vt->config.printer_format) {
		default:
		case VT220_PRINTER_FORMAT_8BIT_NO_PARITY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_8_BITS_NO_PARITY, GET_SGR(1, 1));
			break;
		case VT220_PRINTER_FORMAT_8BIT_EVEN_PARITY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_8_BITS_EVEN_PARITY, GET_SGR(1, 1));
			break;
		case VT220_PRINTER_FORMAT_8BIT_ODD_PARITY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_8_BITS_ODD_PARITY, GET_SGR(1, 1));
			break;
		case VT220_PRINTER_FORMAT_7BIT_NO_PARITY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_7_BITS_NO_PARITY, GET_SGR(1, 1));
			break;
		case VT220_PRINTER_FORMAT_7BIT_EVEN_PARITY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_7_BITS_EVEN_PARITY, GET_SGR(1, 1));
			break;
		case VT220_PRINTER_FORMAT_7BIT_ODD_PARITY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_7_BITS_ODD_PARITY, GET_SGR(1, 1));
			break;
		case VT220_PRINTER_FORMAT_7BIT_MARK_PARITY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_7_BITS_MARK_PARITY, GET_SGR(1, 1));
			break;
		case VT220_PRINTER_FORMAT_7BIT_SPACE_PARITY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_7_BITS_SPACE_PARITY, GET_SGR(1, 1));
			break;
	}
	VT220SetupCursorRight(vt);
	if(vt->config.printer_stop_bits == VT220_PRINTER_2_STOP_BITS) {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_2_STOP_BITS, GET_SGR(1, 2));
	} else {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_1_STOP_BIT, GET_SGR(1, 2));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & DECPEX) {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_PRINT_FULL_PAGE, GET_SGR(2, 0));
	} else {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_PRINT_SCROLL_REGION, GET_SGR(2, 0));
	}
	VT220SetupCursorRight(vt);
	switch(vt->config.printer_data_type) {
		default:
		case VT220_PRINTER_NATIONAL_ONLY:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_PRINT_NATIONAL_ONLY, GET_SGR(2, 1));
			break;
		case VT220_PRINTER_NATIONAL_LINE_DRAWING:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_NATIONAL_AND_LINE_DRAW, GET_SGR(2, 1));
			break;
		case VT220_PRINTER_MULTINATIONAL:
			VT220SetupWriteField(vt, printer_field_names, PRINTER_PRINT_MULTINATIONAL, GET_SGR(2, 1));
			break;
	}
	VT220SetupCursorRight(vt);
	if(vt->mode & DECPFF) {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_TERMINATOR_FF, GET_SGR(2, 2));
	} else {
		VT220SetupWriteField(vt, printer_field_names, PRINTER_NO_TERMINATOR, GET_SGR(2, 2));
	}
}

void VT220SetupShowKeyboard(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			if(vt->setup.cursor_x > 3) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 1;
				} else {
					vt->setup.cursor_x = 3;
				}
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 4) {
				if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
					vt->setup.cursor_x = 0;
					vt->setup.cursor_y = 2;
				} else {
					vt->setup.cursor_x = 4;
				}
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 3;
				vt->setup.cursor_y = 0;
			}
			break;
		case 2:
			if(vt->setup.cursor_x > 2) {
				vt->setup.cursor_x = 2;
			} else if(vt->setup.move == VT220_SETUP_MOVE_LEFT_MARGIN) {
				vt->setup.cursor_x = 4;
				vt->setup.cursor_y = 1;
			}
			break;
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_TO_NEXT_SET_UP, GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_TO_DIRECTORY, GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	if(vt->config.keys == VT220_KEYS_TYPEWRITER) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_TYPEWRITER_KEYS, GET_SGR(0, 2));
	} else {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_DATA_PROCESSING_KEYS, GET_SGR(0, 2));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.lock == VT220_LOCK_CAPS_LOCK) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_CAPS_LOCK, GET_SGR(0, 3));
	} else {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_SHIFT_LOCK, GET_SGR(0, 3));
	}

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	if(vt->mode & DECARM) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_AUTO_REPEAT, GET_SGR(1, 0));
	} else {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_NO_AUTO_REPEAT, GET_SGR(1, 0));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.keyclick == VT220_KEYCLICK) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_KEYCLICK, GET_SGR(1, 1));
	} else {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_NO_KEYCLICK, GET_SGR(1, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.margin_bell == VT220_MARGIN_BELL) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_MARGIN_BELL, GET_SGR(1, 2));
	} else {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_NO_MARGIN_BELL, GET_SGR(1, 2));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.bell == VT220_BELL) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_WARNING_BELL, GET_SGR(1, 3));
	} else {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_NO_WARNING_BELL, GET_SGR(1, 3));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.brk == VT220_BREAK) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_BREAK, GET_SGR(1, 4));
	} else {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_NO_BREAK, GET_SGR(1, 4));
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_NO_AUTO_ANSWERBACK, GET_SGR(2, 0));
	VT220SetupCursorRight(vt);
	if(vt->config.concealed == VT220_ANSWERBACK_CONCEALED) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_ANSWERBACK_CONCEALED, GET_SGR(2, 1));
	} else {
		VT220SetupWriteFieldNoRightPad(vt, keyboard_field_names, KEYBOARD_ANSWERBACK, GET_SGR(2, 1));
		for(unsigned int i = 0; i < 30; i++) {
			if(vt->answerback[i]) {
				VT220SetupWrite(vt, (unsigned char) vt->answerback[i], GET_SGR(2, 1), 1);
			} else {
				for(; i < 30; i++) {
					VT220SetupWrite(vt, ' ', GET_SGR(2, 1), 1);
				}
				break;
			}
		}
		VT220SetupWriteString(vt, " ", GET_SGR(2, 1));
	}
	VT220SetupCursorRight(vt);
	if(vt->config.concealed == VT220_ANSWERBACK_CONCEALED) {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_CONCEALED, GET_SGR(2, 2));
	} else {
		VT220SetupWriteField(vt, keyboard_field_names, KEYBOARD_NOT_CONCEALED, GET_SGR(2, 2));
	}
}

void VT220SetupShowTab(VT220* vt)
{
	int i;

	if(vt->setup.cursor_y == 0) {
		if(vt->setup.move == VT220_SETUP_MOVE_UP) {
			vt->setup.cursor_x = 0;
		} else if(vt->setup.cursor_x > 3) {
			if(vt->setup.move == VT220_SETUP_MOVE_RIGHT) {
				vt->setup.cursor_x = 0;
				vt->setup.cursor_y = 1;
			} else {
				vt->setup.cursor_x = 3;
			}
		}
	} else if(vt->setup.move != VT220_SETUP_MOVE_LEFT_MARGIN) {
		if(vt->setup.move == VT220_SETUP_MOVE_DOWN) {
			if(vt->setup.cursor_y == 1) {
				vt->setup.cursor_x = 0;
			} else {
				vt->setup.cursor_y = 1;
			}
		} else if(vt->setup.move == VT220_SETUP_MOVE_UP) {
			vt->setup.cursor_x = 0;
			vt->setup.cursor_y = 0;
		} else {
			vt->setup.cursor_y = 1;
			if(vt->setup.cursor_x >= vt->columns) {
				vt->setup.cursor_x = vt->columns - 1;
			}
		}
	}

	/* line 1 */
	VT220SetupGoto(vt, 2, 1);
	VT220SetupEraseLine(vt);
	VT220SetupWriteField(vt, tab_field_names, TAB_TO_NEXT_SET_UP, GET_SGR(0, 0));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, tab_field_names, TAB_TO_DIRECTORY, GET_SGR(0, 1));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, tab_field_names, TAB_CLEAR_ALL_TABS, GET_SGR(0, 2));
	VT220SetupCursorRight(vt);
	VT220SetupWriteField(vt, tab_field_names, TAB_SET_8_COLUMN_TABS, GET_SGR(0, 3));

	/* line 2 */
	VT220SetupGoto(vt, 4, 1);
	VT220SetupEraseLine(vt);
	for(i = 0; i < vt->columns; i++) {
		VT220SetupGoto(vt, 4, i + 1);
		if(i > 0 && vt->tabstops[i - 1]) {
			VT220SetupWriteString(vt, "T", GET_SGR(1, i));
		} else {
			VT220SetupWriteString(vt, " ", GET_SGR(1, i));
		}
		VT220SetupGoto(vt, 5, i + 1);
		VT220SetupWrite(vt, '0' + (i + 1) % 10, ((i / 10) % 2) ? SGR_REVERSE : 0, 0);
	}

	/* line 3 */
	VT220SetupGoto(vt, 6, 1);
	VT220SetupEraseLine(vt);
}

void VT220SetupShowScreen(VT220* vt)
{
	if(vt->setup.cursor_y < 0) {
		vt->setup.cursor_y = 0;
	}
	if(vt->setup.cursor_y > 2) {
		vt->setup.cursor_y = 2;
	}

	VT220SetupShowTitle(vt);

	/* clear extra stuff from tab setup screen */
	VT220SetupGoto(vt, 5, 1);
	VT220SetupEraseLine(vt);

	switch(vt->setup.screen) {
		case SETUP_SCREEN_DIRECTORY:
			VT220SetupShowDirectory(vt);
			break;
		case SETUP_SCREEN_DISPLAY:
			VT220SetupShowDisplay(vt);
			break;
		case SETUP_SCREEN_GENERAL:
			VT220SetupShowGeneral(vt);
			break;
		case SETUP_SCREEN_COMM:
			VT220SetupShowComm(vt);
			break;
		case SETUP_SCREEN_PRINTER:
			VT220SetupShowPrinter(vt);
			break;
		case SETUP_SCREEN_KEYBOARD:
			VT220SetupShowKeyboard(vt);
			break;
		case SETUP_SCREEN_TAB:
			VT220SetupShowTab(vt);
			break;
	}

	vt->setup.move = VT220_SETUP_MOVE_NONE;
}

void VT220SetupShow(VT220* vt)
{
	vt->setup.move = VT220_SETUP_MOVE_NONE;
	VT220SetupEraseDisplay(vt);
	VT220SetupShowScreen(vt);
	VT220SetupShowStatus(vt);
}

void VT220SetupSetScreen(VT220* vt, int screen)
{
	vt->setup.screen = screen;
	vt->setup.cursor_x = 0;
	vt->setup.cursor_y = 0;
	VT220SetupShowScreen(vt);
}

void VT220SetupNextScreen(VT220* vt)
{
	vt->setup.screen++;
	vt->setup.screen %= SETUP_SCREEN_COUNT;
	if(vt->setup.screen == 0) {
		vt->setup.screen++;
	}
	vt->setup.cursor_x = 0;
	vt->setup.cursor_y = 0;
	VT220SetupShowScreen(vt);
}

void VT220EnterSetup(VT220* vt)
{
	vt->in_setup = 1;
	vt->setup.in_enq = -1;
	vt->setup.cursor_x = 0;
	vt->setup.cursor_y = 0;
	vt->setup.screen = SETUP_SCREEN_DIRECTORY;
	VT220FlowControl(vt, 0);

	VT220SetupShow(vt);
}

void VT220LeaveSetup(VT220* vt)
{
	vt->in_setup = 0;
	if(!vt->hold_screen) {
		VT220FlowControl(vt, 1);
	}
}

void VT220SetupDirectoryEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0: /* Display */
					VT220SetupSetScreen(vt, SETUP_SCREEN_DISPLAY);
					break;
				case 1: /* General */
					VT220SetupSetScreen(vt, SETUP_SCREEN_GENERAL);
					break;
				case 2: /* Comm */
					VT220SetupSetScreen(vt, SETUP_SCREEN_COMM);
					break;
				case 3: /* Printer */
					VT220SetupSetScreen(vt, SETUP_SCREEN_PRINTER);
					break;
				case 4: /* Keyboard */
					VT220SetupSetScreen(vt, SETUP_SCREEN_KEYBOARD);
					break;
				case 5: /* Tab */
					VT220SetupSetScreen(vt, SETUP_SCREEN_TAB);
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0: /* On Line/Local */
					vt->config.local = !vt->config.local;
					VT220SetupShowScreen(vt);
					break;
				case 1: /* Clear Display */
					VT220EraseInDisplay(vt, 2);
					VT220SetCursor(vt, 1, 1);
					VT220SetupShowDone(vt);
					break;
				case 2:
					/* Clear Comm */
					VT220ClearComm(vt);
					VT220SetupShowDone(vt);
					break;
				case 3: /* Reset */
					VT220SoftReset(vt);
					VT220SetupShowDone(vt);
					break;
				case 4: /* Recall */
					VT220HardReset(vt);
					VT220SetupShow(vt);
					VT220SetupShowDone(vt);
					break;
				case 5: /* Save */
					VT220SaveConfig(vt);
					VT220SetupShowDone(vt);
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 0: /* Set-Up language */
					vt->config.language = (vt->config.language + 1) % VT220_LANGUAGE_COUNT;
					VT220SetupShow(vt);
					break;
				case 1: /* Keyboard language */
					vt->config.keyboard = (vt->config.keyboard + 1) % VT220_KEYBOARD_COUNT;
					VT220SetupShow(vt);
					break;
				case 2: /* Defaults */
					VT220LoadDefaults(vt);
					VT220SetupShow(vt);
					VT220SetupShowDone(vt);
					break;
				case 3: /* Exit */
					VT220LeaveSetup(vt);
					break;
			}
	}
}

void VT220SetupDisplayEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					if(vt->mode & DECCOLM) {
						VT220ClearColumnMode(vt);
					} else {
						VT220SetColumnMode(vt);
					}
					VT220SetupShow(vt);
					break;
				case 3:
					switch(vt->config.controls) {
						case VT220_CONTROLS_INTERPRET_CONTROLS:
							vt->config.controls = VT220_CONTROLS_DISPLAY_CONTROLS;
							break;
						case VT220_CONTROLS_DISPLAY_CONTROLS:
							vt->config.controls = VT220_CONTROLS_INTERPRET_CONTROLS;
							break;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0: /* auto wrap mode */
					vt->mode ^= DECAWM;
					VT220SetupShowScreen(vt);
					break;
				case 1: /* scroll mode */
					vt->mode ^= DECSCLM;
					VT220SetupShowScreen(vt);
					break;
				case 2: /* inverse display */
					vt->mode ^= DECSCNM;
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 0: /* text cursor */
					vt->mode ^= DECTCEM;
					VT220SetupShowScreen(vt);
					break;
				case 1: /* cursor style */
					if(vt->config.cursor_style == VT220_CURSOR_STYLE_BLOCK_CURSOR) {
						vt->config.cursor_style = VT220_CURSOR_STYLE_UNDERLINE_CURSOR;
					} else {
						vt->config.cursor_style = VT220_CURSOR_STYLE_BLOCK_CURSOR;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
	}
}

void VT220SetupGeneralEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					if(!(vt->mode & DECANM)) {
						/* was VT52, go to VT100 */
						vt->mode |= DECANM;
						vt->vt100_mode = 1;
						vt->ct_7bit = 1;
					} else if(vt->vt100_mode) {
						/* was VT100, go to VT200, 7bit */
						vt->mode |= DECANM;
						vt->ct_7bit = 1;
						vt->vt100_mode = 0;
					} else if(vt->ct_7bit) {
						/* was VT200, 7bit, go to VT200, 8bit */
						vt->ct_7bit = 0;
					} else {
						/* was VT200, 8bit, go to VT52 */
						vt->mode &= ~DECANM;
					}
					break;
				case 3:
					switch(vt->config.vt100_terminal_id) {
						case VT220_VT100_TERMINAL_ID_VT220:
							vt->config.vt100_terminal_id = VT220_VT100_TERMINAL_ID_VT100;
							break;
						case VT220_VT100_TERMINAL_ID_VT100:
							vt->config.vt100_terminal_id = VT220_VT100_TERMINAL_ID_VT101;
							break;
						case VT220_VT100_TERMINAL_ID_VT101:
							vt->config.vt100_terminal_id = VT220_VT100_TERMINAL_ID_VT102;
							break;
						case VT220_VT100_TERMINAL_ID_VT102:
							vt->config.vt100_terminal_id = VT220_VT100_TERMINAL_ID_VT220;
							break;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0:
					vt->udk_locked = !vt->udk_locked;
					VT220SetupShowScreen(vt);
					break;
				case 1:
					if(vt->config.user_features == VT220_USER_FEATURES_UNLOCKED) {
						vt->config.user_features = VT220_USER_FEATURES_LOCKED;
					} else {
						vt->config.user_features = VT220_USER_FEATURES_UNLOCKED;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 0: /* Keypad */
					vt->mode ^= KAM;
					VT220SetupShowScreen(vt);
					break;
				case 1: /* Cursor keys */
					vt->mode ^= DECCKM;
					VT220SetupShowScreen(vt);
					break;
				case 2: /* New line */
					vt->mode ^= LNM;
					VT220SetupShowScreen(vt);
					break;
			}
			break;
	}
}

void VT220SetupCommEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					vt->config.tx_baud_rate = VT220GetNextBaudRate(vt->config.tx_baud_rate, 0);
					VT220iUpdateBaudRate(vt);
					break;
				case 3:
					vt->config.rx_baud_rate = VT220GetNextBaudRate(vt->config.rx_baud_rate, 1);
					VT220iUpdateBaudRate(vt);
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0:
					switch(vt->xoff_point) {
						case 64:
							vt->xoff_point = 128;
							break;
						case 128:
							vt->use_xoff = 0;
							vt->xoff_point = 0;
							VT220iUpdateFlowControl(vt);
							break;
						case 0:
							vt->use_xoff = 1;
							vt->xoff_point = 64;
							VT220iUpdateFlowControl(vt);
							break;
					}
					VT220SetupShowScreen(vt);
					break;
				case 1:
					vt->config.format = (vt->config.format + 1) % 12;
					VT220iUpdateFormat(vt);
					break;
				case 2:
					if(vt->config.stop_bits == VT220_COMM_1_STOP_BIT) {
						vt->config.stop_bits = VT220_COMM_2_STOP_BITS;
					} else {
						vt->config.stop_bits = VT220_COMM_1_STOP_BIT;
					}
					VT220iUpdateFormat(vt);
					break;
				case 3:
					vt->mode ^= SRM;
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 0:
					switch(vt->config.port) {
						case VT220_COMM_EIA_PORT_DATA_LEADS_ONLY:
							vt->config.port = VT220_COMM_EIA_PORT_MODEM_CONTROL;
							break;
						case VT220_COMM_EIA_PORT_MODEM_CONTROL:
							vt->config.port = VT220_COMM_20MA_PORT;
							break;
						default:
						case VT220_COMM_20MA_PORT:
							vt->config.port = VT220_COMM_EIA_PORT_DATA_LEADS_ONLY;
							break;
					}
					break;
				case 1:
					if(vt->config.delay == VT220_COMM_2S_DELAY) {
						vt->config.delay = VT220_COMM_60MS_DELAY;
					} else {
						vt->config.delay = VT220_COMM_2S_DELAY;
					}
					break;
				case 2:
					if(vt->config.transmit == VT220_TRANSMIT_LIMITED) {
						vt->config.transmit = VT220_TRANSMIT_UNLIMITED;
					} else {
						vt->config.transmit = VT220_TRANSMIT_LIMITED;
					}
					break;
			}
			break;
	}
}

void VT220SetupPrinterEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					/* printer speed */
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0:
					if(vt->printer_controller) {
						vt->printer_controller = 0;
						vt->auto_print_mode = 0;
					} else if(vt->auto_print_mode) {
						vt->printer_controller = 1;
						vt->auto_print_mode = 0;
					} else {
						vt->printer_controller = 0;
						vt->auto_print_mode = 1;
					}
					break;
				case 1:
					vt->config.printer_format = (vt->config.printer_format + 1) % 8;
					break;
				case 2:
					if(vt->config.printer_stop_bits == VT220_PRINTER_1_STOP_BIT) {
						vt->config.printer_stop_bits = VT220_PRINTER_2_STOP_BITS;
					} else {
						vt->config.printer_stop_bits = VT220_PRINTER_1_STOP_BIT;
					}
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 0:
					vt->mode ^= DECPEX;
					break;
				case 1:
					switch(vt->config.printer_data_type) {
						case VT220_PRINTER_NATIONAL_ONLY:
							vt->config.printer_data_type = VT220_PRINTER_NATIONAL_LINE_DRAWING;
							break;
						case VT220_PRINTER_NATIONAL_LINE_DRAWING:
							vt->config.printer_data_type = VT220_PRINTER_MULTINATIONAL;
							break;
						default:
						case VT220_PRINTER_MULTINATIONAL:
							vt->config.printer_data_type = VT220_PRINTER_NATIONAL_ONLY;
							break;
					}
					break;
				case 2:
					vt->mode ^= DECPFF;
					break;
			}
	}
}

void VT220SetupKeyboardEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					if(vt->config.keys == VT220_KEYS_TYPEWRITER) {
						vt->config.keys = VT220_KEYS_DATA_PROCESSING;
					} else {
						vt->config.keys = VT220_KEYS_TYPEWRITER;
					}
					break;
				case 3:
					if(vt->config.lock == VT220_LOCK_CAPS_LOCK) {
						vt->config.lock = VT220_LOCK_SHIFT_LOCK;
					} else {
						vt->config.lock = VT220_LOCK_CAPS_LOCK;
					}
					break;
			}
			break;
		case 1:
			switch(vt->setup.cursor_x) {
				case 0:
					vt->mode ^= DECARM;
					VT220SetupShowScreen(vt);
					break;
				case 1:
					if(vt->config.keyclick == VT220_KEYCLICK) {
						vt->config.keyclick = VT220_NO_KEYCLICK;
					} else {
						vt->config.keyclick = VT220_KEYCLICK;
					}
					break;
				case 2:
					if(vt->config.margin_bell == VT220_MARGIN_BELL) {
						vt->config.margin_bell = VT220_NO_MARGIN_BELL;
					} else {
						vt->config.margin_bell = VT220_MARGIN_BELL;
					}
					break;
				case 3:
					if(vt->config.bell == VT220_BELL) {
						vt->config.bell = VT220_NO_BELL;
					} else {
						vt->config.bell = VT220_BELL;
					}
					VT220Bell(vt);
					break;
				case 4:
					if(vt->config.brk == VT220_BREAK) {
						vt->config.brk = VT220_NO_BREAK;
					} else {
						vt->config.brk = VT220_BREAK;
					}
					break;
			}
			break;
		case 2:
			switch(vt->setup.cursor_x) {
				case 1:
					if(vt->setup.in_enq >= 0) {
						/* commit */
						memcpy(vt->answerback, vt->setup.enq, 30);
						vt->setup.in_enq = -1;
						vt->config.concealed = VT220_ANSWERBACK_NOT_CONCEALED;
					} else {
						if(vt->config.concealed == VT220_ANSWERBACK_CONCEALED) {
							memset(vt->setup.enq, 0, 30);
						} else {
							memcpy(vt->setup.enq, vt->answerback, 30);
						}

						/* set edit cursor to end of answerback message */
						for(vt->setup.in_enq = 0; vt->setup.in_enq < 30; vt->setup.in_enq++) {
							if(!vt->setup.enq[vt->setup.in_enq]) {
								break;
							}
						}
					}
					VT220SetupShowStatus(vt);
					break;
				case 2:
					vt->config.concealed = VT220_ANSWERBACK_CONCEALED;
					break;
			}
			break;
	}
}

void VT220SetupTabEnter(VT220* vt)
{
	switch(vt->setup.cursor_y) {
		case 0:
			switch(vt->setup.cursor_x) {
				case 0:
					VT220SetupNextScreen(vt);
					break;
				case 1:
					VT220SetupSetScreen(vt, SETUP_SCREEN_DIRECTORY);
					break;
				case 2:
					VT220ClearAllTabstops(vt);
					VT220SetupShowScreen(vt);
					break;
				case 3:
					for(int i = 0; i < vt->columns; i++) {
						vt->tabstops[i] = i % 8 == 7;
					}
					VT220SetupShowScreen(vt);
					break;
			}
			break;
		case 1:
			if(vt->setup.cursor_x > 0) {
				vt->tabstops[vt->setup.cursor_x - 1] = !vt->tabstops[vt->setup.cursor_x - 1];
			}
			VT220SetupShowScreen(vt);
			break;
	}
}

void VT220SetupProcessEnter(VT220* vt)
{
	switch(vt->setup.screen) {
		case SETUP_SCREEN_DIRECTORY:
			VT220SetupDirectoryEnter(vt);
			break;
		case SETUP_SCREEN_DISPLAY:
			VT220SetupDisplayEnter(vt);
			break;
		case SETUP_SCREEN_GENERAL:
			VT220SetupGeneralEnter(vt);
			break;
		case SETUP_SCREEN_COMM:
			VT220SetupCommEnter(vt);
			break;
		case SETUP_SCREEN_PRINTER:
			VT220SetupPrinterEnter(vt);
			break;
		case SETUP_SCREEN_KEYBOARD:
			VT220SetupKeyboardEnter(vt);
			break;
		case SETUP_SCREEN_TAB:
			VT220SetupTabEnter(vt);
			break;
	}
}

unsigned int VT220SetupEncodeAnswerback(u16 key, unsigned char* buf)
{
	switch(key) {
		case VT220_KEY_FIND:
			buf[0] = CSI;
			buf[1] = '1';
			buf[2] = '~';
			return 3;
		case VT220_KEY_INSERT:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '~';
			return 3;
		case VT220_KEY_REMOVE:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '~';
			return 3;
		case VT220_KEY_SELECT:
			buf[0] = CSI;
			buf[1] = '4';
			buf[2] = '~';
			return 3;
		case VT220_KEY_PREV_SCREEN:
			buf[0] = CSI;
			buf[1] = '5';
			buf[2] = '~';
			return 3;
		case VT220_KEY_NEXT_SCREEN:
			buf[0] = CSI;
			buf[1] = '6';
			buf[2] = '~';
			return 3;
		case VT220_KEY_F6:
			buf[0] = CSI;
			buf[1] = '1';
			buf[2] = '7';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F7:
			buf[0] = CSI;
			buf[1] = '1';
			buf[2] = '8';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F8:
			buf[0] = CSI;
			buf[1] = '1';
			buf[2] = '9';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F9:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '0';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F10:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '1';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F11:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '3';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F12:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '4';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F13:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '5';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F14:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '6';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F15:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '8';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F16:
			buf[0] = CSI;
			buf[1] = '2';
			buf[2] = '9';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F17:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '1';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F18:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '2';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F19:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '3';
			buf[3] = '~';
			return 4;
		case VT220_KEY_F20:
			buf[0] = CSI;
			buf[1] = '3';
			buf[2] = '4';
			buf[3] = '~';
			return 4;
		case VT220_KEY_KP_0:
			*buf = '0';
			return 1;
		case VT220_KEY_KP_1:
			*buf = '1';
			return 1;
		case VT220_KEY_KP_2:
			*buf = '2';
			return 1;
		case VT220_KEY_KP_3:
			*buf = '3';
			return 1;
		case VT220_KEY_KP_4:
			*buf = '4';
			return 1;
		case VT220_KEY_KP_5:
			*buf = '5';
			return 1;
		case VT220_KEY_KP_6:
			*buf = '6';
			return 1;
		case VT220_KEY_KP_7:
			*buf = '7';
			return 1;
		case VT220_KEY_KP_8:
			*buf = '8';
			return 1;
		case VT220_KEY_KP_9:
			*buf = '9';
			return 1;
		case VT220_KEY_KP_MINUS:
			*buf = '-';
			return 1;
		case VT220_KEY_KP_COMMA:
			*buf = ',';
			return 1;
		case VT220_KEY_KP_PERIOD:
			*buf = '.';
			return 1;
		case VT220_KEY_KP_PF1:
			buf[0] = SS3;
			buf[1] = 'P';
			return 2;
		case VT220_KEY_KP_PF2:
			buf[0] = SS3;
			buf[1] = 'Q';
			return 2;
		case VT220_KEY_KP_PF3:
			buf[0] = SS3;
			buf[1] = 'R';
			return 2;
		case VT220_KEY_KP_PF4:
			buf[0] = SS3;
			buf[1] = 'S';
			return 2;
	}
	return 0;
}

void VT220SetupProcessAnswerback(VT220* vt, u16 key)
{
	if(key == DEL) {
		if(vt->setup.in_enq > 0) {
			vt->setup.in_enq--;
			vt->setup.enq[vt->setup.in_enq] = 0;
		} else {
			/* DEL in empty answerback = exit the field */
			vt->setup.in_enq = -1;
		}
	} else {
		if(vt->setup.in_enq <= 30) {
			unsigned int cursor = vt->setup.in_enq;
			if(cursor >= 30) {
				cursor = 29;
			}

			/* The real VT220 encodes special keys as 8-bit control
			 * sequences using CSI (9B) or SS3 (8F). If the encoded
			 * sequence does not fit into the answerback string
			 * anymore, the error bell sounds and the end of the
			 * sequence is truncated. */

			if(key > 0xFF) {
				unsigned char buf[4]; /* the longest sequence is 4 characters */
				unsigned int len = VT220SetupEncodeAnswerback(key, buf);
				if(vt->setup.in_enq + len > 30) {
					if(vt->setup.in_enq == 30) {
						/* only copy the first character */
						vt->setup.enq[29] = buf[0];
					} else {
						/* truncate string */
						len = 30 - vt->setup.in_enq;
						memcpy(&vt->setup.enq[vt->setup.in_enq], buf, len);
						vt->setup.in_enq = 30;
					}
					VT220Bell(vt);
				} else {
					memcpy(&vt->setup.enq[vt->setup.in_enq], buf, len);
					vt->setup.in_enq += len;
				}
			} else {
				vt->setup.enq[cursor] = (unsigned char) key;
				vt->setup.in_enq++;
				if(vt->setup.in_enq > 30) {
					vt->setup.in_enq = 30;
				}
			}
		}
	}

	VT220SetupShowStatus(vt);
}

void VT220SetupProcessKey(VT220* vt, u16 key)
{
	vt->setup.move = VT220_SETUP_MOVE_NONE;
	switch(key) {
		case VT220_KEY_KP_ENTER:
			VT220SetupProcessEnter(vt);
			break;
		case VT220_KEY_UP:
			if(vt->setup.cursor_y > 0) {
				vt->setup.move = VT220_SETUP_MOVE_UP;
				vt->setup.cursor_y--;
			}
			vt->setup.in_enq = -1;
			VT220SetupShowStatus(vt);
			break;
		case VT220_KEY_DOWN:
			if(vt->setup.cursor_y < 2) {
				vt->setup.move = VT220_SETUP_MOVE_DOWN;
				vt->setup.cursor_y++;
			}
			vt->setup.in_enq = -1;
			VT220SetupShowStatus(vt);
			break;
		case VT220_KEY_RIGHT:
			if(vt->setup.cursor_x < vt->columns) {
				vt->setup.move = VT220_SETUP_MOVE_RIGHT;
				vt->setup.cursor_x++;
			}
			vt->setup.in_enq = -1;
			VT220SetupShowStatus(vt);
			break;
		case VT220_KEY_LEFT:
			if(vt->setup.cursor_x > 0) {
				vt->setup.move = VT220_SETUP_MOVE_LEFT;
				vt->setup.cursor_x--;
			} else {
				vt->setup.move = VT220_SETUP_MOVE_LEFT_MARGIN;
			}
			vt->setup.in_enq = -1;
			VT220SetupShowStatus(vt);
			break;
		default:
			if(vt->setup.in_enq >= 0) {
				VT220SetupProcessAnswerback(vt, key);
			} else if(key == CR) {
				/* On the real VT220, ENTER (keypad) is not the
				 * same as Return (CR). Only ENTER is accepted
				 * in Setup. However, not every PC keyboard has
				 * a numeric keypad, therefore it is extremely
				 * convenient to have Return = ENTER here.
				 * This is NOT true for the Answerback field:
				 * that field treats ENTER and Return as
				 * separate keys, just like the real VT220. */
				VT220SetupProcessEnter(vt);
			} else if(key == HT) {
				/* In the tab field, you can jump to the next
				 * 'T' with the TAB key */
				if(vt->setup.screen == SETUP_SCREEN_TAB && vt->setup.cursor_y == 1) {
					for(int i = vt->setup.cursor_x + 1; i < vt->columns; i++) {
						if(i > 0 && vt->tabstops[i - 1]) {
							vt->setup.cursor_x = i;
							break;
						}
					}
				} else {
					VT220SetupShowHint(vt);
				}
			} else {
				VT220SetupShowHint(vt);
			}
			break;
	}

	VT220SetupShowScreen(vt);
}
