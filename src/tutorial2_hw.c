#include "main.h"
#include "lcd/lcd.h"
#include "lcd/lcd_graphics.h"

/** Design ideologies:
 * More small functions >>> Few giant functions
 * Reusability and Extendability is most important (Dont-Repeat-Yourself)
 * then Readability (comments are good, self documenting code is best)
 * No "magic numbers", use defines, constants, variables etc
 * ALWAYS INDENT CORRECTLY
 * No blocking (long while loops, HAL_Delay)
 */

// Enums are always good
typedef enum {
    BOTH_BTN, BTN1_ONLY, BTN2_ONLY, ALL_OFF,
} ButtonState;

typedef enum {
    BUTTON1, BUTTON2,

    //This trick means the NumButtons will equal how many buttons there are (how many enum values)
    //This works as long as the enum values are simple (count up from 0)
    NUM_BUTTON,
} Button;

typedef enum {
    LED1, LED2, LED3, LED4, NUM_LED,
} LED;

/**
 * @brief read the button state
 * return 1 if the button is pressed;
 * return 0 if the button is released;
 */
static inline uint8_t read_button(Button btn) {
    switch (btn) {
    case BUTTON1:
        return !btn_read(BTN1);
    case BUTTON2:
        return !btn_read(BTN2);
    default:
        return 0;
    }
}

ButtonState btn_state(void) {
    if (read_button(BUTTON1) && read_button(BUTTON2)) {
        return BOTH_BTN;
    } else if (read_button(BUTTON1)) {
        return BTN1_ONLY;
    } else if (read_button(BUTTON2)) {
        return BTN2_ONLY;
    } else {
        return ALL_OFF;
    }
}

/* Private variables START */



/* Private variables END */

/* Private function prototypes START */

/* Private function prototypes END */

void gpio_classwork(void) {
    /* Your code start here */
    // Initialize Everything Here, this is like setup() in Arduino;
	ButtonState state = btn_state();

	static int last_ticks = 0, second_timer = 0;

	if (state == BOTH_BTN){
		if(HAL_GetTick() - last_ticks < 1000){
			gpio_reset(LED1);
			gpio_reset(LED3);
			if (HAL_GetTick() - second_timer > 50){
				gpio_toggle(LED2);
				second_timer = HAL_GetTick();
			}
		}
		else if (HAL_GetTick() - last_ticks < 2000){
			gpio_reset(LED2);
			if (HAL_GetTick() - second_timer > 50){
				gpio_toggle(LED1);
				gpio_toggle(LED3);
				second_timer = HAL_GetTick();
			}
		}
		else{
			last_ticks = HAL_GetTick();

		}
	}
	else if (state == BTN1_ONLY){
		gpio_reset(LED1);
	}
	else if (state == BTN2_ONLY){
		if(HAL_GetTick() - last_ticks > 50){
			gpio_toggle(LED2);
			last_ticks = HAL_GetTick();
		}
	}
	else{
		gpio_set(LED1);
		gpio_set(LED2);
		gpio_set(LED3);
//		if(HAL_GetTick() - last_ticks > 50){
//			gpio_toggle(LED3);
//			last_ticks = HAL_GetTick();
//		}
	/* Your code end here */
	}
}

void tft_classwork(void) {
    /* Your code start here */
	int msecond = HAL_GetTick();
	int second = msecond / 1000;
	int minute = second / 60;
	int grey = second % 10 + 1;
	if (msecond % 1000 == 0){
		tft_print_rectangle(GRAYSCALE(grey), 0, 30, 50, 50);
	}
    if(tft_update(1) == 0){
    	if (minute < 10){
			if(second % 60 < 10){
				tft_prints(0,0,"0%d:0%d:%d", minute, second % 60, msecond % 1000);
			}
			else{
				tft_prints(0,0,"0%d:%d:%d", minute, second % 60, msecond % 1000);
			}
    	}
    	else{
			if(second % 60 < 10){
				tft_prints(0,0,"%d:0%d:%d", minute, second % 60, msecond % 1000);
			}
			else{
				tft_prints(0,0,"%d:%d:%d", minute, second % 60, msecond % 1000);
			}
    	}
    }
    /* Your code end here */
}

void tutorial2_homework(void) {
    /* Your code start here */
	static int tick, last_state1 = 1, last_state2 = 2, posx = 60, posy = 80, time_pressed, time_released;

	int state1 = gpio_read(BTN1), state2 = gpio_read(BTN2);

	if(state1 == 0 && state1 != last_state1){
		time_pressed = HAL_GetTick();
	}
	else if (state1 == 1 && state1 != last_state1){
		time_released = HAL_GetTick();
		if(time_released - time_pressed > 300){
			posy -= CHAR_HEIGHT;
			tft_force_clear();
		}
		else{
			posx -= CHAR_WIDTH;
			tft_force_clear();
		}
	}

	if(state2 == 0 && state2 != last_state2){
		time_pressed = HAL_GetTick();
	}
	else if (state2 == 1 && state2 != last_state2){
		time_released = HAL_GetTick();
		if(time_released - time_pressed > 300){
			posy += CHAR_HEIGHT;
			tft_force_clear();
		}
		else{
			posx += CHAR_WIDTH;
			tft_force_clear();
		}
	}

	if(tft_update(1) == 0){
		drawCircle(posx, posy, 10, BLUE);
	}

	last_state1 = state1;
	last_state2 = state2;
    /* Your code end here*/
}

// You can define your helper functions down below
void homework1(void){
	ButtonState state = btn_state();
	static int tick = 0;
	char* text;
	if(state != BTN1_ONLY){
		if(HAL_GetTick() - tick > 500){
			gpio_toggle(LED1);
			tick = HAL_GetTick();
		}
		text = "";
	}
	else{
		gpio_set(LED1);
		text = "Hello, Dewa";
	}

	if(tft_update(1) == 0){
		tft_prints(0, 0, "%s", text);
	}
}

void homework2(void){
	static int tick = 0, second_tick = 0;
	static int last_state = 1;
	static char* text;
	static int action = 0;

	int state = gpio_read(BTN1);

	if(state ==  0 && state != last_state){
		tick = HAL_GetTick();
		action = 1;
	}
	else if (state == 1 && state != last_state){
		tick = HAL_GetTick();
		action = 2;
	}

	switch(action){
	case 0:
		gpio_set(LED1);
		text = "";
		break;
	case 1:
		if(HAL_GetTick() - tick < 1000){
			text = "Hello, Dewa";
		}
		else{
			action = 0;
		}
		break;

	case 2:
		if(HAL_GetTick() - tick < 1000){
			if(HAL_GetTick() - second_tick > 50){
				gpio_toggle(LED1);
				second_tick = HAL_GetTick();
			}
		}
		else {
			action = 0;
		}
		break;
	default:
		break;
	}

	if(tft_update(1) == 0){
		tft_prints(0, 0, "%s", text);
	}

	last_state = state;
}

// You can also modify the function definition above as long as your move is reasonable
