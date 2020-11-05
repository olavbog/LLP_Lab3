#ifndef QUERY_IOCTL_H

#define QUERY_IOCTL_H

#include <linux/ioctl.h>

typedef struct
{
	int status, dignity,ego; //Global variables within the driver
} query_arg_t;

#define QUERY_GET_VARIABLES _IOR('q', 1, query_arg_t *)
#define QUERY_CLR_VARIABLES  _IO('q', 2)

#endif