#include "motordriver.h"

#define MOTOR_STOP 0
#define MOTOR_CW_ROTATION 1
#define MOTOR_CCW_ROTATION -1

#define FORWARD_DIR		0
#define BACKWARD_DIR	1

Motor::Motor(PinName pwm, PinName dir):
	_pwm(pwm), _dir(dir)
{
	_pwm.period(0.001); // period = 1 msec
	_pwm.write(0); // _pwm = 0
	
	_dir = FORWARD_DIR;
	_sign = MOTOR_STOP;
}

void Motor::forward(double speed) {
	if (_sign == MOTOR_CCW_ROTATION) {
		_pwm = 0;
		wait(0.2);
	}
	
	
	_dir = FORWARD_DIR;
	_pwm = speed < 0 ? 0 : ((speed > 1.0) ? 1.0 : speed);
	_sign = MOTOR_CW_ROTATION;
}

void Motor::backward(double speed) {
	if (_sign == MOTOR_CCW_ROTATION) {
		_pwm = 0;
		wait(0.2);
	}
	
	
	_dir = BACKWARD_DIR;
	_pwm = speed < 0 ? 0 : ((speed > 1.0) ? 1.0 : speed);
	_sign = MOTOR_CCW_ROTATION;
}

void Motor::stop() {
	_pwm = 0;
	wait(0.1);
	_sign = MOTOR_STOP;
}