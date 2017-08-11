#ifndef __TIM3_H_
#define __TIM3_H_

#ifdef __cplusplus
 extern "C" {
#endif
	 

#define JUMP_FIRST()  Send_data(page_fir)
#define JUMP_SECOND() Send_data(page_sec)


void tim3_init(void);
void  TIM3_Handler(void);

#ifdef __cplusplus
}
#endif

#endif




