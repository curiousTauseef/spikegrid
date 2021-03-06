/// \file
//for debugging
#define _GNU_SOURCE
#include <string.h> //memcpy
#ifndef _WIN32
#include <getopt.h> //getopt
#endif
#include <stdlib.h> //exit
#include <fenv.h>   //for some debugging
#include <stdio.h>
#include "cleanup.h"
#include "evolve.h"
#include "newparam.h"
#include "init.h"
#include "yossarian.h"
#include "out/out.h"
#include "timing.h"
#include "init/model.h"
#ifdef ANDROID
    #define APPNAME "myapp"
    #include <android/log.h>
#endif
//This needs to be a macro so that we can stringify the arguments - the #Switch
#define CheckParam(Switch,var) if (Switch==ON && var==NULL) {printf ("%s is on, so I need %s as an input\n",#Switch,#var);return;}
model* m;               ///< The model we are evolving through time - static data not great but it does only exist in this file
int jobnumber=-1;        ///< The current job number - used for pics directory etc
int yossarianjobnumber=-1;

void copyStuff(
        void* d1,const void* inp,const size_t bytes,
        const on_off recovery, void* recoverydest,const void* recoverysource,const size_t recoverysize)

{
    memcpy(d1,inp,bytes);
    if (recovery==ON) {memcpy(recoverydest,recoverysource,recoverysize);}
}
//DO NOT CALL THIS FUNCTION "step" - this causes a weird collision in matlab that results in segfaults.  Incredibly fun to debug
///Function that steps the model through time (high level).
/// I think maybe we should get rid of this function
/// @param inpV the input voltages
/// @param inpV2 input voltages for layer 2.  In the single layer model a dummy argument needs to be passed.
/// @param inpW the input recoveries
/// @param inpW2 input recoveries for layer 2.  In the single layer model a dummy argument needs to be passed.
void step_(const Compute_float* const inpV,const Compute_float* const inpV2, const Compute_float* inpW, const Compute_float* inpW2)
{
    CheckParam(ON,inpV)
    CheckParam(Features.Recovery,inpW) //note checkparam is a macro here
    if (ModelType==DUALLAYER)
    {
        CheckParam(ON,inpV2)
        CheckParam(Features.Recovery,inpW2)
    }
    m->timesteps++;
    copyStuff(&m->layer1.voltages,inpV,sizeof(Compute_float)*grid_size*grid_size,
            Features.Recovery,&m->layer1.recoverys,inpW,sizeof(Compute_float)*grid_size*grid_size);
    if (ModelType==DUALLAYER)
    {
        copyStuff(&m->layer2.voltages,inpV2,sizeof(Compute_float)*grid_size*grid_size,
            Features.Recovery,&m->layer2.recoverys,inpW2,sizeof(Compute_float)*grid_size*grid_size);
    }
    step1(m);
    DoOutputs(m->timesteps);
}
//this could move files - not quite sure where though
void InitVoltage(Compute_float** Volts,const Compute_float Vrt,const Compute_float Vpk,const Job* const job)
{
    if (job -> initcond == SINGLE_SPIKE) {Fixedinit(*Volts,Vrt,job->Voltage_or_count);}
    else                                 {randinit (*Volts,Vrt,Vpk);}
}

//I am not a huge fan of this function.  A nicer version would be good.
void setuppointers(Compute_float** FirstV,Compute_float** SecondV, Compute_float** FirstW, Compute_float** SecondW,const Job* const job,const parameters* const OneLayer, const parameters* const DualLayerIn, const parameters* const DualLayerEx)
{
    *FirstV = calloc(sizeof(Compute_float),grid_size*grid_size); //we always need the voltage in the first layer
    if (ModelType==SINGLELAYER)
    {
        *SecondV = NULL; *SecondW = NULL; //force all dual layer to be null
        InitVoltage(FirstV,OneLayer->potential.Vrt,OneLayer->potential.Vpk,job);
        if (Features.Recovery==ON)
        {
            *FirstW = calloc(sizeof(Compute_float),grid_size*grid_size);
        } else {*FirstW=NULL;}
    }
    else if (ModelType==DUALLAYER)
    {
        *SecondV = malloc(sizeof(Compute_float)*grid_size*grid_size);
        InitVoltage(FirstV, DualLayerIn->potential.Vrt,DualLayerIn->potential.Vpk,job);
        InitVoltage(SecondV,DualLayerEx->potential.Vrt,DualLayerEx->potential.Vpk,job);
        if (Features.Recovery==ON)
        {
            *FirstW  = calloc(sizeof(Compute_float),grid_size*grid_size);
            *SecondW = calloc(sizeof(Compute_float),grid_size*grid_size);
        } else {*FirstW=NULL;*SecondW=NULL;}
    }
}

#ifdef MATLAB
#include <time.h>
#include "out/outputtable.h"
//The easeiest way to get data out with matlab is to use outputtomxarray and the easiest way to use that is with getoutputbyname.  Getoutputbyname is in output.h so include it.
//#include "matlab_output.h"
int setup_done=0;
mxArray* CreateInitialValues(const Compute_float minval, const Compute_float maxval)
{
    mxArray* vals =mxCreateNumericMatrix(grid_size,grid_size,MatlabDataType(),mxREAL);
    Compute_float* datavals = (Compute_float*)mxGetData(vals);
    randinit(datavals,minval,maxval);
    return vals;
}
typedef struct mexmap
{
    const char* const name;
    const char* outname;
    Compute_float** data;
    const LayerNumbers Lno;
    const on_off recovery;
    Compute_float init_min;
    Compute_float init_max;
} mexmap;

///Matlab entry point. The pointers give access to the left and right hand sides of the function call.
///Note that it is required to assign a value to all entries on the left hand side of the equation.
///Dailing to do so will produce an error in matlab.
void mexFunction(int nlhs,mxArray *plhs[],int nrhs, const mxArray *prhs[])
{
    if (nrhs!=nlhs) {printf("We need the same number of parameters on the left and right hand side\n");return;}
    Compute_float *FirstV,*SecondV,*FirstW,*SecondW;
    mexmap mexmappings[] = {
        //name in matlab , outputtable , pointer  , when which layer , is recovery variable , initial min value              , initial max value
        {"Vsingle_layer" , "V1"        , &FirstV  , SINGLELAYER      , OFF                  , OneLayerModel.potential.Vrt    , OneLayerModel.potential.Vpk}    ,
        {"Wsingle_layer" , "W1"        , &FirstW  , SINGLELAYER      , ON                   , Zero                           , Zero}                           ,
        {"Vin"           , "V1"        , &FirstV  , DUALLAYER        , OFF                  , DualLayerModelIn.potential.Vrt , DualLayerModelIn.potential.Vpk} ,
        {"Vex"           , "V2"        , &SecondV , DUALLAYER        , OFF                  , DualLayerModelEx.potential.Vrt , DualLayerModelEx.potential.Vpk} ,
        {"Win"           , "Recovery1" , &FirstW  , DUALLAYER        , ON                   , Zero                           , Zero}                           ,
        {"Wex"           , "Recovery2" , &SecondW , DUALLAYER        , ON                   , Zero                           , Zero}                           ,
        {0               , 0           , 0        , 0                , 0                    , 0                              , 0}};
    //the entries in this array are the first column of the previous array
    mxArray* variables = mxCreateStructMatrix(1,1,6,(const char*[]){"Vin","Vex","Win","Wex","Vsingle_layer","Wsingle_layer"});
    if (setup_done==0)
    {
        srandom((unsigned)time(0));
            if (ModelType==SINGLELAYER) {m=setup(OneLayerModel,OneLayerModel,ModelType,jobnumber,yossarianjobnumber,0);} //pass the same layer as a double parameter
            else {m=setup(DualLayerModelIn,DualLayerModelEx,ModelType,jobnumber,yossarianjobnumber,0);}
        int i = 0;
        while(mexmappings[i].name != NULL)
        {
            if (ModelType==mexmappings[i].Lno && (Features.Recovery==ON || Features.Recovery==mexmappings[i].recovery))
            {
                mxSetField(variables,0,mexmappings[i].name,CreateInitialValues(mexmappings[i].init_min,mexmappings[i].init_max));
            }
            i++;
        }
        setup_done=1;
    }
    else
    {
        //step the model through time
        //First - get inputs
        int i = 0;
        while(mexmappings[i].name != NULL)
        {
            if (ModelType==mexmappings[i].Lno && (Features.Recovery==ON || Features.Recovery==mexmappings[i].recovery))
            {
                (*mexmappings[i].data) =(Compute_float* ) mxGetData(mxGetField(prhs[0],0,mexmappings[i].name));
            }
            i++;
        }
        //Actually step the model
        step_(FirstV,SecondV,FirstW,SecondW);
        //set outputs
        i=0;
        while(mexmappings[i].name != NULL)
        {
            if (ModelType==mexmappings[i].Lno && (Features.Recovery==ON || Features.Recovery==mexmappings[i].recovery))
            {
                mxSetField(variables,0,mexmappings[i].name,outputToMxArray(GetOutputByName(mexmappings[i].outname)));
            }
            i++;
        }
    }
    plhs[0] = variables;
    outputExtraThings(plhs,nrhs,prhs);
    return;
}
#else
#ifndef _WIN32
//we don't have long options for win32 - so this part gets skipped - note that this means things like the cluster will break on win32
///Structure which holds the command line options that the program recognises
struct option long_options[] = {{"help",no_argument,0,'h'},{"generate",no_argument,0,'g'},{"sweep",required_argument,0,'s'},{"nocv",no_argument,0,'n'},{"nosegfault",no_argument,0,'f'},{0,0,0,0}};

void processopts (int argc,char** argv,parameters** newparam,parameters** newparamEx,parameters** newparamIn,on_off* OpenCv)
{
    while (1)
    {
        int option_index=0;
        int c=getopt_long(argc,argv,"hgns:f",long_options,&option_index);
        if (c==-1) {break;} //end of options
        switch (c)
        {
            case 'h':
                printf("available arguments:\n"
                        "   -h --help print this message\n"
                        "   -g --generate generate a yossarian config file\n"
                        "   -n --nocv disable open cv viewer\n"
                        "   -s --sweep N do the nth element of a sweep\n"
                        "   -f --nosegfault prevents almost all segfaults - very reliable method");
                exit(EXIT_SUCCESS);
            case 'g':
                *OpenCv = OFF; //need to disable gui for generation of yossarin file
                if (ModelType == SINGLELAYER)
                {
                    createyossarianfile("yossarian.csh",Sweep,OneLayerModel,OneLayerModel); //It doesn't really matter too much, but use the original parameters here
                }
                else
                {
                    createyossarianfile("yossarian.csh",Sweep,DualLayerModelIn,DualLayerModelEx);
                }
                exit(EXIT_SUCCESS);
            case 's':
                {
                    yossarianjobnumber=atoi(optarg);
                    printf("doing sweep index %i\n",yossarianjobnumber);
                    if (ModelType == SINGLELAYER)
                    {
                        *newparam = GetNthParam(OneLayerModel,Sweep,(unsigned int)yossarianjobnumber);
                    }
                    else
                    {
                        *newparamEx =Sweep.SweepEx==ON?GetNthParam(DualLayerModelEx,Sweep,(unsigned int)yossarianjobnumber):NULL;
                        *newparamIn =Sweep.SweepIn==ON?GetNthParam(DualLayerModelIn,Sweep,(unsigned int)yossarianjobnumber):NULL;
                    }
                }
                break;
            case 'n':
                *OpenCv = OFF;
                break;
            case 'f':
                exit(EXIT_SUCCESS);
        }
    }
}
#else
void processopts(int argc, char** argv, parameters** newparam, parameters** newparamEx, parameters** newparamIn, on_off* OpenCv)
{
	//TODO: command line not done on windows - note that it might actually be possible to get this to work - but I don't really see the point
}
#endif



///Main function for the entire program
/// @param argc number of cmdline args
/// @param argv what the parameters actually are
int main(int argc,char** argv) //useful for testing w/out matlab
{
#ifndef ANDROID //android doesn't support this function - note the error is that this will fail at linking so it needs to hide in the #if
 //   feenableexcept(FE_INVALID | FE_OVERFLOW); //segfault on NaN and overflow.  Note - this cannot be used in matlab - tends to produce many false positives so leave off, although can be interesting sometimes
#endif
    parameters* newparam = NULL;
    parameters* newparamEx = NULL;
    parameters* newparamIn = NULL;
    processopts(argc,argv,&newparam,&newparamEx,&newparamIn,&showimages);

    const Job* job = &Features.job;
    if (job->next != NULL || (job->initcond==RAND_JOB && job->Voltage_or_count>1)) {jobnumber=0;} //if more than one job - then start at 0 - so that stuff goes in folders
    while (job != NULL)
    {
        int count = job->initcond==RAND_JOB?(int)job->Voltage_or_count:1; //default to 1 job
        for (int c = 0;c<count;c++)
        {
            seedrand(job->initcond,c,yossarianjobnumber);
            //sets up the model code
            //lets create the actual parameters we use
            const parameters actualsingle = newparam  !=NULL ? *newparam  :OneLayerModel; //TODO: it is not going to be hard to remove these
            const parameters actualDualIn = newparamIn!=NULL ? *newparamIn:DualLayerModelIn;
            const parameters actualDualEx = newparamEx!=NULL ? *newparamEx:DualLayerModelEx;
            if (ModelType==SINGLELAYER) {m=setup(actualsingle,actualsingle,ModelType,jobnumber,yossarianjobnumber,0);} //pass the same layer as a double parameter
            else {m=setup(actualDualIn,actualDualEx,ModelType,jobnumber,yossarianjobnumber,0);}
//            SaveModel(m);
            Compute_float *FirstV,*SecondV,*FirstW,*SecondW;
            setuppointers(&FirstV,&SecondV,&FirstW,&SecondW,job,&actualsingle,&actualDualIn,&actualDualEx);
            inittimer();
            const unsigned int printfreq = 200;
            //actually runs the model
            while (m->timesteps<Features.Simlength)
            {
                //every now and then we print some statistics about when we are going to be finished - 
                if (m->timesteps%printfreq==0){timertick(m->timesteps,Features.Simlength);}
                step_(FirstV,SecondV,FirstW,SecondW);//always fine to pass an extra argument here
                //copy the output to be new input - this does seem slightly inelegant.  There is definitely room for improvement here
                memcpy ( FirstV, m->layer1.voltages.Out, sizeof ( Compute_float)*grid_size*grid_size);
                if(SecondV != NULL){memcpy(SecondV,m->layer2.voltages.Out, sizeof(Compute_float)*grid_size*grid_size);}
                if(FirstW != NULL) {memcpy(FirstW, m->layer1.recoverys.Out,sizeof(Compute_float)*grid_size*grid_size);}
                if(SecondW != NULL){memcpy(SecondW,m->layer2.recoverys.Out,sizeof(Compute_float)*grid_size*grid_size);}
            }
            FreeIfNotNull(FirstV);
            FreeIfNotNull(SecondV);
            FreeIfNotNull(FirstW);
            FreeIfNotNull(SecondW);
            CleanupModel(m);
            jobnumber++;
        }
        job=job->next;
    }
    return(EXIT_SUCCESS);
}
#ifdef ANDROID
#include <jni.h>
int android_setup_done=0;
Compute_float *FirstV,*SecondV,*FirstW,*SecondW;
//nice riduclously long function name
JNIEXPORT jdoubleArray JNICALL Java_com_example_conductanceandroid_MainActivity_AndroidEntry(JNIEnv* env, jobject jboj)
{
    if (android_setup_done==0)
    {
        android_setup_done=1;
        __android_log_print(ANDROID_LOG_VERBOSE,APPNAME,"starting code");
        const Job* job = &Features.job;
        srandom((unsigned)time(0));
        //sets up the model code
        m=setup(DualLayerModelIn,DualLayerModelEx,ModelType,0);
        setuppointers(&FirstV,&SecondV,&FirstW,&SecondW,job);
        __android_log_print(ANDROID_LOG_VERBOSE,APPNAME,"setup done");
    }
    //actually runs the model
    for (int i=0;i<5;i++)
    {
        step_(FirstV,SecondV,FirstW,SecondW);//always fine to pass an extra argument here
        //copy the output to be new input
        memcpy ( FirstV, m->layer1->voltages_out, sizeof ( Compute_float)*grid_size*grid_size);
        if(SecondV != NULL){memcpy(SecondV,m->layer2->voltages_out, sizeof(Compute_float)*grid_size*grid_size);}
        if(FirstW != NULL) {memcpy(FirstW, m->layer1->recoverys_out,sizeof(Compute_float)*grid_size*grid_size);}
        if(SecondW != NULL){memcpy(SecondW,m->layer2->recoverys_out,sizeof(Compute_float)*grid_size*grid_size);}
    }
    jdoubleArray ret = (*env) ->NewDoubleArray(env,grid_size*grid_size);
    (*env)->SetDoubleArrayRegion(env, ret, 0, grid_size*grid_size, SecondV );
    return ret;
}
#endif
#endif
