#include "main.h"
#include "tim.h"


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


void pwm_init(void) {
	// init the pwm prescaler value and auto-reload value and start the pwm
	/* Your code start here */
	MX_TIM5_Init();
	TIM5->ARR = 1000;    //set the timer1 auto-reload counter
	TIM5->PSC = 900;    //set the timer1 prescaler value
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
	TIM5->CCR1 = 25; //set the compare value of timer1 channel1
	/* Your code end here */
}

void pwm_classwork(void) {
	/* Your code start here */
	static int tick = 0;
	static int on = 1;
	static char* text = "";
	int state = gpio_read(BTN1);
//	if (state == 0){
//		TIM5->CCR1 = 5999;
//		gpio_reset(LED1);
//	}
//	else{
//		TIM5->CCR1 = 2999;
//		gpio_set(LED1);
//	}
	if (HAL_GetTick() - tick < 1000){
	  TIM5->CCR1 = 50;
	  text = "1";// duty cycle is .5 ms
	}
	else if (HAL_GetTick() - tick < 2000){
	  TIM5->CCR1 = 75;  // duty cycle is 1.5 ms
	  text = "2";
	}
	else if (HAL_GetTick() - tick < 3000){
	  TIM5->CCR1 = 175;  // duty cycle is 2.5 ms
	  text = "3";
	}
	else {
		tick = HAL_GetTick();
	}

	if(tft_update(1) == 0){
		tft_prints(0, 0, "%s", text);
	}
	/* Your code end here */
}


#define LVL_HEIGHT 3
#define LVL_WIDTH 8
void pwm_homework(void) {
	/* Your code start here */
//	static int posy
	/* Your code end here*/
}
