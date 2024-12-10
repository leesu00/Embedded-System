#ifndef _MOTORDRIVER_H_
#define _MOTORDRIVER_H_

#include "mbed.h"

class Motor {
	public:
		Motor(PinName pwm, PinName dir);
		void forward(double speed);
		void backward(double speed);
		void stop();
	
	private:
		PwmOut _pwm;
		DigitalOut _dir;
		int _sign;
};

#endif // _MOTORDRIVER_H_