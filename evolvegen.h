#include "typedefs.h"
void evolvept_duallayer (const unsigned int x,const unsigned int y,const Compute_float* const __restrict connections,const Compute_float strmod, Compute_float* __restrict condmat);
void evolvept_duallayer_STDP (const int x,const int y,const Compute_float* const __restrict connections,const Compute_float* const __restrict STDP_connections,const Compute_float strmod, Compute_float* __restrict condmat);
