#include "lcd.h"

#define READY 0
#define PLAY 1
#define GAMEOVER 2

unsigned char seed = 0;
unsigned char state = READY;
unsigned long score = 0, speed = 0, highScore = 0;
unsigned char ScreenBuffer[8][128];

void startLcd();
void drawScore();
void drawSpeed();
void drawNote(unsigned char[4], unsigned char);

int main(void)
{
	//Initialization
	DDRA = 0xFF;
	PORTB = 0xFF; DDRB = 0xFF;
	DDRD = 0x0;
	DDRE = 0xFF;
	PORTG = 0x0; DDRG = 0xFF;

	cmd_lr( 0x3F ); //DISP ON
	cmd_lr( 0xB8 );
	cmd_lr( 0x40 );
	
	TCNT1H = 0xB;
	TCNT1L = 0xDC;
	TCCR1A = 0x0;
	TCCR1B = 0x01;

	EICRA = 0x66;
	EICRB = 0x66;
	EIMSK = 0x03;
	sei();

	InitScreenBuffer(ScreenBuffer);

	while (1) {
		if (state == READY) {
			TIMSK = (1<<TOIE1);
			lcd_clear(ScreenBuffer);
			startLcd();
			
			while (state == READY) {
				_delay_us(1);
			}
		}
		else if (state == PLAY) {
			unsigned char timeSet = 0;
			srand(seed);
			
			unsigned char NoteBuffer[4];
			InitNoteBuffer(NoteBuffer);
			
			while (state == PLAY) {
				if (timeSet == 0) {
					if (NoteBuffer[3] != 0) {
						if (highScore < score) {
							highScore = score;
						}
						state = GAMEOVER;
					}
					
					for (unsigned char noteLocation = 3; noteLocation > 0; --noteLocation) {
						NoteBuffer[noteLocation] = NoteBuffer[noteLocation - 1];
					}
					
					NoteBuffer[0] = rand() % 4 + 1;
					
					speed += 10;
					if (speed > 999) {
						speed = 999;
					}
				}
				
				lcd_clear(ScreenBuffer);
				startLcd();
				drawNote(NoteBuffer, timeSet);
				
				timeSet += 4;
				timeSet %= 16;
				
				unsigned char lastNote = 0, lastNoteLocation = 0;
				for (char findNote = 4; findNote > 0; --findNote) {
					if (NoteBuffer[findNote - 1] != 0) {
						lastNote = NoteBuffer[findNote - 1];
						lastNoteLocation = findNote - 1;
						break;
					}
				}
				
				if (lastNote != 0) {
					unsigned char isClear = 0;
					
					if (lastNote == 1) {
						for (unsigned long timePassed = 0; timePassed < 1000 - speed; ++timePassed) {
							if (PIND == 0xFB && isClear == 0) {
								score += 5;
								NoteBuffer[lastNoteLocation] = 0;
								isClear = 1;
							}
							_delay_us(200);
						}
					}
					else if (lastNote == 2) {
						for (unsigned long timePassed = 0; timePassed < 1000 - speed; ++timePassed) {
							if (PIND == 0xF7 && isClear == 0) {
								score += 5;
								NoteBuffer[lastNoteLocation] = 0;
								isClear = 1;
							}
							_delay_us(200);
						}
					}
					else if (lastNote == 3) {
						for (unsigned long timePassed = 0; timePassed < 1000 - speed; ++timePassed) {
							if (PIND == 0xEF && isClear == 0) {
								score += 5;
								NoteBuffer[lastNoteLocation] = 0;
								isClear = 1;
							}
							_delay_us(200);
						}
					}
					else if (lastNote == 4) {
						for (unsigned long timePassed = 0; timePassed < 1000 - speed; ++timePassed) {
							if (PIND == 0xDF && isClear == 0) {
								score += 5;
								NoteBuffer[lastNoteLocation] = 0;
								isClear = 1;
							}
							_delay_us(200);
						}
					} // else if
				} // if (lastNote != 0)
			} // while (state == PLAY)
		}
		else {
			while (state == GAMEOVER) {
				_delay_us(1);
				// fill in
			}
		}
	}
}

void startLcd() {
	GLCD_Line(ScreenBuffer, 0, 38, 63, 38);
	
	display_string(0, 0, "Score");
	drawScore();
	
	display_string(3, 0, "Speed");
	drawSpeed();
	
	display_string(6, 0, "H.S");
	drawHighScore();
	
	if (state == READY) {
		display_string(0, 7, "1 - START");
		display_string(1, 7, "2 - RESET");
		display_string(2, 7, "3 - LANE 1");
		display_string(3, 7, "4 - LANE 2");
		display_string(4, 7, "5 - LANE 3");
		display_string(5, 7, "6 - LANE 4");
	}
}

void drawScore() {
	display_char(1, 0, score / 100 + 0x30);
	display_char(1, 1, score % 100 / 10 + 0x30);
	display_char(1, 2, score % 10 + 0x30);
}

void drawSpeed() {
	if (speed == 999) {
		display_string(4, 0, "MAX");
	}
	else {
		display_char(4, 0, speed / 100 + 0x30);
		display_char(4, 1, speed % 100 / 10 + 0x30);
		display_char(4, 2, speed % 10 + 0x30);
	}
}

void drawHighScore() {
	display_char(7, 0, highScore / 100 + 0x30);
	display_char(7, 1, highScore % 100 / 10 + 0x30);
	display_char(7, 2, highScore % 10 + 0x30);
}

void drawNote(unsigned char NoteBuffer[4], unsigned char timeSet) {
	drawScore();
	drawSpeed();

	for (char i = 0; i < 4; ++i) {
		if (NoteBuffer[i] != 0) {
				GLCD_Rectangle(ScreenBuffer, timeSet + 16 * i, (NoteBuffer[i] - 1) * 22 + 39, timeSet + 16 * i + 6, NoteBuffer[i] * 22 + 39);
		}
	}
}

ISR (TIMER1_OVF_vect) {
	++seed;
}

// START
ISR (INT0_vect) {
	if (state == READY) {
		state = PLAY;
	}
	
	TIMSK = (0<<TOIE1);
}

// RESET
ISR (INT1_vect) {
	if (state == GAMEOVER) {
		state = READY;
		score = 0;
		speed = 0;
	}
}