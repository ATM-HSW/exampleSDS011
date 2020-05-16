//------------------------------------
// Feinstaubsensor sds011
//------------------------------------

#include "mbed.h"
#include <sds011.h>


float p10, p25;
int er, id;
RawSerial pr(USBTX, USBRX);

#ifdef TARGET_STM32F767
RawSerial sds1(PC_12,PD_2);
#endif

#ifdef TARGET_STM32F446RE
RawSerial sds1(A0,A1); //PA_0, PA_1
#endif

SDS011 my_sds1;

float fPm25, fPm10;
int bSleep = 0;
int bSleepBtn = 0;
int iFW = 0;
int iID = 0;
int iPeriod;
bool received = false;
int event = 0;

DigitalIn btn(USER_BUTTON);
int btn_old=0;

void fSDS011RxCb(int evnt) {
  fPm25 = my_sds1.getPM25();
  fPm10 = my_sds1.getPM10();
	bSleep = my_sds1.isSleep();
  iFW = my_sds1.getFW();
	iID = my_sds1.getID();
	iPeriod = my_sds1.getPeriod();
	received = true;
	event = evnt;
}

int main() {
  Callback<void(int)> callback(fSDS011RxCb);
  my_sds1.init(&sds1,(fSDS011RxCb)); //RX, TX
  pr.baud(115200);
  pr.printf("Start\n");
  my_sds1.sendCommand(SDS011::Command::FIRMWARE);
    
  while(1) {
		if(received) {
			switch(event) {
				case SDS011::Event::RX_DATA:
					pr.printf("SDS P2.5=%f P10=%f\n", fPm25, fPm10);
				  break;
				case SDS011::Event::RX_SLEEP_WORK:
					pr.printf("SDS sleep=%s\n", bSleep?"no":"yes");
				  break;
				case SDS011::Event::RX_PERIOD:
					pr.printf("SDS period=%d\n", iPeriod);
				  break;
				case SDS011::Event::RX_FIRMWARE:
					pr.printf("SDS fw=%d-%d-%d id=%x\n", (iFW>>16)&0xff, (iFW>>8)&0xff, iFW&0xff, iID);
				  break;
			}
			received=0;
		}

		if(btn_old == 1 && !btn) {
			if(!bSleepBtn) {
			  my_sds1.sendCommand(SDS011::Command::SLEEP);
				//my_sds1.sendCommand(SDS011::Command::PERIOD,1);
			  printf("goto sleep\n");
				bSleepBtn=1;
			} else {
			  my_sds1.sendCommand(SDS011::Command::WAKEUP);
			  printf("goto wakeup\n");
				bSleepBtn=0;
			}
		}
		btn_old = btn;
  }
  return 0;
}
