#include "main.h"
#include "tim.h"
#include "lcd/lcd.h"


/* Private variables END */

/* Private function prototypes START */

/* Private function prototypes END */

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

static ButtonState btn_state(void) {
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





void pwm_classwork(void) {
	/* Your code start here */
	 //set the compare value of timer1 channel1
	//TIM1->CCR2 = 678; //set the compare value of timer1 channel2
	ButtonState state=btn_state();
	static uint8_t mode = 0;
		static uint32_t p = 0;
		if (risingedge(BUTTON1)) {
			TIM5->CCR1 = 99;
			mode = 0;
		} else if (read_button(BUTTON2)) {
			if (mode == 0) {
				TIM5->CCR1 = 297;
				mode = 1;
				p = HAL_GetTick();
			} else if (mode == 1 && p + 1000 < HAL_GetTick()) {
				TIM5->CCR1 = 495;
			}
		}
}
/*
int risingedge() {
	static int level = 0;
	if (read_button(BUTTON1) && level == 0) {
		level = 1;
	} else if (!read_button(BUTTON1) && level == 1) {
		level = 0;
		return 1;
	}
	return 0;
}
*/

int risingedge2() {
	static int level2 = 0;
	if (read_button(BUTTON2) && level2 == 0) {
		level2 = 1;
		return 1;
	} else if (!read_button(BUTTON2) && level2 == 1) {
		level2 = 0;
		return 0;
	}
	return 0;
}

int risingedge3() {
	static int level3 = 0;
	if (read_button(BUTTON2) && level3 == 0) {

		return 1;
	} else if (!read_button(BUTTON2) && level3 == 1) {
		level3 = 0;
		return 0;
	}
	return 0;
}


void pwm_homework(void) {
	/* Your code start here */
	static uint8_t mode = 0;
	static uint32_t last_one = 0;
	static uint8_t update = 0;
	static uint32_t last_one2 = 0;
	static uint32_t double_move = 0;
	static uint32_t double_move_timer = 0;
	static uint32_t double_move_timer_clear = 0;
	static uint8_t update2 = 0;
	static uint32_t counter = 0;

	static uint32_t timer = 0;
	static uint8_t reveal = 0;
	static uint8_t update3 = 0;
	static uint32_t marbles[10]={50,50,50};
	static uint8_t inst_marbles = 3;
	static uint32_t timer2 = 0;

/*

	if (risingedge(BUTTON1)) {
		TIM5->CCR1=360;
		last_one=HAL_GetTick();
		}
	if ( (HAL_GetTick()-last_one>=2000)&& (TIM5->CCR1 == 360)) {
		TIM5->CCR1 = 99;
	}
	*/


	if(HAL_GetTick()-timer>=100) {
		timer=HAL_GetTick();
		for (int i = inst_marbles-1;i>=0; i--){
			marbles[i]--;
			if (marbles[i]==0) {
				marbles[i]=0;
			}
		}
	}


	if (risingedge(BUTTON1)) {
		if (TIM5->CCR1==99 || TIM5->CCR1==150) {
			//tft_prints(0,3,"xd");
			TIM5->CCR1=360;
			counter++;
			for (int i = inst_marbles-1;i>=0; i--){
				if (marbles[i]>5 ||marbles[i]<10) {
					timer2=HAL_GetTick();
					if (HAL_GetTick()-timer2<1000) {
						marbles[i]=marbles[i]+10;
					}
				}
			}
		}
		else {
			TIM5->CCR1=99;
			counter++;
		}
	}




	if (risingedge3(BUTTON2)&& counter>=2) {
		/*if (risingedge2(BUTTON2)){
			marbles[inst_marbles]=50;
			inst_marbles++;
			marbles[inst_marbles]=50;
			inst_marbles++;
			update3=1;
		}
*/
		if (TIM5->CCR1==360) {
			TIM5->CCR1=250;
		}
		if (TIM5->CCR1==99){
		TIM5->CCR1 = 150;
		}
		//tft_prints(0,6,"xd");
		if(mode==0) {
			last_one2 = HAL_GetTick();
			mode=1;
		}
		led_off(LED1);
		if ( (HAL_GetTick()-last_one2>=500)) {
			led_on(LED1);
			//tft_prints(0,3,"xd");

			double_move=1;
		if ( (HAL_GetTick()-last_one2>=1000)) {
			led_on(LED2);
			//tft_prints(0,4,"xd");
			double_move=1;

		if ( (HAL_GetTick()-last_one2>=1500)) {
			led_on(LED3);
			//tft_prints(0,5,"xd");
			double_move=1;

			if ((HAL_GetTick()-last_one2>=2000) ) {
				led_on(LED4);
				//tft_prints(0,7,"xd");
				double_move=2;
				counter=counter-2;
				marbles[inst_marbles]=50;
				inst_marbles++;
				marbles[inst_marbles]=50;
				inst_marbles++;
			}
		}
		}
		}


	}
	else {
				last_one2=0;
				led_off(LED2);
				led_off(LED1);
				led_off(LED3);
				led_off(LED4);
				mode=0;

			}
	if (!read_button(BUTTON2) && (double_move==2)){
		//tft_prints(0,8,"xd");
		if (double_move_timer_clear==0) {
			led_on(LED1);
			if (TIM5->CCR1==150) {
				update2=1;
			}
			if (update2==1) {
				TIM5->CCR1=360;
			}
			if (TIM5->CCR1==250) {
				update2=2;
			}
			if (update2==2) {
				TIM5->CCR1=99;
			}

			if (update==0) {
				double_move_timer=HAL_GetTick();
				update=1;
			}
			if (HAL_GetTick()-double_move_timer>=500){
				if (update2==2) {
					TIM5->CCR1=360;
				}
				if (update2==1) {
					TIM5->CCR1=99;
				}
				for (int i = inst_marbles-1;i>=0; i--){
					if (marbles[i]>5 ||marbles[i]<10) {
						timer2=HAL_GetTick();
						if (HAL_GetTick()-timer2<1000) {
							marbles[i]=marbles[i]+10;
						}

					}
				}
				double_move_timer_clear=1;
				update=0;
				update2=0;
			}
		} else {
		double_move=double_move-2;
		double_move_timer_clear=0;
		}
	}
	if ((!read_button(BUTTON2) && (double_move==1))){

			if (double_move_timer_clear==0) {
				led_on(LED1);
				if (TIM5->CCR1==150) {
					TIM5->CCR1=360;
				}
				if (TIM5->CCR1==250) {
					TIM5->CCR1=99;
				}
				for (int i = inst_marbles-1;i>=0; i--){
					if (marbles[i]>5 ||marbles[i]<10) {
						timer2=HAL_GetTick();
						if (HAL_GetTick()-timer2<1000) {
							marbles[i]=marbles[i]+10;
						}

					}
				}


			} else {
			double_move=double_move-1;
			double_move_timer_clear=0;
			}
		}


		/**
		if (update == 0) {
			last_one = HAL_GetTick();
			TIM5->CCR1 = 495;
			update = 1;
		}

		if (HAL_GetTick()-last_one <=1000 &&(update==1)) {

		} else {
			TIM5->CCR1 = 99;
			update=0;
		}
		**/

	for (int i = inst_marbles-1;i>=0; i--){
		tft_prints(0,i,"Marbles %d:%d",i,marbles[i]);
	}


	/* Your code end here*/
}
