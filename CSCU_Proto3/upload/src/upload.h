#ifndef		__UPLOAD_H__
#define		__UPLOAD_H__


void upload_init(void);
void upload_deinit(void);

void upload_process_record(void);
void upload_real_record(int ac1_num, int ac3_num, int dc_num);


#endif
