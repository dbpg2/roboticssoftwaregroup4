#include "main.h"
#include <stdio.h>
#define true 1
#define false 0
#define bool uint8_t

uint16_t threshold_transform(uint16_t value){
	if(value>0xffff/2) {
		return 0xffff;
	}
	return 0;
}

void threshold(uint16_t* img, uint16_t width, uint16_t height) {
	for(uint16_t i = 0; i<width*height; i++) {
		*(img+i)=threshold_transform(*(img+i));
	}
}

uint16_t indexCal(uint8_t i, uint8_t j) {
	uint16_t tmp1 = i;
	uint16_t tmp2 = j;
	return tmp1*120+tmp2;
}

void sobelOperator(uint16_t *img, uint8_t width, uint8_t height){
	uint16_t tmp1[128] = {0};
	uint16_t tmp2[128] = {0};

	for(uint8_t i = 1; i<(height-1);i++){
		for(uint8_t j = 1; j<width-1; j++){
			int gx = *(img + indexCal(i-1,j-1))*-1
					+*(img + indexCal(i, j-1))*-2
					+*(img + indexCal(i+1,j-1))*-1
					+*(img + indexCal(i-1,j+1))
					+*(img + indexCal(i,j+1))*2
					+*(img + indexCal(i+1,j+1));

			int gy = *(img + indexCal(i-1, j-1))*-1
					+*(img + indexCal(i-1,j))*-2
					+*(img + indexCal(i-1,j+1))*-1
					+*(img + indexCal(i+1,j-1))*1
					+*(img + indexCal(i+1,j))*2
					+*(img + indexCal(i+1,j+1));
			if(gx<0) gx*=-1;
			if(gy<0) gy*=-1;
			uint16_t value = gx+gy;
			if(i%2==1){
				tmp1[j] = value;
			}else{
				tmp2[j] = value;
			}
			if(i>=2 && i%2==1){
				*(img + indexCal(i-2,j)) = tmp2[j]>0x8000?0xffff:0;
			}else if(i>=2){
				*(img + indexCal(i-2,j)) = tmp1[j]>0x8000?0xffff:0;
			}
		}

	}
}



uint8_t detectSensor(uint16_t *img, uint8_t startI, uint8_t startJ, uint8_t endI, uint8_t endJ) {
	uint8_t ans = 0;

	for(uint8_t i = startI; i<endI; i++) {
				for(uint8_t j = startJ; j<endJ; j++) {
					if(ans>40){
						break;
					}
					if(*(img+indexCal(i,j))==0xffff){
						ans++;
					}else{
						*(img+indexCal(i,j)) = 255;
					}
				}
			}
	return ans;
}

uint8_t inRange(uint8_t num, uint8_t min, uint8_t max) {
	if(num>max || num<min) {
		return 0;
	}
	return 1;
}

uint8_t testRight(uint16_t *img, uint8_t currDeg) {
	uint8_t back = 0;
		uint8_t front = 0;
		uint8_t mid = 0;
		for(uint8_t j = 0; j<120; j+=10) {
			for(uint8_t i = 0; i<160; i++) {
				if(*(img+indexCal(i,j))==0xffff){
					break;
				}else{
					if(j==100) {
						front++;
						*(img+indexCal(i,j)) = 0xffff;
					}else if(j==20){
						back++;
						*(img+indexCal(i,j)) = 0xffff;
					}else if(j==60){
						mid++;
						*(img+indexCal(i,j)) = 0xffff;
					}else{
						*(img+indexCal(i,j)) = 255;
					}
				}
			}
		}
		tft_prints(0,1,"front: %d",front);
			tft_prints(0,2,"mid: %d",mid);
			tft_prints(0,3,"back: %d",back);
			int tmp = front-back+mid;
			if(tmp<0) {
				tmp = 0;
			}
			updateDCMotorSpeed(155,2);
			if(mid<30) {
				if(front<back){
					updateDCMotorSpeed(0,0);
					updateDCMotorSpeed(240,1);
				}
			}


		/*if(front>130) {
			updateDCMotorSpeed(240,0);
			updateDCMotorSpeed(80,1);
			return 0;
		}
			updateDCMotorSpeed(155,2);
		if(front<30) {
			return 180;
		}
		if(front<60) {
			return 130;
		}

		if(front>110) {
			return 0;
		}

		if(front>90) {
			return 50;
		}
		*/
		return tmp;

}

uint8_t crazy(uint16_t *img, uint8_t currDeg) {
	uint8_t cnt1 = 0;
	uint8_t cnt2 = 0;
	uint8_t mid = 0;
	tft_prints(0,1,"%d",cnt1);
	tft_prints(0,2,"%d",mid);
	tft_prints(0,3,"%d",cnt2);
	for(uint8_t j = 0; j<120; j+=10) {
		for(uint8_t i = 0; i<160; i++) {
			if(*(img+indexCal(i,j))==0xffff){
				break;
			}else{
				if(j==100) {
					cnt2++;
					*(img+indexCal(i,j)) = 0xffff;
				}else if(j==20){
					cnt1++;
					*(img+indexCal(i,j)) = 0xffff;
				}else if(j==60){
					mid++;
					*(img+indexCal(i,j)) = 0xffff;
				}else{
					*(img+indexCal(i,j)) = 255;
				}
			}
		}
	}
	if(mid<50) {
		if(cnt1<cnt2) {
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);
			updateDCMotorSpeed(250,0);
			updateDCMotorSpeed(250,1);
			return 180;
		}else{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_SET);
			updateDCMotorSpeed(250,0);
			updateDCMotorSpeed(250,1);
			return 0;
		}
	}else{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);
					updateDCMotorSpeed(170,0);
					updateDCMotorSpeed(170,1);
		if(cnt1<=135) {
			return 90-cnt1/5;
		}
		if(cnt2<=150){
			return 120-cnt2/5;
		}
	}
	return 105;

}

uint8_t final(uint16_t *img, uint8_t currDeg, uint8_t speed) {

		static uint32_t stopTick = 0;
		static uint32_t outTick = 0;
		static uint8_t lastDirection = 0; // 0->left, 1->right
		uint8_t cnt1 = 0;
		uint8_t cnt2 = 0;
		uint8_t mid = 0;
		tft_prints(0,1,"%d",cnt1);
		tft_prints(0,2,"%d",mid);
		tft_prints(0,3,"%d",cnt2);
		for(uint8_t j = 0; j<120; j+=10) {
			for(uint8_t i = 0; i<160; i++) {
				if(*(img+indexCal(i,j))==0xffff){
					break;
				}else{
					if(j==100) {
						cnt2++;
						*(img+indexCal(i,j)) = 0xffff;
					}else if(j==20){
						cnt1++;
						*(img+indexCal(i,j)) = 0xffff;
					}else if(j==60){
						mid++;
						*(img+indexCal(i,j)) = 0xffff;
					}else{
						*(img+indexCal(i,j)) = 255;
					}
				}
			}
		}
		static uint8_t status = 0;
		tft_prints(0,1,"%d",cnt1);
		tft_prints(0,2,"%d",mid);
		tft_prints(0,3,"%d",cnt2);

		static uint8_t tmpBackward = 0;
		static uint8_t canBackward= 0;
		if(mid<10) {
			tmpBackward = 1;

		}
		if(tmpBackward){
			if(!canBackward) {
				tmpBackward = 0;
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);
				return 90;
			}
			if(mid>40 && mid!=160) {
				updateDCMotorSpeed(0,2);
				tmpBackward = 0;
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);
			}else{
				updateDCMotorSpeed(speed+20,2);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_SET);
			}
			return currDeg;
		}

		if(status == 0) {
				updateDCMotorSpeed(speed+20,2);
				tft_prints(0,0,"0");
				if(mid<=150) {
					status = 1;
					return 90;
				}
				if(cnt1<=135) {
					return 90-cnt1/5;
				}
				if(cnt2<=150){
					return 160-cnt2/5;
				}
			}else if(status ==1) {
				updateDCMotorSpeed(speed+10,2);//
				canBackward = 1;
				tft_prints(0,0,"1");
				if(cnt1>=110 && cnt1<=130 && cnt2>=110 && cnt2<=130 && mid>=110 && mid<=130 ){
					status = 4;
					outTick = HAL_GetTick();
					return 90;
				}
				if(mid>155) {
					status = 0;
					return 90;
				}
				if(mid<=90) {
					stopTick = HAL_GetTick();
					status = 2;
				}
				if(cnt1<cnt2) {
					return 80;//60/////////
				}else{
					return 120;//140////////
				}

			}else if(status == 2){
				tft_prints(0,0,"2");
				if(HAL_GetTick() - stopTick>=150) {
					status = 3;
				}
				if(cnt1<cnt2) {
					updateDCMotorSpeed(speed+40,1);//20
					updateDCMotorSpeed(speed,0);
					return 0;
				}else{
					return 180;
				}

			}else if(status == 3){
				tft_prints(0,0,"3");
				if(mid>=60) {
							status = 0;

							return 90;
						}
				if(cnt1<cnt2) {
					updateDCMotorSpeed(speed+85,1); // 235
					updateDCMotorSpeed(speed-50,0); //90//60
					lastDirection = 0;
					return 0;
				}else{
					updateDCMotorSpeed(speed+100,0); //250
					updateDCMotorSpeed(speed-55,1); //85//65
					lastDirection = 1;
					return 180;
				}
			}else if(status == 4) {
				if(HAL_GetTick() - outTick >=900 || mid>130) {
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);
					status = 0;
					lastDirection = lastDirection?0:1;
					if(lastDirection) {
						return 0;
					}else{
						return 180;
					}
					return 0;
				}
				if(lastDirection) {
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_SET);
					updateDCMotorSpeed(speed+100,1);
					updateDCMotorSpeed(speed+100,0);
					return 0;
				}else{
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);
					updateDCMotorSpeed(speed+100,0);//250
					updateDCMotorSpeed(speed+100,1);
					return 180;//250
				}
			}
			return 100;

}

uint8_t testLine(uint16_t *img, uint8_t currDeg) {
	static uint8_t outing = 0;
	static uint8_t lastMid = 160;
	static uint32_t stopTick = 0;
	static uint32_t outTick = 0;
	static uint8_t lastDirection = 0; // 0->left, 1->right
	uint8_t cnt1 = 0;
	uint8_t cnt2 = 0;
	uint8_t mid = 0;
	tft_prints(0,1,"%d",cnt1);
	tft_prints(0,2,"%d",mid);
	tft_prints(0,3,"%d",cnt2);
	for(uint8_t j = 0; j<120; j+=10) {
		for(uint8_t i = 0; i<160; i++) {
			if(*(img+indexCal(i,j))==0xffff){
				break;
			}else{
				if(j==100) {
					cnt2++;
					*(img+indexCal(i,j)) = 0xffff;
				}else if(j==20){
					cnt1++;
					*(img+indexCal(i,j)) = 0xffff;
				}else if(j==60){
					mid++;
					*(img+indexCal(i,j)) = 0xffff;
				}else{
					*(img+indexCal(i,j)) = 255;
				}
			}
		}
	}
	static uint8_t status = 0;
	//updateDCMotorSpeed(150,2);
	tft_prints(0,1,"%d",cnt1);
	tft_prints(0,2,"%d",mid);
	tft_prints(0,3,"%d",cnt2);
	/*updateDCMotorSpeed(160,2);
	if(cnt1>=110 && cnt1<=130 && cnt2>=110 && cnt2<=130 && mid>=110 && mid<=130 ){
		outing = 1;
		outTick = HAL_GetTick();
	}
	if(outing && HAL_GetTick()-outTick<=500) {
		updateDCMotorSpeed(230,1);
		updateDCMotorSpeed(0,0);
		return 180;
	}else if(outing && HAL_GetTick() - outTick >=500) {
		outing = 0;
	}
	if(mid<70) {
		if(cnt1<cnt2) {
			updateDCMotorSpeed(250,1);
			updateDCMotorSpeed(80,0);
			lastDirection = 0;
			return 0;
		}else{
			updateDCMotorSpeed(250,0);
			updateDCMotorSpeed(80,1);
			lastDirection = 1;
			return 180;
		}
	}else if(mid<100) {
		if(cnt1<cnt2) {
			updateDCMotorSpeed(150,0);
			updateDCMotorSpeed(165,1);
			lastDirection = 0;
			return 0;
		}else{
			updateDCMotorSpeed(165,0);
			updateDCMotorSpeed(150,1);
			lastDirection = 1;
			return 180;
		}
	}if(mid<140) {
		if(cnt1<cnt2) {
			return 65;
		}else{
			return 140;
		}
	}else{
		if(cnt1<=135) {
			return 60-cnt1/3;
		}
		if(cnt2<=150){
			return 120+60-cnt2/3;
		}
	}
	return 100;*/
	static uint8_t tmpBackward = 0;
	if(mid<10) {
		tmpBackward = 1;

	}
	if(tmpBackward){
		if(mid>40) {
			updateDCMotorSpeed(0,2);
			tmpBackward = 0;
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);

		}else{
		updateDCMotorSpeed(170,2);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_SET);
		}
		return currDeg;
	}

	if(status == 0) {
			updateDCMotorSpeed(150,2);
			tft_prints(0,0,"0");
			if(mid<=150) {
				status = 1;
				return 90;
			}
			if(cnt1<=135) {
				return 70-cnt1/3;
			}
			if(cnt2<=150){
				return 120+60-cnt2/3;
			}
		}else if(status ==1) {
			updateDCMotorSpeed(150,2);
			tft_prints(0,0,"1");
			if(cnt1>=110 && cnt1<=130 && cnt2>=110 && cnt2<=130 && mid>=110 && mid<=130 ){
				status = 4;
				outTick = HAL_GetTick();
				return 90;
			}
			if(mid>155) {
				status = 0;
				return 90;
			}
			if(mid<=90) {
				stopTick = HAL_GetTick();
				status = 2;
			}
			if(cnt1<cnt2) {
				return 60;
			}else{
				return 140;
			}

		}else if(status == 2){
			tft_prints(0,0,"2");
			//updateDCMotorSpeed(0,2);
			if(HAL_GetTick() - stopTick>=200) {
				status = 3;
			}
			if(cnt1<cnt2) {
				updateDCMotorSpeed(170,1);
				updateDCMotorSpeed(150,0);
				return 0;
			}else{
				return 180;
			}

		}else if(status == 3){
			tft_prints(0,0,"3");
			if(mid>=60) {
						status = 0;

						return 90;
					}
			if(cnt1<cnt2) {
				updateDCMotorSpeed(250,1);
				updateDCMotorSpeed(85,0);
				lastDirection = 0;
				return 0;
			}else{
				updateDCMotorSpeed(250,0);
				updateDCMotorSpeed(85,1);
				lastDirection = 1;
				return 180;
			}
		}else if(status == 4) {
			if(HAL_GetTick() - outTick >=1000 || mid>130) {
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
								HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
								HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
								HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);
				status = 0;
				lastDirection = lastDirection?0:1;
				return 0;
			}
			if(lastDirection) {
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_SET);
				updateDCMotorSpeed(250,1);
				updateDCMotorSpeed(250,0);
				return 0;
			}else{
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET);
												HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
												HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
												HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);
				updateDCMotorSpeed(250,0);
				updateDCMotorSpeed(250,1);
				return 180;
			}
		}
		return 100;


/*if(status == 0) {
		updateDCMotorSpeed(155,2);
		tft_prints(0,0,"0");
		if(mid<=130) {
			status = 1;
			return 90;
		}
		if(cnt1<=135) {
			return 60-cnt1/3;
		}
		if(cnt2<=150){
			return 120+60-cnt2/3;
		}
	}else if(status ==1) {
		updateDCMotorSpeed(150,2);
		tft_prints(0,0,"1");
		if(cnt1>=110 && cnt1<=130 && cnt2>=110 && cnt2<=130 && mid>=110 && mid<=130 ){
			status = 4;
			outTick = HAL_GetTick();
			return 90;
		}
		if(mid>155) {
			status = 0;
			return 90;
		}
		if(mid<=90) {
			stopTick = HAL_GetTick();
			status = 2;
		}
		if(cnt1<cnt2) {
			return 65;
		}else{
			return 140;
		}

	}else if(status == 2){
		tft_prints(0,0,"2");
		//updateDCMotorSpeed(0,2);
		if(HAL_GetTick() - stopTick>=200) {
			status = 3;
		}
		if(cnt1<cnt2) {
			return 0;
		}else{
			return 180;
		}

	}else if(status == 3){
		tft_prints(0,0,"3");
		if(mid>=75) {
					status = 0;

					return 90;
				}
		if(cnt1<cnt2) {
			updateDCMotorSpeed(250,1);
			updateDCMotorSpeed(80,0);
			lastDirection = 0;
			return 0;
		}else{
			updateDCMotorSpeed(250,0);
			updateDCMotorSpeed(80,1);
			lastDirection = 1;
			return 180;
		}
	}else if(status == 4) {
		if(HAL_GetTick() - outTick >=1200 || mid>145) {
			status = 0;
			return 90;
		}
		if(lastDirection) {
			return 0;
		}else{
			updateDCMotorSpeed(250,0);
			updateDCMotorSpeed(40,1);
			return 180;
		}
	}
	return 100;*/

	/*
	static uint8_t status = 0; // 0->straight, 1->waiting to turn, 2->turning
	updateDCMotorSpeed(150,2);
	tft_prints(0,1,"%d",cnt1);
	tft_prints(0,2,"%d",mid);
	tft_prints(0,3,"%d",cnt2);
	if(status == 0) {
		tft_prints(0,0,"0");
		if(mid<=145) {
			status = 1;
			lastDetectMidTick = HAL_GetTick();
			return 90;
		}
		if(cnt1>100 && cnt1<=120) {
			return 70;
		}
		if(cnt1>=80 && cnt1<=100) {
			return 60;
		}
		if(cnt1>=60 && cnt1<=80) {
			return 50;
		}
		if(cnt1>=40 && cnt1<=60) {
					return 40;
				}
		if(cnt1>=30 && cnt1<=40) {
					return 30;
				}


		if(cnt2>100 && cnt2<=120) {
			return 120;
		}
		if(cnt2>=80 && cnt2<=100) {
			return 130;
		}
		if(cnt2>=60 && cnt2<=80) {
			return 140;
		}if(cnt2>=50 && cnt2<=60) {
			return 150;
		}if(cnt2>=40 && cnt2<=50) {
			return 160;
		}

	}else if(status == 1) {
		tft_prints(0,0,"1");
		if(mid<=140 && HAL_GetTick() - lastDetectMidTick >=100) {
			status = 2;
			lastTurnTick = HAL_GetTick();
		}
		if(mid>=150) {
			status = 0;
		}
	}else{
		tft_prints(0,0,"2");
		//if(mid>=150 && finishTurning){
			if(mid>=135 && finishTurning){

			finishTurning = 0;
			status = 0;
			return 90;
		}else if(mid>=130){
			finishTurning = 1;
			return turnDirection?50:120;
		}else {
			finishTurning = 0;
			lastTurnTick = 0;
		}
		if(cnt1<cnt2) {

			updateDCMotorSpeed(120,0);
			updateDCMotorSpeed(230,1);
			return 0;
		}else{
			if(cnt2<20){
				updateDCMotorSpeed(250,0);
				updateDCMotorSpeed(0,1);
			}else if(cnt2<35){
				updateDCMotorSpeed(250,0);
				updateDCMotorSpeed(60,1);
			}else if(cnt2<50){
				updateDCMotorSpeed(250,0);
							updateDCMotorSpeed(100,1);
			}else{
				updateDCMotorSpeed(230,0);
							updateDCMotorSpeed(120,1);
			}

			return 180;
		}
	}*/

	//uint8_t s1 = detectSensor(img, 10, 20, 15, 100);
	/*if(!boosting && ((cnt1>cnt2 && cnt1-cnt2>30) || (s1>30))) {
		if(cnt1>cnt2) {
			boostDirection = 0;
		}else{
			boostDirection = 1;
		}
		boosting = 1;
		lastBoostTick = HAL_GetTick();
	}
	if(boosting) {
		if(HAL_GetTick() - lastBoostTick>=1000 ) {
			boosting = 0;
			updateDCMotorSpeed(150,2);
		}else{
			if(boostDirection) {
				updateDCMotorSpeed(230,0);
				updateDCMotorSpeed(140,1);
				return 180;
			}else{
				updateDCMotorSpeed(140,0);
				updateDCMotorSpeed(230,1);
				return 0;
			}
		}

	}else{
		updateDCMotorSpeed(150,2);
	}*/

	//if(cnt2<cnt1) {
		/*if(cnt2<90) {
				updateDCMotorSpeed(230,0);
				updateDCMotorSpeed(100,1);
				return 90+(90-cnt2);
		}*/
		/*	}
	}else{
		if(cnt1<90){
			updateDCMotorSpeed(100,0);
							updateDCMotorSpeed(230,1);
							return (cnt1);
		}

	}*/

	/*if(mid<60) {
		if(cnt1<cnt2) {
			return 180;
		}else{
			return 0;
		}
	}

	if(cnt1<cnt2) {
		if((cnt2-cnt1)*2>90){
			return 0;
		}
		return 90-(cnt2-cnt1)*2;

	}else{
		if((cnt1-cnt2)>90) {
			return 180;
		}
		return 90+(cnt1-cnt2)*2;
	}*/


	/*if(cnt1>120 && cnt2>120) {
		return 50;
	}
	if(cnt1>40 && cnt1<85 && cnt2>40 && cnt2<85 && cnt1>cnt2){
		return 70;
	}
	if(cnt1<20 && cnt2<45 && cnt2>cnt1 && cnt2-cnt1>10) {

		updateDCMotorSpeed(230,0);
						updateDCMotorSpeed(100,1);
		return 180;
	}

		if(cnt1>cnt2) {
			if((cnt1-cnt2)*3>90) return 0;
			return 90-(cnt1-cnt2)*3;
		}else if(cnt1<cnt2) {
			return 90+10+(cnt2-cnt1)*3;
		}

*/

	//uint8_t s1 = detectSensor(img, 0, 0, 30, 30);
	//uint8_t s2 = detectSensor(img, 100, 80, 120, 120);
	/*
	 *  if(cnt1>45 && cnt1<85 && cnt2>45 && cnt2<85){
		return 70;
	}
	if(cnt1<20 && cnt2<45 && cnt2>cnt1 && cnt2-cnt1>10) {
		return 180;
	}

	if(cnt1>cnt2) {
		if((cnt1-cnt2)*3>90) return 0;
		return 90-(cnt1-cnt2)*3;
	}else if(cnt1<cnt2) {
		return 90+10+(cnt2-cnt1)*3;
	}

	 */
	/*if(cnt1<25 && cnt2<30) {

		return 180;
	}
	if(cnt1<25 && cnt2<25) {
		updateDCMotorSpeed(230,0);
						updateDCMotorSpeed(145,1);
			return 180;
		}
	if(cnt1<25 && cnt2<50) {
		return 180;
	}
	if(cnt1>40 && cnt1<60 && cnt2>40 && cnt2<80) {
			return 70;
		}
	if(cnt1>40 && cnt1<60) {
		return 140;
	}
	if(cnt1<40 && cnt2<40) {
		return 90+(50-cnt1)-(50-cnt2);
	}
	if(cnt1>140) {
		updateDCMotorSpeed(145,0);
				updateDCMotorSpeed(230,1);
		return 0;
	}
	if(cnt1>60) {
			return 70;
		}

	if(cnt1>75) {
		return 30;
	}
	if(cnt1>90) {
				return 0;
			}
	if(cnt1>100) {
		return 0;
	}


	return 90;

	}*/

	return 180;
}

void testNewF(uint16_t *img) {
	for(uint8_t i = 0; i<3; i++){
		detectSensor(img, i*40, 40,i*40+40,80 );
	}
}

uint8_t lineDetection(uint16_t *img, uint8_t width, uint8_t height){
	static uint8_t pixelLimit = 40;
	uint8_t cnt[9] = {0};
	//for(uint8_t i = 0; i<10; i++){
		cnt[0] = detectSensor(img, 0, 0, 40, 40);
		cnt[1] = detectSensor(img, 0, 40, 40, 80);
		cnt[2] = detectSensor(img, 0, 80, 40, 120);
		cnt[3] = detectSensor(img, 40, 0, 80, 40);
		cnt[4] = detectSensor(img, 40, 40, 80, 80);
		cnt[5] = detectSensor(img, 40, 80, 80, 120);
		cnt[6] = detectSensor(img, 80, 0, 120, 40);
		cnt[7] = detectSensor(img, 80, 40, 120, 80);
		cnt[8] = detectSensor(img, 80, 80, 120, 120);


		// 8 7 6
		// 5 4 3
		// 2 1 0

		// 0 1 2
		uint8_t tmp[3] = {0,0,0};
		if(cnt[0]>=pixelLimit) {
			tmp[2] = 1;
		}else if(cnt[3]>=pixelLimit) {
			tmp[2] = 2;
		}else if(cnt[6]>=pixelLimit) {
			tmp[2] = 3;
		}else if(cnt[9]>=pixelLimit){
			tmp[2] = 4;
		}else{
			tmp[2] = 0;
		}

		if(cnt[1]>=pixelLimit) {
			tmp[1] = 1;
		}else if(cnt[4]>=pixelLimit) {
			tmp[1] = 2;
		}else if(cnt[7]>=pixelLimit) {
			tmp[1] = 3;
		}else if(cnt[10]>=pixelLimit){
			tmp[1]=4;
		}else{
			tmp[1] = 0;
		}

		if(cnt[2]>=pixelLimit) {
			tmp[0] = 1;
		}else if(cnt[5]>=pixelLimit) {
			tmp[0] = 2;
		}else if(cnt[8]>=pixelLimit) {
			tmp[0] = 3;
		}else if(cnt[11]>=pixelLimit){
			tmp[0] = 4;
		}else{
			tmp[0] = 0;
		}

		uint8_t slope = 0;
		uint8_t direction = 0; //0 -> not sure, 1 -> down, 2->up
		if(tmp[0]<tmp[2]) {
			updateServoMotor(140);
		}


	return 90;
}



uint8_t straightLine(uint16_t *img, uint8_t width, uint8_t height){

	uint8_t cnt[4] = {0,0,0,0};
	for(uint8_t i = 0; i<30; i++) {
			for(uint8_t j = 0; j<width; j++) {
				if(cnt[0]>40){
					break;
				}
				if(*(img+indexCal(i,j))==0xffff){
					cnt[0]++;
				}else{
					*(img+indexCal(i,j)) = 255;
				}
			}
		}
	for(uint8_t i = 30; i<60; i++) {
			for(uint8_t j = 0; j<width; j++) {
				if(cnt[1]>40){
									break;
								}
				if(*(img+indexCal(i,j))==0xffff){
					cnt[1]++;
				}else{
					*(img+indexCal(i,j)) = 500;
				}

			}
		}
	for(uint8_t i = 60; i<90; i++) {
				for(uint8_t j = 0; j<width; j++) {
					if(cnt[2]>40){
										break;
									}
					if(*(img+indexCal(i,j))==0xffff){
						cnt[2]++;
					}else{
						*(img+indexCal(i,j)) = 1000;
					}

				}
			}
	for(uint8_t i = 90; i<120; i++) {
				for(uint8_t j = 0; j<width; j++) {
					if(cnt[3]>40){
										break;
									}
					if(*(img+indexCal(i,j))==0xffff){
						cnt[3]++;
					}else{
						*(img+indexCal(i,j)) = 1500;
					}

				}
			}
	/*for(uint8_t i = height/2; i<height; i++) {
			for(uint8_t j = 0; j<width; j++) {
				if(*(img+indexCal(i,j))==0xffff){
					cnt2++;
				}
			}
		}*/
	//turnServoMotor(90);
	if(cnt[0]>40) {
		//turnServoMotor(170);
		//degree = 170;
		//updateDCMotorSpeed(160,0);
		//updateDCMotorSpeed(150,1);
		return 170;
		//TIM5->CCR1 = 980;
		//
	}else if(cnt[1]>50){
		//turnServoMotor(140);
		//updateDCMotorSpeed(155,0);
		//updateDCMotorSpeed(150,1);
		return 140;
		//degree = 140;
		//TIM5->CCR1 = 700;
		//updateDCMotorSpeed(205,2);
	}else if(cnt[2]>50){
		//turnServoMotor(60);
		//updateDCMotorSpeed(150,0);
		//updateDCMotorSpeed(155,1);
		return 60;
		//degree = 60;
		//TIM5->CCR1 = 500;
		//updateDCMotorSpeed(205,2);
	}else if(cnt[3]>50){
		//turnServoMotor(40);
		//updateDCMotorSpeed(150,0);
		//updateDCMotorSpeed(160,1);
		return 40;
		//degree = 40;
		//TIM5->CCR1 = 400;
		//updateDCMotorSpeed(205,2);
	}
	//updateDCMotorSpeed(150,0);
	//updateDCMotorSpeed(165,1);
	return 0;


}

uint8_t* pathFinder(uint16_t *img, uint8_t width, uint8_t height) {
	led_off(LED1);
	led_off(LED2);
	led_off(LED3);
	led_off(LED4);
	uint8_t startHeight = 20;
	bool sensor[6] = {0};
	uint8_t cnt = 0;
	static uint32_t lastTurnTick = 0;
	static uint8_t lastSensor = 5;
	for(uint8_t i = startHeight+85; i<startHeight+90; i++){
		for(uint8_t j = 105; j<110; j++) {
			if(*(img+indexCal(i,j))==0xffff){
				cnt++;
			}
			*(img+indexCal(i,j)) = 255;
		}
	}
	if(cnt>=5) {
		sensor[0]=1;
	}
	cnt= 0 ;
	for(uint8_t i = startHeight+50; i<startHeight+55; i++){
			for(uint8_t j = 105; j<110; j++) {
				if(*(img+indexCal(i,j))==0xffff){
					cnt++;
				}
				*(img+indexCal(i,j)) = 255;
			}
		}
	if(cnt>=5) {
			sensor[1]=1;
		}
	cnt= 0 ;
	for(uint8_t i = startHeight+85; i<startHeight+90; i++){
			for(uint8_t j = 60; j<65; j++) {
				if(*(img+indexCal(i,j))==0xffff){
					cnt++;
				}
				*(img+indexCal(i,j)) = 255;
			}
		}
	cnt= 0 ;
	if(cnt>=5) {
			sensor[2]=1;
		}
	for(uint8_t i = startHeight+50; i<startHeight+55; i++){
				for(uint8_t j = 60; j<65; j++) {
					if(*(img+indexCal(i,j))==0xffff){
						cnt++;
					}
					*(img+indexCal(i,j)) = 255;
				}
			}
	if(cnt>=5) {
			sensor[3]=1;
		}
		cnt= 0 ;
		for(uint8_t i = startHeight+85; i<startHeight+90; i++){
					for(uint8_t j = 20; j<25; j++) {
						if(*(img+indexCal(i,j))==0xffff){
							cnt++;
						}
						*(img+indexCal(i,j)) = 255;
					}
				}
		if(cnt>=5) {
				sensor[4]=1;
			}
			cnt= 0 ;
			for(uint8_t i = startHeight+50; i<startHeight+55; i++){
						for(uint8_t j = 20; j<25; j++) {
							if(*(img+indexCal(i,j))==0xffff){
								cnt++;
							}
							*(img+indexCal(i,j)) = 255;
						}
					}
			if(cnt>=5) {
					sensor[5]=1;
				}
				cnt= 0 ;

	if(sensor[0]==1 && sensor[5]==1) {
		turnServoMotor(140);
	}else if(sensor[1]==1 && sensor[4]==1){
		turnServoMotor(40);
	}
	//turnServoMotor(120);
	return 0;
}

void autoBrightness(uint16_t *img, uint16_t width, uint16_t height) {
	//uint16_t samplePixel = 10000;
	uint16_t shift = 3000;
	for(uint16_t i = 0; i<width*height; i++) {
		if(*(img+i)<0xffff-shift){
			*(img+i) += shift;
		}
	}
}

void autoContrast(uint16_t *img, uint16_t width, uint16_t height) {
	//uint16_t samplePixel = *(img+height/2*width+width/2);
	uint16_t shift = 9000;
	for(uint16_t i = 0; i<width*height; i++) {
		uint16_t pixel = *(img+i);
		if(pixel>0xffff/2){
			if(*(img+i)<0xffff-shift){
				*(img+i) = pixel+shift;
			}else{
				*(img+i) = 0xffff;
			}
		}else if(pixel<0xffff/2){
			if(pixel>shift){
				*(img+i) = pixel-shift;
			}else{
				*(img+i) = 0;
			}
		}
	}
}

void change_contrast(uint16_t* Image, uint16_t height, uint16_t width, uint16_t contrast){
    for(uint16_t i = 0; i < height; i++) {
        for (uint16_t j = 0; j < width; j++) {
            uint16_t value = (*(Image + width*i + j) - 128) * contrast + 128;
            if(0 <= value <= 255){
                *(Image + width*i + j) = value;
            }
            if(value >= 255){
                *(Image + width*i + j) = 255;
            }
            if(0 >= value){
                *(Image + width*i + j) = 0;
            }

        }
    }
}


// the auto correct function
void autocorrect(uint16_t* Image, uint16_t height, uint16_t width){
    uint16_t shift =  128 - *(Image + height*height/2 + width/2); // the center pixel of the image
    change_brightness(Image, height, width, shift); // change brightness to around 128
    uint16_t max = 0;
    for(uint16_t i = 0; i < height; i++) {
        for (uint16_t j = 0; j < width; j++) {
            if(*(Image + width*i + j) > max){
                max = *(Image + width*i + j);
            }
        }
    }
    float contrast = 127/(max-128); // maximize contrast
    change_contrast(Image, height, width, contrast);
}
