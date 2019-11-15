#ifndef _PROGRAMMER_H__
#define _PROGRAMMER_H__


#include <eagle_soc.h>


// VPP(+12v):  D5   GPIO14 
#define VPP_MUX		    PERIPHS_IO_MUX_MTMS_U	
#define VPP_NUM			14
#define VPP_FUNC		FUNC_GPIO14


// VDD:        D4   GPIO2
#define VDD_MUX		    PERIPHS_IO_MUX_GPIO2_U	
#define VDD_NUM			2
#define VDD_FUNC		FUNC_GPIO2


// DATA:        D2  GPIO4
#define DATA_MUX		    PERIPHS_IO_MUX_GPIO4_U	
#define DATA_NUM			4
#define DATA_FUNC		    FUNC_GPIO4


// CLOCK:        D1  GPIO5
#define CLOCK_MUX		    PERIPHS_IO_MUX_GPIO5_U	
#define CLOCK_NUM			5
#define CLOCK_FUNC		    FUNC_GPIO5


int programmer_detect(int *chip);


#endif

