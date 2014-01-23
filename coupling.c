#include "paramheader.h"
#include "coupling.h" //not actually required at the moment but should ensure that function types match
#include <tgmath.h> //logf / exp
#include <stdlib.h> //calloc
#include <stdio.h>  //printf

/* //This function is useful - but not used
   Compute_float __attribute__((const))exrange(const couple_parameters c)
   {
   return -(c.sigE*c.sigI*logf(c.WE/c.WI))/(c.sigE-c.sigI); //from mathematica
   }
   */
//check how far back we need to keep track of histories
unsigned int setcap(const decay_parameters d,const Compute_float minval, const Compute_float dt)
{
    Compute_float prev = -1000;//initial values
    Compute_float time=0;
    Compute_float norm=One/(d.D-d.R);
    while(1)
    {
        time+=dt;
        Compute_float alpha=(exp(-time/d.D) - exp(-time/d.R))*norm;
        // check that the spike is in the decreasing phase and that it has magnitude less than the critical value
        if (alpha<minval && alpha<prev) {break;}
        prev=alpha;
    }
    return (unsigned int)(time/dt) +1; //this keeps compatibility with the matlab - seems slightly inelegent - maybe remove
}
//normalize the coupling matrix
Compute_float* Norm_couplematrix(const couple_parameters c, Compute_float* const unnormed)
{
    switch (c.norm_type)
    {
        case None:
            return unnormed;
        case TotalArea:
            {   //Apparently we need a separate scope to declare the variables in - C is weird
                Compute_float plusval = 0;
                Compute_float negval = 0;
                for (int i=0;i<couple_array_size*couple_array_size;i++)
                {
                    const Compute_float val = unnormed[i];
                    if (val < 0) {negval += val;} else {plusval += val;}

                }
                const Compute_float plusnorm = c.normalization_parameters.total_area.WE/plusval;
                const Compute_float negnorm = c.normalization_parameters.total_area.WI/negval;
                for (int i=0;i<couple_array_size*couple_array_size;i++)
                {
                    if (unnormed[i] < 0) {unnormed[i]  *= plusnorm;} else {unnormed[i] *= negnorm;}

                }
                return unnormed;
            }
        case GlobalMultiplier:
            {
                for (int i=0;i<couple_array_size*couple_array_size;i++)
                {
                    unnormed[i] *= c.normalization_parameters.glob_mult.GM;
                }
                return unnormed;
            }
        default:
            printf("unknown normalization method\n");
            return NULL;
    }
}


//compute the mexican hat function used for coupling - should really be marked forceinline or whatever the notation is for GCC.
Compute_float mexhat(const Compute_float rsq,const couple_parameters c){return c.WE*exp(-rsq/c.sigE)-c.WI*exp(-rsq/c.sigI);}

//does what it says on the tin
Compute_float* CreateCouplingMatrix(const couple_parameters c)
{
    Compute_float* matrix = calloc(sizeof(Compute_float),couple_array_size*couple_array_size); //matrix of coupling values
    for(int x=-couplerange;x<=couplerange;x++)
    {
        for(int y=-couplerange;y<=couplerange;y++)
        {
            if (x*x+y*y<=couplerange*couplerange)//if we are within coupling range
            {
                Compute_float val = mexhat((Compute_float)(x*x+y*y),c);//compute the mexican hat function
                if (val>0) {val=val*c.SE;} else {val=val*c.SI;}//and multiply by some constants
                matrix[(x+couplerange)*couple_array_size + y + couplerange] = val;//and set the array
            }
        }
    }
    return Norm_couplematrix(c, matrix);
}

