/* Automatically generated source file */
#include <float.h>
#include <tinyengine_function.h>
#include <tinyengine_function_fp.h>

#include "genNN.h"
#include "genModel.h"
#include "genInclude.h"

/* Variables used by all ops */
ADD_params add_params;
int i;
int8_t *int8ptr,*int8ptr2;
int32_t *int32ptr;
float *fptr,*fptr2,*fptr3;

signed char* getInput() {
    return &buffer0[0];
}
signed char* getOutput() {
    return NNoutput;
}
const int anchors[3][3][2] = {
{{14,12},{16,16},{20,18},},
{{8,6},{10,10},{12,8},},
{{2,3},{4,5},{6,7},},
};
void det_post_procesing(int* box_cnt, det_box** ret_box, float threshold){
    int8_t *input_data[3]={&buffer0[1152],&buffer0[1024],&buffer0[512],};
    int8_t y_zero[3]={-45,-65,-15,};
    float y_scale[3]={0.06625068932771683,0.06012457236647606,0.08905195444822311,};
 	postprocessing(input_data, y_zero, y_scale, &buffer[1172], 32, 32, 18, 1, anchors, 3, 0.45f, threshold, box_cnt, ret_box);
}
void end2endinference(q7_t* img){
    invoke(NULL);
}
void invoke(float* labels){
/* layer 0:CONV_2D */
convolve_s8_kernel3_inputch3_stride2_pad1_fpreq(&buffer0[0],32,32,3,(const q7_t*) weight0,bias0,scales0,-128,14,-128,127,&buffer0[3072],16,16,4,sbuf,kbuf,-14);
/* layer 1:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[3072],16,16,4,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,scales1,-128,128,-128,127,&buffer0[3072],16,16,4,sbuf,-128);
/* layer 2:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[3072],16,16,4,(const q7_t*) weight2,bias2,scales2,-128,128,-128,127,&buffer0[0],16,16,8,sbuf);
/* layer 3:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[0],16,16,8,(const q7_t*) CHWweight3,offsetBias3,offsetRBias3,scales3,-128,128,-128,127,&buffer0[0],8,8,8,sbuf,-128);
/* layer 4:CONV_2D */
convolve_1x1_s8_ch8_fpreq(&buffer0[0],8,8,8,(const q7_t*) weight4,bias4,scales4,-128,128,-128,127,&buffer0[512],8,8,16,sbuf);
/* layer 5:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[512],8,8,16,(const q7_t*) weight5,bias5,scales5,-128,128,-128,127,&buffer0[1536],8,8,16,sbuf);
/* layer 6:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[1536],8,8,16,(const q7_t*) CHWweight6,offsetBias6,offsetRBias6,scales6,-128,128,-128,127,&buffer0[1536],4,4,16,sbuf,-128);
/* layer 7:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[1536],4,4,16,(const q7_t*) weight7,bias7,scales7,-128,128,-128,127,&buffer0[0],4,4,32,sbuf);
/* layer 8:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],4,4,32,(const q7_t*) weight8,bias8,scales8,-128,128,-128,127,&buffer0[512],4,4,32,sbuf);
/* layer 9:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[512],4,4,32,(const q7_t*) CHWweight9,offsetBias9,offsetRBias9,scales9,-128,128,-128,127,&buffer0[512],2,2,32,sbuf,-128);
/* layer 10:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],2,2,32,(const q7_t*) weight10,bias10,scales10,-128,128,-128,127,&buffer0[640],2,2,64,sbuf);
/* layer 11:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[640],2,2,64,(const q7_t*) weight11,bias11,scales11,-128,128,-128,127,&buffer0[896],2,2,64,sbuf);
/* layer 12:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[896],2,2,64,(const q7_t*) CHWweight12,offsetBias12,offsetRBias12,scales12,-128,128,-128,127,&buffer0[896],1,1,64,sbuf,-128);
/* layer 13:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[896],1,1,64,(const q7_t*) weight13,bias13,scales13,-128,128,-128,127,&buffer0[512],1,1,64,sbuf);
/* layer 14:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],1,1,64,(const q7_t*) weight14,bias14,scales14,-128,128,-128,127,&buffer0[576],1,1,64,sbuf);
/* layer 15:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[576],1,1,64,(const q7_t*) CHWweight15,offsetBias15,offsetRBias15,scales15,-128,128,-128,127,&buffer0[576],1,1,64,sbuf,-128);
/* layer 16:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[576],1,1,64,(const q7_t*) weight16,bias16,scales16,-128,128,-128,127,&buffer0[512],1,1,64,sbuf);
/* layer 17:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],1,1,64,(const q7_t*) weight17,bias17,scales17,-45,128,-128,127,&buffer0[1152],1,1,18,sbuf);
/* layer 18:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[640],2,2,64,(const q7_t*) weight18,bias18,scales18,-128,128,-128,127,&buffer0[896],2,2,64,sbuf);
/* layer 19:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[896],2,2,64,(const q7_t*) CHWweight19,offsetBias19,offsetRBias19,scales19,-128,128,-128,127,&buffer0[896],2,2,64,sbuf,-128);
/* layer 20:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[896],2,2,64,(const q7_t*) weight20,bias20,scales20,-128,128,-128,127,&buffer0[512],2,2,64,sbuf);
/* layer 21:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],2,2,64,(const q7_t*) weight21,bias21,scales21,-65,128,-128,127,&buffer0[1024],2,2,18,sbuf);
/* layer 22:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],4,4,32,(const q7_t*) weight22,bias22,scales22,-128,128,-128,127,&buffer0[512],4,4,32,sbuf);
/* layer 23:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[512],4,4,32,(const q7_t*) CHWweight23,offsetBias23,offsetRBias23,scales23,-128,128,-128,127,&buffer0[512],4,4,32,sbuf,-128);
/* layer 24:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],4,4,32,(const q7_t*) weight24,bias24,scales24,-128,128,-128,127,&buffer0[0],4,4,32,sbuf);
/* layer 25:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],4,4,32,(const q7_t*) weight25,bias25,scales25,-15,128,-128,127,&buffer0[512],4,4,18,sbuf);
}
void invoke_inf(){
/* layer 0:CONV_2D */
convolve_s8_kernel3_inputch3_stride2_pad1_fpreq(&buffer0[0],32,32,3,(const q7_t*) weight0,bias0,scales0,-128,14,-128,127,&buffer0[3072],16,16,4,sbuf,kbuf,-14);
/* layer 1:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[3072],16,16,4,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,scales1,-128,128,-128,127,&buffer0[3072],16,16,4,sbuf,-128);
/* layer 2:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[3072],16,16,4,(const q7_t*) weight2,bias2,scales2,-128,128,-128,127,&buffer0[0],16,16,8,sbuf);
/* layer 3:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[0],16,16,8,(const q7_t*) CHWweight3,offsetBias3,offsetRBias3,scales3,-128,128,-128,127,&buffer0[0],8,8,8,sbuf,-128);
/* layer 4:CONV_2D */
convolve_1x1_s8_ch8_fpreq(&buffer0[0],8,8,8,(const q7_t*) weight4,bias4,scales4,-128,128,-128,127,&buffer0[512],8,8,16,sbuf);
/* layer 5:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[512],8,8,16,(const q7_t*) weight5,bias5,scales5,-128,128,-128,127,&buffer0[1536],8,8,16,sbuf);
/* layer 6:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[1536],8,8,16,(const q7_t*) CHWweight6,offsetBias6,offsetRBias6,scales6,-128,128,-128,127,&buffer0[1536],4,4,16,sbuf,-128);
/* layer 7:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[1536],4,4,16,(const q7_t*) weight7,bias7,scales7,-128,128,-128,127,&buffer0[0],4,4,32,sbuf);
/* layer 8:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],4,4,32,(const q7_t*) weight8,bias8,scales8,-128,128,-128,127,&buffer0[512],4,4,32,sbuf);
/* layer 9:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[512],4,4,32,(const q7_t*) CHWweight9,offsetBias9,offsetRBias9,scales9,-128,128,-128,127,&buffer0[512],2,2,32,sbuf,-128);
/* layer 10:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],2,2,32,(const q7_t*) weight10,bias10,scales10,-128,128,-128,127,&buffer0[640],2,2,64,sbuf);
/* layer 11:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[640],2,2,64,(const q7_t*) weight11,bias11,scales11,-128,128,-128,127,&buffer0[896],2,2,64,sbuf);
/* layer 12:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[896],2,2,64,(const q7_t*) CHWweight12,offsetBias12,offsetRBias12,scales12,-128,128,-128,127,&buffer0[896],1,1,64,sbuf,-128);
/* layer 13:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[896],1,1,64,(const q7_t*) weight13,bias13,scales13,-128,128,-128,127,&buffer0[512],1,1,64,sbuf);
/* layer 14:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],1,1,64,(const q7_t*) weight14,bias14,scales14,-128,128,-128,127,&buffer0[576],1,1,64,sbuf);
/* layer 15:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[576],1,1,64,(const q7_t*) CHWweight15,offsetBias15,offsetRBias15,scales15,-128,128,-128,127,&buffer0[576],1,1,64,sbuf,-128);
/* layer 16:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[576],1,1,64,(const q7_t*) weight16,bias16,scales16,-128,128,-128,127,&buffer0[512],1,1,64,sbuf);
/* layer 17:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],1,1,64,(const q7_t*) weight17,bias17,scales17,-45,128,-128,127,&buffer0[1152],1,1,18,sbuf);
/* layer 18:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[640],2,2,64,(const q7_t*) weight18,bias18,scales18,-128,128,-128,127,&buffer0[896],2,2,64,sbuf);
/* layer 19:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[896],2,2,64,(const q7_t*) CHWweight19,offsetBias19,offsetRBias19,scales19,-128,128,-128,127,&buffer0[896],2,2,64,sbuf,-128);
/* layer 20:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[896],2,2,64,(const q7_t*) weight20,bias20,scales20,-128,128,-128,127,&buffer0[512],2,2,64,sbuf);
/* layer 21:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],2,2,64,(const q7_t*) weight21,bias21,scales21,-65,128,-128,127,&buffer0[1024],2,2,18,sbuf);
/* layer 22:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],4,4,32,(const q7_t*) weight22,bias22,scales22,-128,128,-128,127,&buffer0[512],4,4,32,sbuf);
/* layer 23:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[512],4,4,32,(const q7_t*) CHWweight23,offsetBias23,offsetRBias23,scales23,-128,128,-128,127,&buffer0[512],4,4,32,sbuf,-128);
/* layer 24:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[512],4,4,32,(const q7_t*) weight24,bias24,scales24,-128,128,-128,127,&buffer0[0],4,4,32,sbuf);
/* layer 25:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],4,4,32,(const q7_t*) weight25,bias25,scales25,-15,128,-128,127,&buffer0[512],4,4,18,sbuf);
}
