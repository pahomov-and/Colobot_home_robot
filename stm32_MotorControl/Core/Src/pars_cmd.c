#include "stm32f1xx_hal.h"

#include "pars_cmd.h"
#include "queue_char.h"


/**
 * cmd:
 *
 * [0] 0xAx - motor right,left on
 * 		1010 xx01b - motor right run left
 * 		1010 xx10b - motor right run right
 * 		1010 xx00b - motor right run stop
 * 		1010 xx11b - motor right run stop
 *
 * 		1010 01xxb - motor left run left
 * 		1010 10xxb - motor left run right
 * 		1010 00xxb - motor right run stop
 * 		1010 00xxb - motor right run stop
 *
 * [0] 0xBx - motor right set power
 *		1011 xxxxb - xx 0000..1111
 *
 * [0] 0xCx - motor left set power
 *		1100 xxxxb - xx 0000..1111
 *
 *
 *
 */

GPIO_TypeDef *gpio_motor = NULL;
UART_HandleTypeDef *uart_cmd = NULL;

uint16_t pin0_motor_right;
uint16_t pin1_motor_right;
uint16_t pin0_motor_left;
uint16_t pin1_motor_left;

bool isRun(char cmd) {
	bool ret = false;
	switch(cmd & 0x03) {
	case 0x01: // motor right run left
		HAL_GPIO_WritePin(gpio_motor, pin0_motor_right, GPIO_PIN_SET);
		HAL_GPIO_WritePin(gpio_motor, pin1_motor_right, GPIO_PIN_RESET);
		ret = true;
		break;
	case 0x02: // motor right run right
		HAL_GPIO_WritePin(gpio_motor, pin0_motor_right, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(gpio_motor, pin1_motor_right, GPIO_PIN_SET);
		ret = true;
		break;

	default: // motor right run stop
		HAL_GPIO_WritePin(gpio_motor, pin0_motor_right, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(gpio_motor, pin1_motor_right, GPIO_PIN_RESET);
		break;
	}

	switch(cmd & 0x0C) {
	case 0x04: // motor left run left
		HAL_GPIO_WritePin(gpio_motor, pin0_motor_left, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(gpio_motor, pin1_motor_left, GPIO_PIN_SET);
		ret = true;
		break;
	case 0x08: // motor left run right
		HAL_GPIO_WritePin(gpio_motor, pin0_motor_left, GPIO_PIN_SET);
		HAL_GPIO_WritePin(gpio_motor, pin1_motor_left, GPIO_PIN_RESET);
		ret = true;
		break;

	default: // motor right run stop
		HAL_GPIO_WritePin(gpio_motor, pin0_motor_left, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(gpio_motor, pin1_motor_left, GPIO_PIN_RESET);
		break;
	}

	return ret;
}


void processing_cmd(struct queue_char *qchar) {
	if(queue_char_is_empty(qchar)) return;

	while(!queue_char_is_empty(qchar)) {
		char cmd = char_pop(qchar);
		switch(cmd & CMD_MASK) {
		case CMD_RUN:
			isRun(cmd);
			break;
		case CMD_POWER_RIGHT:
			break;
		case CMD_POWER_LEFT:
			break;

		default:
			break;
		}
	}

}

void processing_init(UART_HandleTypeDef *uart,
		GPIO_TypeDef *gpio,
		uint16_t pin0_mr, uint16_t pin1_mr,
		uint16_t pin0_ml, uint16_t pin1_ml) {
	uart_cmd = uart;
	gpio_motor = gpio;

	pin0_motor_right = pin0_mr;
	pin1_motor_right = pin1_mr;
	pin0_motor_left = pin0_ml;
	pin1_motor_left = pin1_ml;

}
