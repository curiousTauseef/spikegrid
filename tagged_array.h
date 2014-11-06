/// \file
#include "typedefs.h"
///used for storing arrays with their size.  Allows for the matlab_output (and other) function to take both the big and large arrays
typedef struct {
    //we require volatile below as we don't want you to be able to write to an array using the pointer from the tagged array
    //however, other parts of the code could modify the underlying array, so use volatile to force reads
    const volatile Compute_float* const data; ///< the actual data
    const unsigned int size;                  ///< the total dimensions
    const unsigned int offset;                ///< offset (used by the gE and gI matrices
    const unsigned int subgrid;               ///< used when there is a subgrid within the grid - currently the only example is STDP.  The default value of this should be 1 - often not implemented
    const Compute_float minval;               ///< minimum value in array (for a colorbar - currently unused)
    const Compute_float maxval;               ///< maximum value in array (for a colorbar - currently unused)
} tagged_array;
Compute_float* taggedarrayTocomputearray(const tagged_array input);

unsigned int __attribute__((const)) tagged_array_size(const tagged_array in);