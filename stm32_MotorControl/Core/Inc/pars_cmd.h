/*
 * pars_cmd.h
 *
 *  Created on: Feb 23, 2021
 *      Author: tymbys
 */

#ifndef INC_PARS_CMD_H_
#define INC_PARS_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define CMD_MASK 0xF0

enum CMD {
	CMD_RUN = 0xA0,
	CMD_POWER_RIGHT = 0xB0,
	CMD_POWER_LEFT = 0xC0,
};


//void processing_cmd(UART_HandleTypeDef *huart, struct queue_char *qchar);


#ifdef __cplusplus
}
#endif
#endif /* INC_PARS_CMD_H_ */
