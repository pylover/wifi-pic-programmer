#ifndef _SP_PIC_H__
#define _SP_PIC_H__


#define GPIO_OUTPUT(n) GPIO_OUTPUT_SET(GPIO_ID_PIN(n), 0)
#define GPIO_INPUT(n) GPIO_DIS_OUTPUT(GPIO_ID_PIN(n))
#define GPIO_SET(n, v) GPIO_OUTPUT_SET(GPIO_ID_PIN(n), v)
#define GPIO_GET(n) GPIO_INPUT_GET(GPIO_ID_PIN(n))

#endif
