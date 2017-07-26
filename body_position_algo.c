#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "body_position_algo.h"

#define FILTER_TIMES                                                 5           //filter times 
#define FILTER_LEN                                                   5           //the size of filter windows 
#define CAPTURE_SIZE                                                 50          //the size of capture body position 
#define EVERY_VARIABLE_FEATURE_NUM                                   3           // sum  features numbers 
 
#define DIFF_BODYPOSITION_AMPLITUDEVALUE_THRESHODVALUE               20000.00    // the threshold value of body position amplitudeValue   
#define STAND_BODYPOSITION_THRESHOD_VALUE                           -11042.00f   //the threshold value of stand body position
#define BACK_BODYPOSITION_THRESHOD_VALUE                             11788.00f   //the threshold value of back body position 
#define LEFT_BODYPOSITION_THRESHOD_VALUE                             11714.00f   //the threshold value of left body position 
#define FRONT_BODYPOSITION_THRESHOD_VALUE                           -11743.60f   //the threshold value of front body position 
#define RIGHT_BODYPOSITION_THRESHOD_VALUE                           -11783.00f   //the threshold value of right body position


typedef struct rev_data_s
{
	float* gx_movingfilter_data;                                                 //save movingfilter data
	float* gy_movingfilter_data; 
	float* gz_movingfilter_data;
	
	float feature_value[EVERY_VARIABLE_FEATURE_NUM];                            //save feature value 
	float former_feature_value[EVERY_VARIABLE_FEATURE_NUM];                     //save former feature value
	
	float diff_body_position_amplitude_value;                                   //body position amplitude value 
	int precise_body_position_style;	                                          //precise body position style
	int body_position_amplitude_rank;
	                                                                            //body position amplitude rank	 
}rev_data_t;                                                                    //define a struct for receiving data


//{create and initialize working space: start}--------------------
//{get size of working space}
int bp_get_size_of_space() 
{
	int size;
	
	size = sizeof(rev_data_t);
	
	return size;
}

//{create and initialize working space}
unsigned char *bp_create_and_initialize_space() 
{
	unsigned char *space;

	int size = bp_get_size_of_space();

	space = (unsigned char *)malloc(size);

	memset(space, 0, size);

	bp_initialize_space(space);

	return space;
}

//{initialize working space}
void bp_initialize_space(unsigned char *space) 
{
	rev_data_t *rev_data_p = (rev_data_t*)space;
	int i;

	rev_data_p->precise_body_position_style = 0;
	rev_data_p->diff_body_position_amplitude_value = 0.0;  
	rev_data_p->body_position_amplitude_rank = 0;

	for(i = 0; i < EVERY_VARIABLE_FEATURE_NUM; i++)
	 {
	 	rev_data_p->feature_value[i] = 0.0;
	 	rev_data_p->former_feature_value[i] = 0.0;
	 }

    rev_data_p->gx_movingfilter_data = (float*)malloc(sizeof(float) * CAPTURE_SIZE);
    rev_data_p->gy_movingfilter_data = (float*)malloc(sizeof(float) * CAPTURE_SIZE);
    rev_data_p->gz_movingfilter_data = (float*)malloc(sizeof(float) * CAPTURE_SIZE);

}
//{create and initialize working space: end}----------------------


void free_point(unsigned char *space)
{
	rev_data_t* rev_data_p = (rev_data_t*)space;
	
	free(rev_data_p->gx_movingfilter_data);
	free(rev_data_p->gy_movingfilter_data);
	free(rev_data_p->gz_movingfilter_data);
}


static float comput_mean_value(float *data_vector , int start , int len)
{
   float sum = 0.0;
   int index = 0;
   
   for(index = start ; index < start + len ; index++)
   {
   	  sum = sum + data_vector[index];
   }
   
   return(sum/len);
	
}


static void multiply_moving_filter(float *initdata, int data_len, float *movingfilter_data) 
{
	int filter_num = 0 , vector_index = 0;
	float *index = (float*)malloc(sizeof(float) * data_len);
	
	for(vector_index = 0; vector_index < data_len; vector_index++)
	{		
		index[vector_index] = initdata[vector_index];  
	}
	
	for(vector_index = 0; vector_index < FILTER_LEN-1; vector_index++)
	{		
		movingfilter_data[vector_index] = index[vector_index];  
	}
	
	for(filter_num = 0; filter_num  < FILTER_TIMES; filter_num++)   
	{ 
		for(vector_index = FILTER_LEN-1; vector_index < CAPTURE_SIZE; vector_index++)
		{ 
			movingfilter_data[vector_index] = comput_mean_value(index, vector_index - FILTER_LEN + 1, FILTER_LEN); 
		}
		
		for(vector_index = 0; vector_index < CAPTURE_SIZE; vector_index++)
		{
			index[vector_index] = movingfilter_data[vector_index]; 
		}
	}
	
	free(index);
		
}


static void  get_feature_value(unsigned char *space)
{
    rev_data_t* rev_data_p = (rev_data_t*)space;
    
	rev_data_p->feature_value[0] = comput_mean_value(rev_data_p->gx_movingfilter_data, 0, CAPTURE_SIZE);//compute mean-value 
	rev_data_p->feature_value[1] = comput_mean_value(rev_data_p->gy_movingfilter_data, 0, CAPTURE_SIZE);///compute mean-value  
	rev_data_p->feature_value[2] = comput_mean_value(rev_data_p->gz_movingfilter_data, 0, CAPTURE_SIZE);///compute mean-value 	
} 

static void copy_feature_value(unsigned char *space)
{
	rev_data_t* rev_data_p = (rev_data_t*)space;  
	int index;
	
	for(index = 0; index < EVERY_VARIABLE_FEATURE_NUM; index++)
	{
		rev_data_p->former_feature_value[index] = rev_data_p->feature_value[index]; 
	}
}


static void get_diff_amplitude_value(unsigned char *space)
{
	rev_data_t* rev_data_p = (rev_data_t*)space;
	int  index;
	
	rev_data_p->diff_body_position_amplitude_value = 0; 
	
	for(index = 0; index < EVERY_VARIABLE_FEATURE_NUM; index++) //compute body position amplitude value 
	{
		rev_data_p->diff_body_position_amplitude_value = rev_data_p->diff_body_position_amplitude_value + pow(rev_data_p->feature_value[index] - rev_data_p->former_feature_value[index],2); 
	}
	rev_data_p->diff_body_position_amplitude_value = sqrt(rev_data_p->diff_body_position_amplitude_value); 
  	
} 


static void amplitude_rank_test(unsigned char *space)
{
	 rev_data_t* rev_data_p = (rev_data_t*)space;
	 float  index;
	 
	 index = rev_data_p->diff_body_position_amplitude_value/DIFF_BODYPOSITION_AMPLITUDEVALUE_THRESHODVALUE;
	 
	 if(index >= 1)
	 {
	 	index = 10.0;
	 }
	 else
	 {
	 	index = pow(10, index)-1; //amplitude_rank test function 
	 } 
	
	 if(index-(int)index < 0.50)
	 {
	 	rev_data_p->body_position_amplitude_rank = (int)index;
	 }
	 else
	 {
	 	rev_data_p->body_position_amplitude_rank = (int)index + 1;
	 }	 
 } 


static void precise_identify_body_position_style(unsigned char *space)
{
   
   rev_data_t* rev_data_p = (rev_data_t*)space;
   
   rev_data_p->precise_body_position_style = 0;
   	
   if(rev_data_p->feature_value[2] <= FRONT_BODYPOSITION_THRESHOD_VALUE)
	{
		
		rev_data_p->precise_body_position_style = 3; //front body position 
		 
	}
	
	if(rev_data_p->feature_value[1] <= RIGHT_BODYPOSITION_THRESHOD_VALUE)
	{
		
		rev_data_p->precise_body_position_style = 4; //right body position 
		 
	}
	
	if(rev_data_p->feature_value[1] >= LEFT_BODYPOSITION_THRESHOD_VALUE)
	{
		
		rev_data_p->precise_body_position_style = 2; //left body position 
		 
	}
	
	if(rev_data_p->feature_value[2] >= BACK_BODYPOSITION_THRESHOD_VALUE)
	{
		
		rev_data_p->precise_body_position_style = 1; // back body position 
		 
	}
	
	if(rev_data_p->feature_value[0] <= STAND_BODYPOSITION_THRESHOD_VALUE)
	{
		
		rev_data_p->precise_body_position_style = 5;  //stand body position 
		 
	}
		
} 

void  capture_body_position_test(float *gx, float *gy, float *gz, int data_len, unsigned char *space)
{
	     
	     rev_data_t* rev_data_p = (rev_data_t*)space;
	     
	     multiply_moving_filter(gx, data_len, rev_data_p->gx_movingfilter_data);
	     multiply_moving_filter(gy, data_len, rev_data_p->gy_movingfilter_data);//movingfilter 
	     multiply_moving_filter(gz, data_len, rev_data_p->gz_movingfilter_data);
	     
	     get_feature_value(space); //get feature value
	     
	   	 precise_identify_body_position_style(space); //precise identify body position style
		  
	   	 get_diff_amplitude_value(space); //get amplitudeValue 
		
	     copy_feature_value(space); //copy feature value
		  
	     amplitude_rank_test(space); // amplitude rank test
}
	
		



int get_precise_body_position_style(unsigned char *space)
{
	rev_data_t* rev_data_p = (rev_data_t*)space; 
	
	return  rev_data_p->precise_body_position_style; 
}


int get_amplitude_rank(unsigned char *space)
{
	rev_data_t* rev_data_p = (rev_data_t*)space; 
	
	return  rev_data_p->body_position_amplitude_rank; 
}
