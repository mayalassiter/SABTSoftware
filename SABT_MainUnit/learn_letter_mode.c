

/*
 * @file learn_letter_mode.c
 *
 * @brief code for helper functions "learning letter modes" (MD 2,7,8,12)
 * @author: Edward Cai
 * Edited by Marjorie Carlson
 *
 * This file provides the functionality for all the modes for learning letters 
 * (English, Hindi, etc.) At the menu, the user chooses between "learning" mode
 * (1), which tells them the correct dots, and which iterates through the alphabet
 * in order; and "practicing" mode (2), which does not prompt them with the correct
 * dots, and which shuffles the alphabet first (as well as after each time the
 * student completes the entire alphabet.)
 *
 * Students must input the correct dots for the prompted letter and then press
 * enter, at which point the mode will check their answer and respond appropriately.
 * They may replay the question by hitting cancel or left, and skip to the next
 * letter by hitting right arrow.
 *
 * The states are: MENU -> GENQUES (gets the next letter) -> PROMPT (plays
 * the letter, and its dot sequence if in learn mode) -> INPUT (gets the inputted
 * cell; if it's a cell terminated by enter, goes to CHECK; if input is cancel or
 * left arrow, goes back to PROMPT to replay question; if input is right arrow, goes
 * back to GENQUES to skip to the next letter. CHECK checks whether the input was
 * correct; if the input was correct, then it plays tada and goes to GENQUES *if*
 * the inputted glyph was the last glyph in the current letter; if there are more
 * glyphs in this letter, it goes back to input to get them. If the input was
 * incorrect, it goes back to the first glyph in this letter and returns to PROMPT
 * for another attempt, unless the user is out of tries, in which case it plays
 * the dot sequence and returns to INPUT for another attempt.
 * 
 */ 

// Standard libraries
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Custom libraries
#include "audio.h"
#include "io.h"
#include "common.h"
#include "debug.h"
#include "script_common.h"
#include "learn_letter_mode.h"

// State variables
static char mode_name[5];
static char next_state = STATE_NULL;
static char user_response = NO_DOTS;
static char submode = SUBMODE_NULL;
static bool should_shuffle = false;
static int index = 0;
static glyph_t* curr_glyph = NULL;
static glyph_t* user_glyph = NULL;
static char cell = 0;
static char cell_pattern = 0;
static char cell_control = 0;
static int incorrect_tries = 0;

void learn_letter_reset(script_t* new_script, char* new_lang_fileset, char* new_mode_fileset) {
	set_mode_globals(new_script, new_lang_fileset, new_mode_fileset);
	strcpy(mode_name,new_mode_fileset);
	strtok(mode_name, "_");
	next_state = STATE_MENU;
	user_response = NO_DOTS;
	submode = SUBMODE_NULL;	
	index = -1;
	curr_glyph = NULL;
	user_glyph = NULL;
	cell = 0;
	cell_pattern = 0;
	cell_control = 0;
	incorrect_tries = 0;
	log_msg("[%s] Mode reset", mode_name);
	NEWLINE;
}

void learn_letter_main(script_t* SCRIPT_ADDRESS, char* LANG_FILESET, char* MODE_FILESET) {
	switch (next_state) {
		case STATE_MENU:
			switch(create_dialog(MP3_MENU, DOT_1 | DOT_2 | ENTER_CANCEL)) {	
				case NO_DOTS:
					break;

				case '1':
					log_msg("[%s] Submode: Learn", mode_name);
					NEWLINE;
								
					play_mp3(MODE_FILESET, MP3_INSTRUCTIONS);
					submode = SUBMODE_LEARN;
					unshuffle(SCRIPT_ADDRESS->num_letters, SCRIPT_ADDRESS->letters);
					next_state = STATE_GENQUES;
					break;

				case '2':
					log_msg("[%s] Submode: Play", mode_name);
					NEWLINE;
					
					play_mp3(MODE_FILESET, MP3_INSTRUCTIONS);
					submode = SUBMODE_PLAY;
					shuffle(SCRIPT_ADDRESS->num_letters, SCRIPT_ADDRESS->letters);
					should_shuffle = true;
					next_state = STATE_GENQUES;
					break;

				case CANCEL:
					log_msg("[%s] Quitting to main menu", mode_name);
					NEWLINE;
					
					quit_mode();
					break;

				case ENTER:
					log_msg("[%s] Re-issuing main menu prompt", mode_name);
					NEWLINE;
					
					next_state = STATE_MENU;
					break;

				default:
					break;
			}
			break;

		case STATE_GENQUES:
			curr_glyph = get_next_letter(SCRIPT_ADDRESS, should_shuffle);
			log_msg("[%s] State: GENQUES. Next glyph: %s", mode_name, curr_glyph->sound);
			NEWLINE;
			
			play_mp3(LANG_FILESET, MP3_NEXT_LETTER);
			next_state = STATE_PROMPT;
			break;

		case STATE_PROMPT:
			log_msg("[%s] State: PROMPT.",mode_name);
			
			switch(submode) {
				case SUBMODE_LEARN:
					play_glyph(curr_glyph);
					play_mp3(MODE_FILESET, MP3_FOR_X_PRESS_DOTS);
					play_dot_sequence(curr_glyph);
					break;

				case SUBMODE_PLAY:
				//	play_silence(500);
					play_glyph(curr_glyph);
					break;

				default:
					break;
			}
			next_state = STATE_INPUT;
			break;

		case STATE_INPUT:
			if (io_user_abort == true) {
				log_msg("[%s] User aborted input\n\r", mode_name);
				
				next_state = STATE_PROMPT;
				io_init();
			}
			cell = get_cell();
			if (cell == NO_DOTS)
				break;
			cell_pattern = GET_CELL_PATTERN(cell);
			cell_control = GET_CELL_CONTROL(cell);
			log_msg("Pattern %d, control %d.\n\r", cell_pattern, cell_control);
			
						
			switch (cell_control) {
				case WITH_ENTER:
					user_glyph = search_script(SCRIPT_ADDRESS, cell_pattern);
					next_state = STATE_CHECK;
					log_msg("[%s] Checking answer \n\r", mode_name);
					
					break;
				case WITH_CANCEL: // for easy debugging access to multiglyph letters
/*					SCRIPT_ADDRESS->index = 47;
					next_state = STATE_GENQUES;*/
				case WITH_LEFT:
					next_state = STATE_PROMPT;
					break;
				case WITH_RIGHT:
					next_state = STATE_GENQUES;
					break;
			}
			break;

		case STATE_CHECK:
			log_msg("[%s} State: CHECK.\n\r", mode_name);
			
			if (glyph_equals(curr_glyph, user_glyph)) {
				if (curr_glyph -> next == NULL) {
					incorrect_tries = 0;
					log_msg("[%s] User answered correctly\n\r", mode_name);
					
					play_mp3(LANG_FILESET, MP3_CORRECT);
					play_mp3(SYS_FILESET, MP3_TADA);
					next_state = STATE_GENQUES;
				}
				else {
					curr_glyph = curr_glyph->next;
					play_mp3(LANG_FILESET, MP3_NEXT_CELL);
					
					if (submode == SUBMODE_LEARN)
						play_dot_sequence(curr_glyph);
					else
						play_glyph(curr_glyph);
					next_state = STATE_INPUT;
				}
			}
			else {
				incorrect_tries++;
				log_msg("[%s] User answered incorrectly\n\r", mode_name);;
							
				play_mp3(LANG_FILESET, MP3_INCORRECT);
				play_mp3(LANG_FILESET, MP3_TRY_AGAIN);
				curr_glyph = get_root(SCRIPT_ADDRESS, curr_glyph);	
				next_state = STATE_PROMPT;
				if (incorrect_tries >= MAX_INCORRECT_TRIES) {
					play_glyph(curr_glyph);
					play_mp3(MODE_FILESET, MP3_FOR_X_PRESS_DOTS);
					play_dot_sequence(curr_glyph);
					next_state = STATE_INPUT;
				}
			}
		break;

		default:
			break;
	}
}
