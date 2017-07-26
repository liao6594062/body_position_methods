#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "body_position_algo.h"

#define FILTER_TIMES                                                 5           //filter times 
#define FILTER_LEN                                                   5           //the size of filter windows 
#define CAPTURE_SIZE                                                 50          //the size of capture body position 
#define EVERY_VARIABLE_FEATURE_NUM                                   3           // sum  features numbers 
 
#define diff_bodyPosition_amplitudeValue_ThresholdValue              20000.00    // the threshold value of body position amplitudeValue   
#define STAND_BODYPOSITION_THRESHOD_VALUE                           -11042.00f   //the threshold value of stand body position
#define BACK_BODYPOSITION_THRESHOD_VALUE                             11788.00f   //the threshold value of back body position 
#define LEFT_BODYPOSITION_THRESHOD_VALUE                             11714.00f   //the threshold value of left body position 
#define FRONT_BODYPOSITION_THRESHOD_VALUE                           -11743.60f   //the threshold value of front body position 
#define RIGHT_BODYPOSITION_THRESHOD_VALUE                           -11783.00f   //the threshold value of right body position


typedef struct rev_data_s{
	
	float* gx_movingFilterData;                                                 //save movingfilter data
	float* gy_movingFilterData; 
	float* gz_movingFilterData;
	
	float feature_value[EVERY_VARIABLE_FEATURE_NUM];                            //save feature value 
	float former_feature_value[EVERY_VARIABLE_FEATURE_NUM];                     //save former feature value
	
	float diff_body_position_amplitude_value;                                   //body position amplitude value 
	int precise_body_position_style;	                                          //precise body position style
	int bodyPosition_amplitudeValue_rank;                                    //body position amplitude rank
		 
}rev_Data_t;                                                                  //define a struct for receiving data


//{create and initialize working space: start}--------------------
//{get size of working space}
int bp_get_size_of_space() {
	int size;
	size = sizeof(Rev_Data);
	return size;
}

//{create and initialize working space}
unsigned char * bp_create_and_initialize_space() {

	unsigned char* space;

	int size = bp_get_size_of_space();

	space = (unsigned char *)malloc(size);

	memset(space, 0, size);

	bp_initialize_space(space);

	return space;
}

//{initialize working space}
void bp_initialize_space(unsigned char* space) {
	Rev_Data *revData_p = (Rev_Data*)space;
	int i;

	revData_p->precise_bodyPosition_style = 0;
	revData_p->diff_bodyPosition_amplitudeValue = 0.0;  
	revData_p->bodyPosition_amplitudeValue_rank = 0;

	for(i = 0; i < EVERY_VARIABLE_FEATURE_NUM; i ++)
	 {
	 	revData_p->featureValue[i] = 0.0;
	 	revData_p->former_featureValue[i] = 0.0;
	 }

    revData_p->gx_movingFilterData = (float*)malloc(sizeof(float) * CAPTURE_SIZE);
    revData_p->gy_movingFilterData = (float*)malloc(sizeof(float) * CAPTURE_SIZE);
    revData_p->gz_movingFilterData = (float*)malloc(sizeof(float) * CAPTURE_SIZE);


}
//{create and initialize working space: end}----------------------


void FreePoint(unsigned char* space)
{
	Rev_Data* revData_p = (Rev_Data*)space;
	
	free(revData_p->gx_movingFilterData);
	free(revData_p->gy_movingFilterData);
	free(revData_p->gz_movingFilterData);
}


static float   comput_meanValue(float* data_vector , int start , int len)
{
   float sum = 0.0;
   int index = 0;
   
   for(index = start ; index < start + len ; index ++)
   {
   	  sum = sum + data_vector[index];
   }
   
   return(sum/len);
	
}


static void   multiply_moving_filter(float* initdata, int data_len, float* movingFilterData) 
{
	int filter_num = 0 , vector_index = 0;
	float*  index = (float*)malloc(sizeof(float) * data_len);
	
	for(vector_index = 0 ; vector_index < data_len ; vector_index ++)
	{		
		index[vector_index] = initdata[vector_index];  
	}
	
	for(vector_index = 0 ; vector_index < FILTER_LEN-1 ; vector_index ++)
	{		
		movingFilterData[vector_index] = index[vector_index];  
	}
	
	for(filter_num = 0 ; filter_num < FILTER_TIMES ; filter_num ++)   
	{ 
		for(vector_index = FILTER_LEN-1 ; vector_index < CAPTURE_SIZE ; vector_index ++)
		{ 
			movingFilterData[vector_index] = comput_meanValue(index , vector_index - FILTER_LEN + 1 , FILTER_LEN); 
		}
		
		for(vector_index = 0 ; vector_index < CAPTURE_SIZE ; vector_index ++)
		{
			index[vector_index] = movingFilterData[vector_index]; 
		}
	}
	
	free(index);
		
}



static void  getFeatureValue(unsigned char* space)
{
    Rev_Data* revData_p = (Rev_Data*)space;
    
	revData_p->featureValue[0] = comput_meanValue(revData_p->gx_movingFilterData , 0 , CAPTURE_SIZE);//compute mean-value 
	revData_p->featureValue[1] = comput_meanValue(revData_p->gy_movingFilterData , 0 , CAPTURE_SIZE);///compute mean-value  
	revData_p->featureValue[2] = comput_meanValue(revData_p->gz_movingFilterData , 0 , CAPTURE_SIZE);///compute mean-value 	
} 

static void copyFeatureValue(unsigned char* space)
{
	Rev_Data* revData_p = (Rev_Data*)space;  
	int index;
	
	for(index = 0; index < EVERY_VARIABLE_FEATURE_NUM; index ++)
	{
		revData_p->former_featureValue[index] = revData_p->featureValue[index]; 
	}
}


static void getDiffAmplitudeValue(unsigned char* space)
{
	Rev_Data* revData_p = (Rev_Data*)space;
	int  index;
	
	revData_p->diff_bodyPosition_amplitudeValue = 0; 
	
	for(index = 0; index < EVERY_VARIABLE_FEATURE_NUM; index ++ ) //compute body position amplitude value 
	{
		revData_p->diff_bodyPosition_amplitudeValue = revData_p->diff_bodyPosition_amplitudeValue + pow(revData_p->featureValue[index] - revData_p->former_featureValue[index],2); 
	}
	revData_p->diff_bodyPosition_amplitudeValue = sqrt(revData_p->diff_bodyPosition_amplitudeValue); 
  	
} 


static void  amplitudeRankTest(unsigned char* space)
{
	 Rev_Data* revData_p = (Rev_Data*)space;
	 float  index;
	 
	 index = revData_p->diff_bodyPosition_amplitudeValue/diff_bodyPosition_amplitudeValue_ThresholdValue;
	 
	 if(index >= 1)
	 {
	 	index = 10.0;
	 }
	 else
	 {
	 	index = pow(10,index)-1; //amplitude_rank test function 
	 } 
	
	 if(index-(int)index < 0.50)
	 {
	 	revData_p->bodyPosition_amplitudeValue_rank = (int)index;
	 }
	 else
	 {
	 	revData_p->bodyPosition_amplitudeValue_rank = (int)index + 1;
	 }	 
 } 


static void   precise_identify_bodyPosition_style(unsigned char* space)
{
   
   Rev_Data* revData_p = (Rev_Data*)space;
   
   revData_p->precise_bodyPosition_style = 0;
   	
   if(revData_p->featureValue[2] <= FRONT_BODYPOSITION_THRESHOD_VALUE)
	{
		
		revData_p->precise_bodyPosition_style = 3; //front body position 
		 
	}
	
	if(revData_p->featureValue[1] <= RIGHT_BODYPOSITION_THRESHOD_VALUE)
	{
		
		revData_p->precise_bodyPosition_style = 4; //right body position 
		 
	}
	
	if(revData_p->featureValue[1] >= LEFT_BODYPOSITION_THRESHOD_VALUE)
	{
		
		revData_p->precise_bodyPosition_style = 2; //left body position 
		 
	}
	
	if(revData_p->featureValue[2] >= BACK_BODYPOSITION_THRESHOD_VALUE)
	{
		
		revData_p->precise_bodyPosition_style = 1; // back body position 
		 
	}
	
	if(revData_p->featureValue[0] <= STAND_BODYPOSITION_THRESHOD_VALUE)
	{
		
		revData_p->precise_bodyPosition_style = 5;  //stand body position 
		 
	}
		
} 

void  capture_bodyPosition_Test(float* gx, float* gy, float* gz, int data_len, unsigned char* space)
{
	     
	     Rev_Data* revData_p = (Rev_Data*)space;
	     
	     multiply_moving_filter(gx, data_len, revData_p->gx_movingFilterData);
	     multiply_moving_filter(gy, data_len, revData_p->gy_movingFilterData);//movingfilter 
	     multiply_moving_filter(gz, data_len,revData_p->gz_movingFilterData);
	     
	     getFeatureValue(space); //get feature value
	     
	   	 precise_identify_bodyPosition_style(space); //precise identify body position style
		  
	   	 getDiffAmplitudeValue(space); //get amplitudeValue 
		
	     copyFeatureValue(space); //copy feature value
		  
	     amplitudeRankTest(space); // amplitude rank test
}
	
		



int get_precise_bodyPosition_style(unsigned char* space)
{
	Rev_Data* revData_p = (Rev_Data*)space; 
	
	return  revData_p->precise_bodyPosition_style; 
}


int get_AmplitudeValue_rank(unsigned char* space)
{
	Rev_Data* revData_p = (Rev_Data*)space; 
	
	return  revData_p->bodyPosition_amplitudeValue_rank; 
}
