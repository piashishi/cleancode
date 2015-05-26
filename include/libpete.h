/*
 * libpete.h
 *
 *  Created on: Mar 3, 2015
 *      Author: root
 */

#ifndef LIB_PETE_H_
#define LIB_PETE_H_
#include<sys/time.h>
#include <stdint.h>
int64_t  timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y);

#define GET_COST_USEC(expression) \
    do{\
        const char* name = #expression;\
        struct timeval t0, t1, td;\
        int64_t td_usec;\
        gettimeofday(&t0, 0);\
        expression;\
        gettimeofday(&t1, 0);\
        td_usec = timeval_subtract(&td, &t0, &t1);\
        printf("%s usec = %ld\n", name, td_usec);\
}while(0)

#endif /* LIB_PETE_H_ */
