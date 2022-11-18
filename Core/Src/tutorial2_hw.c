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

ButtonState state = ALL_OFF;

/* Private variables END */

/* Private function prototypes START */

/* Private function prototypes END */
/*
void gpio_classwork(void) {

	static uint32_t last_ticks =0;
	static uint8_t last_action = 0;
	ButtonState state=btn_state();
	static uint32_t last_one =0;



	switch (state) {
	case BOTH_BTN:
		if (HAL_GetTick() - last_one >= 1000) {
			last_one = HAL_GetTick();
			if (last_action == 0) {
				last_action=1;
			}
			else {
				last_action=0;
			}	// do step 1

		}
		if (last_action ==0) {
			gpio_reset(LED1);
			gpio_reset(LED3);
			if (HAL_GetTick()-last_ticks >=50){
				last_ticks = HAL_GetTick();
				led_toggle(LED2);
			}
		}
		if (last_action ==1){
			gpio_reset(LED2);
			if (HAL_GetTick()-last_ticks >=50){
				last_ticks = HAL_GetTick();
				led_toggle(LED1);
				led_toggle(LED3);
			}
		}
		break;

	case BTN1_ONLY:
		gpio_reset(LED1);
		break;
	case BTN2_ONLY:
		if (HAL_GetTick()-last_ticks >=50){
			last_ticks = HAL_GetTick();
			led_toggle(LED2);
		}
		break;
	case ALL_OFF:
		gpio_set(LED1);
		gpio_set(LED2);
		gpio_set(LED3);
		break;



	}


}


void tft_classwork(void) {

	uint32_t real_time= HAL_GetTick();
	if(tft_update(100)){
		int real_time2=real_time%1000;
		int mm=real_time/60000;
		int ss=(real_time/1000)%60;
		tft_prints(0,0,"%d:%d:%d",mm,ss,real_time2);

	}

//	if (HAL_GetTick()-last_ticks >=1000){
//		last_ticks = HAL_GetTick();
//	tft_print_rectangle(uint16_t color, uint32_t x, uint32_t y, uint32_t w,
//	        uint32_t h);

//
//	}

}
*/
void tutorial2_homework(void) {

   /* Your code start here */
	static uint32_t last_ticks =0;
	static uint32_t last_one =0;
	static uint32_t last_LED1 =0;
	static uint32_t last_LED2 =0;

	ButtonState state=btn_state();

	static int mode_now=0;
	static int mode_previous=0;
	int mode=0;
	static int mode_LED=0;

	if(btn_read(BTN1)==true) {
			mode_now=0;
			mode_previous=0;
			last_ticks= HAL_GetTick();

	}
	else{
		mode_now=1;
		mode_LED = 1;

//					mode_now=0;

	}

	if (HAL_GetTick() - last_ticks >= 200) {
		mode_previous=1;
	}



	tft_prints(0, 1, "mode now:  %d", mode_now);
	tft_prints(0, 2, "prev    :  %d", mode_previous);
	tft_prints(0, 3, "LED    :  %d", mode_LED);


	if ((mode_now == 1) && (mode_previous == 0)) {
		mode = 1;
	}
//	if ((mode_now == 0) && (mode_previous == 1)) {
//		mode_LED = 1;
//	}

	if (mode == 1){
		last_one=HAL_GetTick();
		if (HAL_GetTick() - last_one <= 1000) {
			tft_prints(0,0,"Hello Ricky");
		}
	}
//	if (mode_LED == 1) {
//		last_LED1=HAL_GetTick();
//
//		if (HAL_GetTick() - last_LED1 <= 1000) {
//			if (HAL_GetTick() - last_LED2 >= 50) {
//				last_LED2= HAL_GetTick();
//				led_toggle(LED1);
//			}
//		}
//		mode_previous=0;
//		led_off(LED1);
//	}

	static int update = 0;
	if (mode_now == 0 && mode_LED == 1) {
		// flash led
		if (update == 0) {
			last_LED1 = HAL_GetTick();
			update = 1;
		}

				if (HAL_GetTick() - last_LED1 <= 1000) {
//					last_LED1=HAL_GetTick();

					if (HAL_GetTick() - last_LED2 >= 50) {
						last_LED2= HAL_GetTick();
						led_toggle(LED1);
				}
				} else {
					mode_LED = 0;
					update = 0;
				}


	} else {
		led_off(LED1);
	}
}

//				else {
//					last_LED1 =HAL_GetTick();
//					if (HAL_GetTick() - last_LED1 <= 1000) {
//						if (HAL_GetTick() - last_LED2 <= 1000) {
//							last_LED2= HAL_GetTick();
//							led_toggle(LED1);






	/*switch(state) {
	case BOTH_BTN:

		gpio_set(LED1);
		gpio_set(LED2);
		gpio_set(LED3);
		break;
	case BTN1_ONLY:
		if (HAL_GetTick() - last_one <= 1000) {
			last_one = HAL_GetTick();
			tft_prints(0,0,"Hello Ricky");
		}
			gpio_set(LED1);
			gpio_set(LED2);
			gpio_set(LED3);
			break;
		case BTN2_ONLY:
			tft_prints(0,0,"Hello Ricky");
			gpio_set(LED1);
			gpio_set(LED2);
			gpio_set(LED3);
			break;
		case ALL_OFF:
			if (HAL_GetTick()-last_ticks >=50){
				last_ticks = HAL_GetTick();
				led_toggle(LED1);
			}

		}
		*/

    /* Your code end here*/




// You can define your helper functions down below
// You can also modify the function definition above as long as your move is reasonable
