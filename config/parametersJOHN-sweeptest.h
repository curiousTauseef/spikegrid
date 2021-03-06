/// \file
#include <stddef.h> //offsetof
//these first few parameters actually escape into the paramheader file through magic
#define grid_size 100
///Total size of the grid
///Coupling range
#define couplerange 25
#ifndef PARAMETERS  //DO NOT REMOVE
///include guard
#define PARAMETERS  //DO NOT REMOVE
//disable warnings about float conversion in this file only
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
//the following typedef must be before the include to get the right compute types
///Whether we are using the single or double layer model
static const LayerNumbers ModelType = DUALLAYER;

//Fun note - with the right optimisations GCC actually will pull these constants inline (for example disassemble evolvept_STDP with STDP off)
///Parameters for the single layer model
static const parameters OneLayerModel = {.couple={0}}; //since unused - shortes possible definition that produces no warnings

#define potparams .potential =     \
    {                                  \
        .type    =                     \
        {                           \
            .type = LIF,            \
        },                          \
        .Vrt     = -70,             \
        .Vpk    = -55,              \
        .Vlk     = -70,             \
        .Vex     = 0,               \
        .Vin     = -80,             \
        .glk     = 0.05,            \
        .rate = 0,                  \
    }

#define STDPparams .STDP=   \
    {                       \
        .stdp_limit=1.5,    \
        .stdp_tau=20,       \
        .stdp_strength=0.00055,  \
        .STDP_on=ON\
    }
#define STDparams .STD= \
    {                   \
        .U = 0.5,       \
        .D = 0.2,      \
        .F = 0.45      \
    }
#define Stimparams .Stim=\
{\
    .timeperiod=220,\
    .lag=55,\
}
//#define SWEEP (prop,v,min,max)
///parameters for the inhibitory layer of the double layer model
static const parameters DualLayerModelIn =
{
    .couple =
    {
        .Layertype = DUALLAYER,
        .Layer_parameters =
        {
            .dual =
            {
                .W          = -0.5, //-0.40 //-0.57 //-0.70 //-1.25,
                .sigma      = 90,
                .synapse    = {.R=0.5,.D=2.0},
            }
        },

        .norm_type = GlobalMultiplier,
        .normalization_parameters = {.glob_mult = {.GM=1.0}},
        .tref       = 5,
    },
    .random =
    {
        .numberper={int t=1; 100},
        .str = 0.8
    },
    STDparams,
    STDPparams,
    potparams,
    Stimparams,
    .skip=2,
};
///parameters for the excitatory layer of the double layer model
static const parameters DualLayerModelEx =
{
    .couple =   
    {
        .Layertype = DUALLAYER,
        .Layer_parameters =
        {
            .dual =
            {
                .W          =  0.25,
                .sigma      = 20,
                .synapse    = {.R=0.5,.D=2.0},
            }
        },
        .tref       = 5,
        .norm_type = GlobalMultiplier,
        .normalization_parameters = {.glob_mult = {.GM=1.0}},
    },
    .random =
    {
        .numberper=706,
        .str = 0.8
    },
    STDparams,
    STDPparams,
    potparams,
    .skip=-2,
    Stimparams,
//    .output = {{ .method=PICTURE,.Output=5,.Delay=10} }
};
///Some global features that can be turned on and off
static const model_features Features =
{
    .STD        = OFF,
    .STDP		= ON, //Question - some of these do actually make more sense as a per-layer feature - just about everything that isn't the timestep -
    .Random_connections = OFF,
    .Timestep   = 0.1,
    .Simlength  = 1000000,
    .ImageStim  = OFF,
    .job        = {.initcond = SINGLE_SPIKE, .Voltage_or_count = -70},
    .Disablewrapping = ON,
};
///Constant external input to conductances
static const extinput Extinput =
{
    .gE0 = 0.000,
    .gI0 = 0.15,
};
///Parameters for conducting a parameter sweep.
static const sweepable Sweep =
{
    .offset=offsetof(parameters,Stim)+offsetof(Stimulus_parameters,lag) ,
    .minval = 0.000,
    .maxval = 100,
    .count = 100
};

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif
#endif //DO NOT REMOVE
