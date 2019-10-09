#ifndef _particleFilterDefs_h_
#define _particleFilterDefs_h_

/******************************************************************************
 FILTER GENERAL PARAMETERS
******************************************************************************/
#ifndef MAX_PARTICLES	   //maximum number of particles allowed
#define MAX_PARTICLES 10000 //10000 //14400 //3000 //20000 //
#endif

#ifndef USE_AUG_MCL      //boolean indicating if augmented MCL algorithm should 
#define USE_AUG_MCL 0    //be used  
#endif

#ifndef PARTICLESTOFILE
#define PARTICLESTOFILE 1
#endif 

#ifndef HISTOGRAMTOFILE
#define HISTOGRAMTOFILE 0
#endif 

// filter state should be saved as particles as a histogram.
#ifndef SAVE_PARTICLES
#define SAVE_PARTICLES HISTOGRAMTOFILE
#endif 

/******************************************************************************
 FILTER RESAMPLING PARAMETERS
******************************************************************************/
#ifndef MIN_EFF_SAMP_SIZE   //Multiplies nParticles to determine the minimum 
#define MIN_EFF_SAMP_SIZE 0.75 // effective sample size used in the decision of
#endif                         // whether to resample the particle distribution.

#ifndef MIN_NUM_SOUNDINGS   //Minimum number of soundings on which weights must
#define MIN_NUM_SOUNDINGS 1 //be based in order for resampling to occur.
#endif		//TODO: make this number larger, possibly on number of time steps rather than number of beams

#ifndef N_RESAMP_STDDEV     //Std dev of Gaussian noise added to resampled 
#define N_RESAMP_STDDEV 0   //particle position N coordinate
#endif

#ifndef E_RESAMP_STDDEV     //Std dev of Gaussian noise added to resampled 
#define E_RESAMP_STDDEV 0   //particle position E coordinate
#endif

#ifndef Z_RESAMP_STDDEV     //Std dev of Gaussian noise added to resampled 
#define Z_RESAMP_STDDEV 0   //particle position Z coordinate
#endif

#ifndef PHI_RESAMP_STDDEV    //Std dev of Gaussian noise added to resampled 
#define PHI_RESAMP_STDDEV 0  //particle phi coordinate
#endif

#ifndef THETA_RESAMP_STDDEV   //Std dev of Gaussian noise added to resampled 
#define THETA_RESAMP_STDDEV 0 //particle theta coordinate
#endif

#ifndef PSI_RESAMP_STDDEV     //Std dev of Gaussian noise added to resampled
#define PSI_RESAMP_STDDEV 0   //particle psi coordinate
#endif

#ifndef DNDT_RESAMP_STDDEV    //Std dev of Gaussian noise added to resampled
#define DNDT_RESAMP_STDDEV 0  //particle terrain North rate coordinate 
#endif

#ifndef DEDT_RESAMP_STDDEV    //Std dev of Gaussian noise added to resampled 
#define DEDT_RESAMP_STDDEV 0  //particle terrain East rate coordinate
#endif

#ifndef DHDT_RESAMP_STDDEV    //Std dev of Gaussian noise added to resampled
#define DHDT_RESAMP_STDDEV 0  //particle terrain Heading rate coordinate
#endif
#endif
