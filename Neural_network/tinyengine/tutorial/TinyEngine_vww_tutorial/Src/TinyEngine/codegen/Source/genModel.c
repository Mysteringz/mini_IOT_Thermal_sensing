/* Automatically generated source file */
#include <float.h>
#include <string.h>
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

void invoke_1patch(uint16_t,  uint16_t,  uint16_t,  uint16_t);

signed char* getInput() {
    return &buffer0[8464];
}
signed char* getOutput() {
    return NNoutput;
}
void end2endinference(q7_t* img){
    //stage 1
    int i, j, h, w, c;
    for (i = 0; i < 2; i++){
        uint16_t pad_t=0,pad_b=0;
        if (i == 0){
            pad_t = 5;
        }
        else if (i == 1){
            pad_b = 2;
        }
        for (j = 0; j < 2; j++){
            uint16_t pad_l=0,pad_r=0;
            if (j == 0){
                pad_l = 5;
            }
            else if (j == 1){
                pad_r = 2;
            }
            /* load partial input from the img */
            q7_t* patch_input = getInput(); // for partial input
            int start_x = TN_MAX(40 * j - 5,0);
            int start_y = TN_MAX(40 * i - 5,0);
            q7_t* img_ptr = &img[(start_x + start_y * 80) * 3];

            //skip top
            patch_input += pad_t * 47 * 3;
            for (h = pad_t; h < 47 - pad_b; h++){
                //skip left
                patch_input += pad_l * 3;
                //fill middle
                int bytes = (47 - (pad_l + pad_r)) * 3;
                memcpy (patch_input, img_ptr, bytes);
                img_ptr += 80 * 3;
                patch_input += bytes;
                //skip right
                patch_input += pad_r * 3;
            }
            invoke_1patch(pad_t,pad_b,pad_l,pad_r);
            /* concat the output from buffer0 (this is set manually for now) */
            q7_t* output_ptr = &buffer0[21168];;
            q7_t* patch_output = &buffer0[0];
            for (h = 0; h < 10; h++){
                for (w = 0; w < 10; w++){
                    for (c = 0; c < 48; c++){
                        int output_idx = ((w + j * 10) + (h + i * 10) * 20) * 48 + c;;
                        output_ptr[output_idx] = patch_output[(w + h * 10) * 48 + c];
                    }
                }
            }
        }
    }
    //stage 2
    invoke(NULL);
}

void invoke_1patch(uint16_t pad_t, uint16_t pad_b, uint16_t pad_l ,uint16_t pad_r){
/* layer 0:CONV_2D */
patchpadding_convolve_s8_kernel3_inputch3_stride2(&buffer0[8464],47,47,3,(const q7_t*) weight0,bias0,shift0,multiplier0,-128,1,-128,127,&buffer0[0],23,23,16,sbuf,kbuf,-1, pad_t, pad_b, pad_l, pad_r);
pad_t /= 2;pad_b /= 2;pad_l /= 2;pad_r /= 2;
/* layer 1:DEPTHWISE_CONV_2D */
patchpadding_depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],23,23,16,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,shift1,multiplier1,-128,128,-128,127,&buffer0[0],21,21,16,sbuf,-128, pad_t, pad_b, pad_l, pad_r);
pad_t = TN_MAX(0, pad_t-1);pad_b = TN_MAX(0, pad_b-1);pad_l = TN_MAX(0, pad_l-1);pad_r = TN_MAX(0, pad_r-1);
/* layer 2:CONV_2D */
convolve_1x1_s8_ch16(&buffer0[0],21,21,16,(const q7_t*) weight2,bias2,shift2,multiplier2,-4,128,-128,127,&buffer0[40368],21,21,8,sbuf);
/* layer 3:CONV_2D */
convolve_1x1_s8_ch8(&buffer0[40368],21,21,8,(const q7_t*) weight3,bias3,shift3,multiplier3,-128,4,-128,127,&buffer0[0],21,21,48,sbuf);
/* layer 4:DEPTHWISE_CONV_2D */
patchpadding_depthwise_kernel3x3_stride2_inplace_CHW(&buffer0[0],21,21,48,(const q7_t*) CHWweight4,offsetBias4,offsetRBias4,shift4,multiplier4,-128,128,-128,127,&buffer0[0],10,10,48,sbuf,-128, pad_t, pad_b, pad_l, pad_r);
pad_t /= 2;pad_b /= 2;pad_l /= 2;pad_r /= 2;
}

void invoke(float* labels){
/* layer 0:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[21168],20,20,48,(const q7_t*) weight5,bias5,shift5,multiplier5,-22,128,-128,127,&buffer0[40368],20,20,16,sbuf);
/* layer 1:CONV_2D */
convolve_1x1_s8_ch16(&buffer0[40368],20,20,16,(const q7_t*) weight6,bias6,shift6,multiplier6,-128,22,-128,127,&buffer0[0],20,20,48,sbuf);
/* layer 2:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],20,20,48,(const q7_t*) CHWweight7,offsetBias7,offsetRBias7,shift7,multiplier7,-128,128,-128,127,&buffer0[0],20,20,48,sbuf,-128);
/* layer 3:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[0],20,20,48,(const q7_t*) weight8,bias8,shift8,multiplier8,8,128,-128,127,&buffer0[46768],20,20,16,sbuf);
/* layer 4:ADD */
add_fpreq(6400, &buffer0[46768],0.07699620723724365,8,&buffer0[40368],0.08598089963197708,-22,0.10197763890028,-20,&buffer0[53168]);
/* layer 5:CONV_2D */
convolve_1x1_s8_ch16(&buffer0[53168],20,20,16,(const q7_t*) weight9,bias9,shift9,multiplier9,-128,20,-128,127,&buffer0[0],20,20,48,sbuf);
/* layer 6:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],20,20,48,(const q7_t*) CHWweight10,offsetBias10,offsetRBias10,shift10,multiplier10,-128,128,-128,127,&buffer0[0],20,20,48,sbuf,-128);
/* layer 7:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[0],20,20,48,(const q7_t*) weight11,bias11,shift11,multiplier11,-4,128,-128,127,&buffer0[40368],20,20,16,sbuf);
/* layer 8:ADD */
add_fpreq(6400, &buffer0[40368],0.024848325178027153,-4,&buffer0[53168],0.10197763890028,-20,0.10464993864297867,-15,&buffer0[46768]);
/* layer 9:CONV_2D */
convolve_1x1_s8_ch16(&buffer0[46768],20,20,16,(const q7_t*) weight12,bias12,shift12,multiplier12,-128,15,-128,127,&buffer0[0],20,20,48,sbuf);
/* layer 10:DEPTHWISE_CONV_2D */
depthwise_kernel7x7_stride2_inplace_CHW(&buffer0[0],20,20,48,(const q7_t*) CHWweight13,offsetBias13,offsetRBias13,shift13,multiplier13,-128,128,-128,127,&buffer0[0],10,10,48,sbuf,-128);
/* layer 11:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[0],10,10,48,(const q7_t*) weight14,bias14,shift14,multiplier14,-16,128,-128,127,&buffer0[14400],10,10,24,sbuf);
/* layer 12:CONV_2D */
convolve_1x1_s8_ch24(&buffer0[14400],10,10,24,(const q7_t*) weight15,bias15,shift15,multiplier15,-128,16,-128,127,&buffer0[0],10,10,144,sbuf);
/* layer 13:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],10,10,144,(const q7_t*) CHWweight16,offsetBias16,offsetRBias16,shift16,multiplier16,-128,128,-128,127,&buffer0[0],10,10,144,sbuf,-128);
/* layer 14:CONV_2D */
convolve_1x1_s8(&buffer0[0],10,10,144,(const q7_t*) weight17,bias17,shift17,multiplier17,-14,128,-128,127,&buffer0[16800],10,10,24,sbuf);
/* layer 15:ADD */
add_fpreq(2400, &buffer0[16800],0.06462342292070389,-14,&buffer0[14400],0.06311018764972687,-16,0.07023955136537552,-3,&buffer0[12000]);
/* layer 16:CONV_2D */
convolve_1x1_s8_ch24(&buffer0[12000],10,10,24,(const q7_t*) weight18,bias18,shift18,multiplier18,-128,3,-128,127,&buffer0[0],10,10,120,sbuf);
/* layer 17:DEPTHWISE_CONV_2D */
depthwise_kernel5x5_stride1_inplace_CHW(&buffer0[0],10,10,120,(const q7_t*) CHWweight19,offsetBias19,offsetRBias19,shift19,multiplier19,-128,128,-128,127,&buffer0[0],10,10,120,sbuf,-128);
/* layer 18:CONV_2D */
convolve_1x1_s8(&buffer0[0],10,10,120,(const q7_t*) weight20,bias20,shift20,multiplier20,-3,128,-128,127,&buffer0[14400],10,10,24,sbuf);
/* layer 19:ADD */
add_fpreq(2400, &buffer0[14400],0.018893597647547722,-3,&buffer0[12000],0.07023955136537552,-3,0.07366174459457397,-1,&buffer0[16800]);
/* layer 20:CONV_2D */
convolve_1x1_s8_ch24(&buffer0[16800],10,10,24,(const q7_t*) weight21,bias21,shift21,multiplier21,-128,1,-128,127,&buffer0[0],10,10,144,sbuf);
/* layer 21:DEPTHWISE_CONV_2D */
depthwise_kernel7x7_stride2_inplace_CHW(&buffer0[0],10,10,144,(const q7_t*) CHWweight22,offsetBias22,offsetRBias22,shift22,multiplier22,-128,128,-128,127,&buffer0[0],5,5,144,sbuf,-128);
/* layer 22:CONV_2D */
convolve_1x1_s8(&buffer0[0],5,5,144,(const q7_t*) weight23,bias23,shift23,multiplier23,-11,128,-128,127,&buffer0[6000],5,5,40,sbuf);
/* layer 23:CONV_2D */
convolve_1x1_s8(&buffer0[6000],5,5,40,(const q7_t*) weight24,bias24,shift24,multiplier24,-128,11,-128,127,&buffer0[0],5,5,240,sbuf);
/* layer 24:DEPTHWISE_CONV_2D */
depthwise_kernel7x7_stride1_inplace_CHW(&buffer0[0],5,5,240,(const q7_t*) CHWweight25,offsetBias25,offsetRBias25,shift25,multiplier25,-128,128,-128,127,&buffer0[0],5,5,240,sbuf,-128);
/* layer 25:CONV_2D */
convolve_1x1_s8(&buffer0[0],5,5,240,(const q7_t*) weight26,bias26,shift26,multiplier26,1,128,-128,127,&buffer0[7000],5,5,40,sbuf);
/* layer 26:ADD */
add_fpreq(1000, &buffer0[7000],0.04340619966387749,1,&buffer0[6000],0.0489354208111763,-11,0.04974109306931496,-3,&buffer0[8000]);
/* layer 27:CONV_2D */
convolve_1x1_s8(&buffer0[8000],5,5,40,(const q7_t*) weight27,bias27,shift27,multiplier27,-128,3,-128,127,&buffer0[0],5,5,240,sbuf);
/* layer 28:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],5,5,240,(const q7_t*) CHWweight28,offsetBias28,offsetRBias28,shift28,multiplier28,-128,128,-128,127,&buffer0[0],5,5,240,sbuf,-128);
/* layer 29:CONV_2D */
convolve_1x1_s8(&buffer0[0],5,5,240,(const q7_t*) weight29,bias29,shift29,multiplier29,15,128,-128,127,&buffer0[6000],5,5,48,sbuf);
/* layer 30:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[6000],5,5,48,(const q7_t*) weight30,bias30,shift30,multiplier30,-128,-15,-128,127,&buffer0[0],5,5,192,sbuf);
/* layer 31:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],5,5,192,(const q7_t*) CHWweight31,offsetBias31,offsetRBias31,shift31,multiplier31,-128,128,-128,127,&buffer0[0],5,5,192,sbuf,-128);
/* layer 32:CONV_2D */
convolve_1x1_s8(&buffer0[0],5,5,192,(const q7_t*) weight32,bias32,shift32,multiplier32,-11,128,-128,127,&buffer0[4800],5,5,48,sbuf);
/* layer 33:ADD */
add_fpreq(1200, &buffer0[4800],0.04923376813530922,-11,&buffer0[6000],0.04425579309463501,15,0.05336926504969597,5,&buffer0[7200]);
/* layer 34:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[7200],5,5,48,(const q7_t*) weight33,bias33,shift33,multiplier33,-128,-5,-128,127,&buffer0[0],5,5,240,sbuf);
/* layer 35:DEPTHWISE_CONV_2D */
depthwise_kernel5x5_stride2_inplace_CHW(&buffer0[0],5,5,240,(const q7_t*) CHWweight34,offsetBias34,offsetRBias34,shift34,multiplier34,-128,128,-128,127,&buffer0[0],3,3,240,sbuf,-128);
/* layer 36:CONV_2D */
convolve_1x1_s8(&buffer0[0],3,3,240,(const q7_t*) weight35,bias35,shift35,multiplier35,-7,128,-128,127,&buffer0[4320],3,3,96,sbuf);
/* layer 37:CONV_2D */
convolve_1x1_s8(&buffer0[4320],3,3,96,(const q7_t*) weight36,bias36,shift36,multiplier36,-128,7,-128,127,&buffer0[0],3,3,480,sbuf);
/* layer 38:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],3,3,480,(const q7_t*) CHWweight37,offsetBias37,offsetRBias37,shift37,multiplier37,-128,128,-128,127,&buffer0[0],3,3,480,sbuf,-128);
/* layer 39:CONV_2D */
convolve_1x1_s8(&buffer0[0],3,3,480,(const q7_t*) weight38,bias38,shift38,multiplier38,-2,128,-128,127,&buffer0[5184],3,3,96,sbuf);
/* layer 40:ADD */
add_fpreq(864, &buffer0[5184],0.04011229798197746,-2,&buffer0[4320],0.042191825807094574,-7,0.03923744335770607,-9,&buffer0[3456]);
/* layer 41:CONV_2D */
convolve_1x1_s8(&buffer0[3456],3,3,96,(const q7_t*) weight39,bias39,shift39,multiplier39,-128,9,-128,127,&buffer0[0],3,3,384,sbuf);
/* layer 42:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],3,3,384,(const q7_t*) CHWweight40,offsetBias40,offsetRBias40,shift40,multiplier40,-128,128,-128,127,&buffer0[0],3,3,384,sbuf,-128);
/* layer 43:CONV_2D */
convolve_1x1_s8(&buffer0[0],3,3,384,(const q7_t*) weight41,bias41,shift41,multiplier41,-6,128,-128,127,&buffer0[4320],3,3,96,sbuf);
/* layer 44:ADD */
add_fpreq(864, &buffer0[4320],0.019318707287311554,-6,&buffer0[3456],0.03923744335770607,-9,0.0419764444231987,-12,&buffer0[2592]);
/* layer 45:CONV_2D */
convolve_1x1_s8(&buffer0[2592],3,3,96,(const q7_t*) weight42,bias42,shift42,multiplier42,-128,12,-128,127,&buffer0[0],3,3,288,sbuf);
/* layer 46:DEPTHWISE_CONV_2D */
depthwise_kernel7x7_stride1_inplace_CHW(&buffer0[0],3,3,288,(const q7_t*) CHWweight43,offsetBias43,offsetRBias43,shift43,multiplier43,-128,128,-128,127,&buffer0[0],3,3,288,sbuf,-128);
/* layer 47:CONV_2D */
convolve_1x1_s8(&buffer0[0],3,3,288,(const q7_t*) weight44,bias44,shift44,multiplier44,9,128,-128,127,&buffer0[2592],3,3,160,sbuf);
/* layer 48:AVERAGE_POOL_2D */
avg_pooling(&buffer0[2592],3,3,160,3,3,1,1,-128,127,&buffer0[0]);
/* layer 49:CONV_2D */
convolve_1x1_s8(&buffer0[0],1,1,160,(const q7_t*) weight45,bias45,shift45,multiplier45,-1,-9,-128,127,&buffer0[160],1,1,2,sbuf);
}
void invoke_inf(){
/* layer 0:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[21168],20,20,48,(const q7_t*) weight5,bias5,shift5,multiplier5,-22,128,-128,127,&buffer0[40368],20,20,16,sbuf);
/* layer 1:CONV_2D */
convolve_1x1_s8_ch16(&buffer0[40368],20,20,16,(const q7_t*) weight6,bias6,shift6,multiplier6,-128,22,-128,127,&buffer0[0],20,20,48,sbuf);
/* layer 2:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],20,20,48,(const q7_t*) CHWweight7,offsetBias7,offsetRBias7,shift7,multiplier7,-128,128,-128,127,&buffer0[0],20,20,48,sbuf,-128);
/* layer 3:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[0],20,20,48,(const q7_t*) weight8,bias8,shift8,multiplier8,8,128,-128,127,&buffer0[46768],20,20,16,sbuf);
/* layer 4:ADD */
add_fpreq(6400, &buffer0[46768],0.07699620723724365,8,&buffer0[40368],0.08598089963197708,-22,0.10197763890028,-20,&buffer0[53168]);
/* layer 5:CONV_2D */
convolve_1x1_s8_ch16(&buffer0[53168],20,20,16,(const q7_t*) weight9,bias9,shift9,multiplier9,-128,20,-128,127,&buffer0[0],20,20,48,sbuf);
/* layer 6:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],20,20,48,(const q7_t*) CHWweight10,offsetBias10,offsetRBias10,shift10,multiplier10,-128,128,-128,127,&buffer0[0],20,20,48,sbuf,-128);
/* layer 7:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[0],20,20,48,(const q7_t*) weight11,bias11,shift11,multiplier11,-4,128,-128,127,&buffer0[40368],20,20,16,sbuf);
/* layer 8:ADD */
add_fpreq(6400, &buffer0[40368],0.024848325178027153,-4,&buffer0[53168],0.10197763890028,-20,0.10464993864297867,-15,&buffer0[46768]);
/* layer 9:CONV_2D */
convolve_1x1_s8_ch16(&buffer0[46768],20,20,16,(const q7_t*) weight12,bias12,shift12,multiplier12,-128,15,-128,127,&buffer0[0],20,20,48,sbuf);
/* layer 10:DEPTHWISE_CONV_2D */
depthwise_kernel7x7_stride2_inplace_CHW(&buffer0[0],20,20,48,(const q7_t*) CHWweight13,offsetBias13,offsetRBias13,shift13,multiplier13,-128,128,-128,127,&buffer0[0],10,10,48,sbuf,-128);
/* layer 11:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[0],10,10,48,(const q7_t*) weight14,bias14,shift14,multiplier14,-16,128,-128,127,&buffer0[14400],10,10,24,sbuf);
/* layer 12:CONV_2D */
convolve_1x1_s8_ch24(&buffer0[14400],10,10,24,(const q7_t*) weight15,bias15,shift15,multiplier15,-128,16,-128,127,&buffer0[0],10,10,144,sbuf);
/* layer 13:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],10,10,144,(const q7_t*) CHWweight16,offsetBias16,offsetRBias16,shift16,multiplier16,-128,128,-128,127,&buffer0[0],10,10,144,sbuf,-128);
/* layer 14:CONV_2D */
convolve_1x1_s8(&buffer0[0],10,10,144,(const q7_t*) weight17,bias17,shift17,multiplier17,-14,128,-128,127,&buffer0[16800],10,10,24,sbuf);
/* layer 15:ADD */
add_fpreq(2400, &buffer0[16800],0.06462342292070389,-14,&buffer0[14400],0.06311018764972687,-16,0.07023955136537552,-3,&buffer0[12000]);
/* layer 16:CONV_2D */
convolve_1x1_s8_ch24(&buffer0[12000],10,10,24,(const q7_t*) weight18,bias18,shift18,multiplier18,-128,3,-128,127,&buffer0[0],10,10,120,sbuf);
/* layer 17:DEPTHWISE_CONV_2D */
depthwise_kernel5x5_stride1_inplace_CHW(&buffer0[0],10,10,120,(const q7_t*) CHWweight19,offsetBias19,offsetRBias19,shift19,multiplier19,-128,128,-128,127,&buffer0[0],10,10,120,sbuf,-128);
/* layer 18:CONV_2D */
convolve_1x1_s8(&buffer0[0],10,10,120,(const q7_t*) weight20,bias20,shift20,multiplier20,-3,128,-128,127,&buffer0[14400],10,10,24,sbuf);
/* layer 19:ADD */
add_fpreq(2400, &buffer0[14400],0.018893597647547722,-3,&buffer0[12000],0.07023955136537552,-3,0.07366174459457397,-1,&buffer0[16800]);
/* layer 20:CONV_2D */
convolve_1x1_s8_ch24(&buffer0[16800],10,10,24,(const q7_t*) weight21,bias21,shift21,multiplier21,-128,1,-128,127,&buffer0[0],10,10,144,sbuf);
/* layer 21:DEPTHWISE_CONV_2D */
depthwise_kernel7x7_stride2_inplace_CHW(&buffer0[0],10,10,144,(const q7_t*) CHWweight22,offsetBias22,offsetRBias22,shift22,multiplier22,-128,128,-128,127,&buffer0[0],5,5,144,sbuf,-128);
/* layer 22:CONV_2D */
convolve_1x1_s8(&buffer0[0],5,5,144,(const q7_t*) weight23,bias23,shift23,multiplier23,-11,128,-128,127,&buffer0[6000],5,5,40,sbuf);
/* layer 23:CONV_2D */
convolve_1x1_s8(&buffer0[6000],5,5,40,(const q7_t*) weight24,bias24,shift24,multiplier24,-128,11,-128,127,&buffer0[0],5,5,240,sbuf);
/* layer 24:DEPTHWISE_CONV_2D */
depthwise_kernel7x7_stride1_inplace_CHW(&buffer0[0],5,5,240,(const q7_t*) CHWweight25,offsetBias25,offsetRBias25,shift25,multiplier25,-128,128,-128,127,&buffer0[0],5,5,240,sbuf,-128);
/* layer 25:CONV_2D */
convolve_1x1_s8(&buffer0[0],5,5,240,(const q7_t*) weight26,bias26,shift26,multiplier26,1,128,-128,127,&buffer0[7000],5,5,40,sbuf);
/* layer 26:ADD */
add_fpreq(1000, &buffer0[7000],0.04340619966387749,1,&buffer0[6000],0.0489354208111763,-11,0.04974109306931496,-3,&buffer0[8000]);
/* layer 27:CONV_2D */
convolve_1x1_s8(&buffer0[8000],5,5,40,(const q7_t*) weight27,bias27,shift27,multiplier27,-128,3,-128,127,&buffer0[0],5,5,240,sbuf);
/* layer 28:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],5,5,240,(const q7_t*) CHWweight28,offsetBias28,offsetRBias28,shift28,multiplier28,-128,128,-128,127,&buffer0[0],5,5,240,sbuf,-128);
/* layer 29:CONV_2D */
convolve_1x1_s8(&buffer0[0],5,5,240,(const q7_t*) weight29,bias29,shift29,multiplier29,15,128,-128,127,&buffer0[6000],5,5,48,sbuf);
/* layer 30:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[6000],5,5,48,(const q7_t*) weight30,bias30,shift30,multiplier30,-128,-15,-128,127,&buffer0[0],5,5,192,sbuf);
/* layer 31:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],5,5,192,(const q7_t*) CHWweight31,offsetBias31,offsetRBias31,shift31,multiplier31,-128,128,-128,127,&buffer0[0],5,5,192,sbuf,-128);
/* layer 32:CONV_2D */
convolve_1x1_s8(&buffer0[0],5,5,192,(const q7_t*) weight32,bias32,shift32,multiplier32,-11,128,-128,127,&buffer0[4800],5,5,48,sbuf);
/* layer 33:ADD */
add_fpreq(1200, &buffer0[4800],0.04923376813530922,-11,&buffer0[6000],0.04425579309463501,15,0.05336926504969597,5,&buffer0[7200]);
/* layer 34:CONV_2D */
convolve_1x1_s8_ch48(&buffer0[7200],5,5,48,(const q7_t*) weight33,bias33,shift33,multiplier33,-128,-5,-128,127,&buffer0[0],5,5,240,sbuf);
/* layer 35:DEPTHWISE_CONV_2D */
depthwise_kernel5x5_stride2_inplace_CHW(&buffer0[0],5,5,240,(const q7_t*) CHWweight34,offsetBias34,offsetRBias34,shift34,multiplier34,-128,128,-128,127,&buffer0[0],3,3,240,sbuf,-128);
/* layer 36:CONV_2D */
convolve_1x1_s8(&buffer0[0],3,3,240,(const q7_t*) weight35,bias35,shift35,multiplier35,-7,128,-128,127,&buffer0[4320],3,3,96,sbuf);
/* layer 37:CONV_2D */
convolve_1x1_s8(&buffer0[4320],3,3,96,(const q7_t*) weight36,bias36,shift36,multiplier36,-128,7,-128,127,&buffer0[0],3,3,480,sbuf);
/* layer 38:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],3,3,480,(const q7_t*) CHWweight37,offsetBias37,offsetRBias37,shift37,multiplier37,-128,128,-128,127,&buffer0[0],3,3,480,sbuf,-128);
/* layer 39:CONV_2D */
convolve_1x1_s8(&buffer0[0],3,3,480,(const q7_t*) weight38,bias38,shift38,multiplier38,-2,128,-128,127,&buffer0[5184],3,3,96,sbuf);
/* layer 40:ADD */
add_fpreq(864, &buffer0[5184],0.04011229798197746,-2,&buffer0[4320],0.042191825807094574,-7,0.03923744335770607,-9,&buffer0[3456]);
/* layer 41:CONV_2D */
convolve_1x1_s8(&buffer0[3456],3,3,96,(const q7_t*) weight39,bias39,shift39,multiplier39,-128,9,-128,127,&buffer0[0],3,3,384,sbuf);
/* layer 42:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW(&buffer0[0],3,3,384,(const q7_t*) CHWweight40,offsetBias40,offsetRBias40,shift40,multiplier40,-128,128,-128,127,&buffer0[0],3,3,384,sbuf,-128);
/* layer 43:CONV_2D */
convolve_1x1_s8(&buffer0[0],3,3,384,(const q7_t*) weight41,bias41,shift41,multiplier41,-6,128,-128,127,&buffer0[4320],3,3,96,sbuf);
/* layer 44:ADD */
add_fpreq(864, &buffer0[4320],0.019318707287311554,-6,&buffer0[3456],0.03923744335770607,-9,0.0419764444231987,-12,&buffer0[2592]);
/* layer 45:CONV_2D */
convolve_1x1_s8(&buffer0[2592],3,3,96,(const q7_t*) weight42,bias42,shift42,multiplier42,-128,12,-128,127,&buffer0[0],3,3,288,sbuf);
/* layer 46:DEPTHWISE_CONV_2D */
depthwise_kernel7x7_stride1_inplace_CHW(&buffer0[0],3,3,288,(const q7_t*) CHWweight43,offsetBias43,offsetRBias43,shift43,multiplier43,-128,128,-128,127,&buffer0[0],3,3,288,sbuf,-128);
/* layer 47:CONV_2D */
convolve_1x1_s8(&buffer0[0],3,3,288,(const q7_t*) weight44,bias44,shift44,multiplier44,9,128,-128,127,&buffer0[2592],3,3,160,sbuf);
/* layer 48:AVERAGE_POOL_2D */
avg_pooling(&buffer0[2592],3,3,160,3,3,1,1,-128,127,&buffer0[0]);
/* layer 49:CONV_2D */
convolve_1x1_s8(&buffer0[0],1,1,160,(const q7_t*) weight45,bias45,shift45,multiplier45,-1,-9,-128,127,&buffer0[160],1,1,2,sbuf);
}
