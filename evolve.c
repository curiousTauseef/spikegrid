#include "parameters.h"
#include "layer.h"
#include "parameters.h"
#include "evolve.h"
#include "helpertypes.h"
#include "STD.h"
#include <string.h> //memset
#include <tgmath.h> //exp
#include <stdio.h> //printf
#include <stdlib.h> //random
//add gE/gI when using STDP - untested
//when STDP is turned off, gcc will warn about this function needing const.  It is wrong
void evolvept_STDP  (const int x,const  int y,const Compute_float* const __restrict connections_STDP,const Compute_float Estrmod,const Compute_float Istrmod,Compute_float* __restrict gE,Compute_float* __restrict gI)
{
    //ex coupling
    if (Param.features.STDP == OFF) {return;}
    for (int i = 0; i < couple_array_size;i++)
    {
        const int outoff = (x +i )*conductance_array_size +y;//as gE and gI are larger than the neuron grid size, don't have to worry about wrapping
        for (int j = 0 ; j<couple_array_size;j++) 
        {
            const int coupleidx =(x*grid_size +y)*couple_array_size*couple_array_size + i*couple_array_size + j;
            if (connections_STDP[coupleidx] > 0) //add either to gE or gI
            {
                gE[outoff+j] += connections_STDP[coupleidx]*Estrmod;
            }
            else
            {
                gI[outoff+j] += -(connections_STDP[coupleidx]*Istrmod);
            }
        }
    } 
}

//add conductance from a firing neuron the gE and gI arrays
////TODO: add the skip part to skip zero entries as in the threestate code
void evolvept (const int x,const  int y,const Compute_float* const __restrict connections,const Compute_float Estrmod,const Compute_float Istrmod,Compute_float* __restrict gE,Compute_float* __restrict gI,const Compute_float* STDP_CONNS)
{
    //ex coupling
    
    for (int i = 0; i < couple_array_size;i++)
    {
        const int outoff = (x +i )*conductance_array_size +y;//as gE and gI are larger than the neuron grid size, don't have to worry about wrapping
        for (int j = 0 ; j<couple_array_size;j++) 
        {
            const int coupleidx = i*couple_array_size + j;
            if (connections[coupleidx] > 0) //add either to gE or gI
            {
                gE[outoff+j] += connections[coupleidx]*Estrmod;
            }
            else
            {
                gI[outoff+j] += -(connections[coupleidx]*Istrmod);
            }
        }
    } 
    evolvept_STDP(x,y,STDP_CONNS,Estrmod,Istrmod,gE,gI);
}
void fixboundary(Compute_float* __restrict gE, Compute_float* __restrict gI)
{
    //top + bottom
    for (int i=0;i<couplerange;i++)
	{
        for (int j=0;j<conductance_array_size;j++)
		{
            gE[(grid_size+i)*conductance_array_size + j] +=gE[i*conductance_array_size+j]; //add to bottom
            gE[(i+couplerange)*conductance_array_size+j] += gE[(grid_size+couplerange+i)*conductance_array_size+j];//add to top
            gI[(grid_size+i)*conductance_array_size + j] +=gI[i*conductance_array_size+j]; //add to bottom
            gI[(i+couplerange)*conductance_array_size+j] += gI[(grid_size+couplerange+i)*conductance_array_size+j];//add to top
		}
	}
    //left + right boundary condition fix
    for (int i=couplerange;i<couplerange+grid_size;i++)
	{
        for (int j=0;j<couplerange;j++)
		{
             gE[i*conductance_array_size +grid_size+j ] += gE[i*conductance_array_size+j];//left
             gE[i*conductance_array_size +couplerange+j] += gE [i*conductance_array_size + grid_size+couplerange+j];//right
             gI[i*conductance_array_size +grid_size+j ] += gI[i*conductance_array_size+j];//left
             gI[i*conductance_array_size +couplerange+j] += gI [i*conductance_array_size + grid_size+couplerange+j];//right
		}
	}

}
//rhs_func used when integrating the neurons forward through time
Compute_float __attribute__((const)) rhs_func  (const Compute_float V,const Compute_float gE,const Compute_float gI) {return -(Param.misc.glk*(V-Param.potential.Vlk) + gE*(V-Param.potential.Vex) + gI*(V-Param.potential.Vin));}
//step the model through time
void step1 (layer_t* layer,const int time)
{

    static Compute_float gE[conductance_array_size*conductance_array_size];
    static Compute_float gI[conductance_array_size*conductance_array_size];
    coords* current_firestore = layer->spikes.data[layer->spikes.curidx];//get the thing for currently firing neurons
    memset(gE,0,sizeof(Compute_float)*conductance_array_size*conductance_array_size); //zero the gE/gI matrices so they can be reused
    memset(gI,0,sizeof(Compute_float)*conductance_array_size*conductance_array_size);
    for (int i=1;i<layer->spikes.count;i++) //start at 1 so we don't get currently firing (which should be empty anyway)
    {
        coords* fire_with_this_lag;//this is a bit of a funny definition due to macros.
        RINGBUFFER_GETOFFSET(layer->spikes,i,fire_with_this_lag)
        const Compute_float delta = ((Compute_float)i)*Param.time.dt;//small helper constant
        const Compute_float Estr = (One/(Param.synapse.taudE-Param.synapse.taurE))*(exp(-delta/Param.synapse.taudE)-exp(-delta/Param.synapse.taurE));
        const Compute_float Istr = (One/(Param.synapse.taudI-Param.synapse.taurI))*(exp(-delta/Param.synapse.taudI)-exp(-delta/Param.synapse.taurI));
        int idx=0; //iterate through all neurons firing with this lag
        while (fire_with_this_lag[idx].x != -1)
        {
            coords c = fire_with_this_lag[idx]; //add conductances
            Compute_float strmod=One;
            if (Param.features.STD == ON)
            {
                const int stdidx=c.x*grid_size+c.y;
                if (i==1)
                {
                    const Compute_float dt = ((Compute_float)(time-STD.ftimes[stdidx]))/1000.0/Param.time.dt;//calculate inter spike interval in seconds
                    STD.ftimes[stdidx]=time; //update the time
                    const Compute_float prevu=STD.U[stdidx]; //need the previous U value
                    STD.U[stdidx] = Param.STD.U + STD.U[stdidx]*(One-Param.STD.U)*exp(-dt/Param.STD.F);
                    STD.R[stdidx] = One + (STD.R[stdidx] - prevu*STD.R[stdidx] - One)*exp(-dt/Param.STD.D);
                }
                strmod = STD.U[stdidx] * STD.R[stdidx] * 2.0; //multiplication by 2 is not in the cited papers, but you could eliminate it by multiplying some other parameters by 2, but multiplying by 2 here enables easier comparison with the non-STD model.  Max has an improvement that calculates a first-order approxiamation that should be included
            }
            evolvept(c.x,c.y,layer->connections,Estr*strmod,Istr*strmod,gE,gI,layer->STDP_connections);
            idx++;
        }
    }
    fixboundary(gE,gI);
    for (int x=0;x<grid_size;x++) 
    {
        for (int y=0;y<grid_size;y++)
        { //step all neurons through time - use midpoint method
            const int idx = (x+couplerange)*conductance_array_size + y + couplerange; //index for gE/gI
            const int idx2=  x*grid_size+y;//index for voltages
            const Compute_float rhs1=rhs_func(layer->voltages[idx2],gE[idx],gI[idx]);
            const Compute_float Vtemp = layer->voltages[idx2] + 0.5*Param.time.dt*rhs1;
            const Compute_float rhs2=rhs_func(Vtemp,gE[idx],gI[idx]);
            layer->voltages_out[idx2]=layer->voltages[idx2]+0.5*Param.time.dt*(rhs1 + rhs2);
        }
    }
    int this_fcount=0;//now find which neurons fired at the current time step
    for (int x=0;x<grid_size;x++)
    {
        for (int y=0;y<grid_size;y++)
        {
            if (layer->voltages[x*grid_size + y]  > Param.potential.Vth)
            {
                const coords c= {x,y}; //required to keep the compiler happy - can't do an inline constructor
                current_firestore[this_fcount] =c;
                layer->voltages_out[x*grid_size+y]=Param.potential.Vrt;
                this_fcount++;
                if (Param.features.Output==ON)
                {
                    printf("%i,%i;",x,y);
                }
            }
            else if (((Compute_float)random())/((Compute_float)RAND_MAX) < (Param.misc.rate*0.001*Param.time.dt))
            {
                layer->voltages_out[x*grid_size+y]=Param.potential.Vth+0.1;//make sure it fires
            }
        }
    }
    //todo: fix for time scale properly
    for (int i=1;i<=51;i++) //start at 1 so we don't get currently firing (which should be empty anyway)
    {   //put refractory neurons at reset potential
        coords* fire_with_this_lag;//this is a bit of a funny definition due to macros.
        RINGBUFFER_GETOFFSET(layer->spikes,i,fire_with_this_lag)
        int idx=0;
        while (fire_with_this_lag[idx].x != -1)
        {
            coords c = fire_with_this_lag[idx]; 
            layer->voltages_out[c.x*grid_size+c.y]=Param.potential.Vrt;
            idx++;
        }
    }
    current_firestore[this_fcount].x=-1;
    if (Param.features.Output==ON &&time % 10 ==0 ) {printf("\n");}

}
