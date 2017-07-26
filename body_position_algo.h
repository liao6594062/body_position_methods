#ifndef  BODY_POSITION_ALGO_H
#define  BODY_POSITION_ALGO_H



int bp_get_size_of_space(); //get the size of space 

void bp_initialize_space(unsigned char *space); //initialize space

unsigned char *bp_create_and_initialize_space(); //create and initialize space 


//central function: capture_bodyLocationStyle£¬body Position test
//gx¡¢gy£¬gz are three axises gravity£¬the length of gx(gy,gz) is 50, then output a result when adding extra 25 points. 
void  capture_body_position_test(float *gx, float *gy, float *gz, int data_len, unsigned char *space);  


int get_precise_body_position_style(unsigned char *space); //get precise body position style 

int get_amplitude_rank(unsigned char *space); //get amplitude rank 

void FreePoint(unsigned char *space); //free space 


#endif
