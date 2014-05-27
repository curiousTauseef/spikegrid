/// \file
//these first few parameters actually escape into the paramheader file through magic
#define grid_size 100
///Total size of the grid
///Coupling range
#define couplerange 15
#ifndef PARAMATERS  //DO NOT REMOVE
///include guard
#define PARAMATERS  //DO NOT REMOVE
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
//the following typedef must be before the include to get the right compute types
#include "paramheader.h"
///Whether we are using the single or double layer model
static const LayerNumbers ModelType = DUALLAYER;

//Fun note - with the right optimisations GCC actually will pull these constants inline (for example disassemble evolvept_STDP with STDP off)
///Parameters for the single layer model
static const parameters OneLayerModel = 
{
    .couple =
    {
        .Layertype = SINGLELAYER,
        .Layer_parameters = 
        {
            .single = 
            {
                .WE     = 0.42,                 //excitatory coupling strength
                .sigE   = 14,                   //char. length for Ex symapses (int / float?)
                .WI     = 0.19,                 //Inhib coupling strength
                .sigI   = 1000,                   //char. length for In synapses (int / float?)
                .Ex = {.R=0.5,.D=2.0},          //excitatory rise / decay time
                .In = {.R=0.5,.D=2.0},          //inhibitory rise / decay time
            }
        },
        .tref   = 5,
        .norm_type = None,
    },
    .potential = 
    {
        .type    = QIF,                  //how the neuron adds input - LIF (leaky integrate-and-fire), QIF (quadratic integrate-and-fire), or EIF (exponential integrate-and-fire)
        .Vrt     = -70,                  //reset potential
        .Vpk    = -55,                   //peak potential (at which membrane potential is reset -- must match Vth for LIF neurons)
        .extra =
        {
            .EIF={.Dpk = 1,.QIF.Vth=-55},//slope of spike (EIE only)
        },
        .Vlk     = -70,                  //leak reversal potential
        .Vex     = 0,                    //Ex reversal potential
        .Vin     = -80,                  //In reversal potential
        .glk     = 0.05,                 //leak current
        .rate = 1,
    },
    .STDP = 
    {
        .stdp_limit     = 0.1,
        .stdp_tau       = 20,
        .stdp_strength  = 0.0004
    }, 
    .STD =
    {
        .U  = 0.5,
        .D  = 0.11,
        .F  = 0.005
    },
    .Movie = 
    {
        .MakeMovie = OFF,
        .Delay = 10,
    },
    .theta = 
    {
        .strength    = 5.0,
        .period     = 0.2,
    },
    .skip=1,
};
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
                .W          = -1.25, //-0.40 //-0.57 //-0.70 //-1.25, 
                .sigma      = 42, 
                .synapse    = {.R=0.5,.D=3.0},
            }
        },
        .norm_type = None,
        .tref       = 5,
    },
    .potential = 
    {
        .type    = QIF,                  //how the neuron adds input - LIF (leaky integrate-and-fire), QIF (quadratic integrate-and-fire), or EIF (exponential integrate-and-fire)
        .Vrt     = -70,                  //reset potential
        .Vpk    = -55,                   //peak potential (at which membrane potential is reset -- must match Vth for LIF neurons)
        .extra =
        {
            .EIF={.Dpk = 1,.QIF.Vth=-55},//slope of spike (EIE only)
        },
        .Vlk     = -70,                  //leak reversal potential
        .Vex     = 0,                    //Ex reversal potential
        .Vin     = -80,                  //In reversal potential
        .glk     = 0.05,                 //leak current
        .rate = 1,
    },    
    .STDP = 
    {
        .stdp_limit     = 0.1,
        .stdp_tau       = 20,
        .stdp_strength  = 0.0004
    }, 
    .STD =
    {
        .U  = 0.5,
        .D  = 0.11,
        .F  = 0.005
    },
    .Movie = 
    {
        .MakeMovie = OFF,
        .Output = 0,
        .Delay = 10,
    },
    .theta = 
    {
        .strength    = 5.0,
        .period     = 0.2,
    },
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
                .W          =  0.23, //0.09 //0.12 //0.14  //0.23
                .sigma      = 14,
                .synapse    = {.R=0.5,.D=3.0},
            }
        },
        .tref       = 5,
        .norm_type = None,
    },
    .potential = 
    {
        .type    = QIF,                  //how the neuron adds input - LIF (leaky integrate-and-fire), QIF (quadratic integrate-and-fire), or EIF (exponential integrate-and-fire)
        .Vrt     = -70,                  //reset potential
        .Vpk    = -55,                   //peak potential (at which membrane potential is reset -- must match Vth for LIF neurons)
        .extra =
        {
            .EIF={.Dpk = 1,.QIF.Vth=-55},//slope of spike (EIE only)
        },
        .Vlk     = -70,                  //leak reversal potential
        .Vex     = 0,                    //Ex reversal potential
        .Vin     = -80,                  //In reversal potential
        .glk     = 0.05,                 //leak current
        .rate = 1,
    },
    .STDP = 
    {
        .stdp_limit     = 0.1,
        .stdp_tau       = 20,
        .stdp_strength  = 0.0004
    }, 
    .STD =
    {
        .U  = 0.5,
        .D  = 0.11,
        .F  = 0.005
    },
    .Movie = 
    {
        .MakeMovie = ON,
        .Output = 0,
        .Delay = 10,
    },
    .theta = 
    {
        .strength    = 5.0,
        .period     = 0.2,
    },
    .skip=1,
};
///Some global features that can be turned on and off
static const model_features Features = 
{
    .STDP		= OFF, //Question - some of these do actually make more sense as a per-layer feature - just about everything that isn't the timestep - 
    .STD        = OFF, //               if we need any of these features we can make the changes then.
    .Output     = OFF,
    .Theta      = OFF,
    .Timestep   = 0.1,
};
///Parameters for conducting a parameter sweep.
static const sweepable Sweep =
{
    .type = Vrt,
    .minval = 1.0,
    .maxval = 2.0,
    .count = 10
};

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif
#endif //DO NOT REMOVE
