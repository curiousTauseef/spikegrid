/// \file
#include <stdlib.h>
#include "typedefs.h"
#include "sizes.h"
#include "mymath.h"
void ApplyLocalBoost(Compute_float* geIn,const int xcenter,const int ycenter)
{
    for (int x=0;x<grid_size;x++)
    {
        for (int y=0;y<grid_size;y++)
        {
            const int idx = (x+couplerange)*conductance_array_size + y + couplerange; //index for gE/gI
            const Compute_float rsq =(Compute_float) ((x-xcenter)*(x-xcenter) + (y-ycenter)*(y-ycenter));
            const Compute_float str = exp(-rsq/25.0) * 0.1;
            geIn[idx]+=str;
        }
    }
}
int curblock = 0;
void RandomBlocking(Compute_float* geIn,const unsigned int time)
{
    if(time % 100 == 0)
    {
        curblock = (int)random() % 2;
    }
    for (int x=0;x<grid_size;x++)
    {
        for (int y=0;y<grid_size;y++)
        {

            const int idx = (x+couplerange)*conductance_array_size + y + couplerange; //index for gE/gI
            if ((curblock ==0 && x<y) || (curblock == 1 && x>y)) {geIn[idx]= 0.0;}
        }
    }

}
