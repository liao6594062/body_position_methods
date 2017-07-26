#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"body_position_algo.h"
#include "readcsv.h"
/*run this program using the console pauser or add your own getch, system("pause") or input loop*/


int main()
{
	char *fname = "C:\\Users\\EEG\\Desktop\\ios_trackgyro_liyugao_28_20170710_232557.csv"; //the address of test_File 
	int row, col; // the row and col of data
	float *raw; // save data as vector  
	int num_index = 0; // the order of data elements 
    unsigned char *space = bp_create_and_initialize_space(); // create and initialize space
    float *gx = (float*)malloc(sizeof(float) * 50); // save x-axis gravity
    float *gy = (float*)malloc(sizeof(float) * 50); // save y-axis gravity
    float *gz = (float*)malloc(sizeof(float) * 50); // save z-axis gravity
    int a, b; // body Position style and body amplitude rank 
    int index; 
	//fp1 = fopen("precise_location_style_vector.txt","w");   // ��ÿ�β��񵽵ľ�ȷ��λ����д�� precise_location_style_vector.txt�ļ��� 
    //fp2 = fopen("hospital_location_style_vector.txt","w");  // ��ÿ�β��񵽵�ҽ����λ����д�� hospital_location_style_vector.txt�ļ���
    //fp3 = fopen("precise_location_feature_data.txt","w");   // ��ÿ�β��񵽵ľ�ȷ��λ��������д�� precise_location_feature_data.txt�ļ���
    //fp4 = fopen("hospital_location_feature_data.txt","w");  // ��ÿ�β��񵽵�ҽ����λ��������д�� precise_location_feature_data.txt�ļ���
    //fp5 = fopen("diff_location_amplitudeValue.txt","w");    // ��ÿ�β��񵽵���λ���ȱ仯д�� diff_location_amplitudeValue.txt�ļ���
    //fp6 = fopen("former_location_feature_data.txt","w");    // ����һ�����λʱ������д�� former_location_feature_data.txt�ļ���*/
    //fp7 = fopen("location_amplitudeValue_rank.txt","w");    // ����λ�ȼ�д�� location_amplitudeValue_rank.txt�ļ���
    
	col = get_col_of_raw(fname); //get the col of raw 
	row = get_row_of_raw(fname, col); // get the row of raw 
	raw = (float*)malloc(sizeof(float) * row * col);  // get the data named raw 
	get_raw_from_csv(fname, raw, row, col); // get raw from csv 
	printf("size of matrix: %d x %d\n", row, col); // print row and col 
	
	for(index = 0; index < 100; index++)
	{
	  for(num_index = 25*index; num_index < 25*index+50; num_index++)  
	 {
		gx[num_index-25*index] = raw[9*num_index+3]; 
		gy[num_index-25*index] = raw[9*num_index+4];
		gz[num_index-25*index] = raw[9*num_index+5];
	 }
     capture_body_position_test(gx, gy, gz, 50, space); //bodyPosition test
     a = get_precise_body_position_style(space); // get precise bodyPosition style
     b = get_amplitude_rank(space); //get amplitudeValue rank
     printf("\n a= %d,b=%d", a, b);
    }
	
   free_point(space); // free point
   return 0;		    
}
