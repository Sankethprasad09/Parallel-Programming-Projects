// CS 575 Project #1
// Sanketh Karuturi, karutusa@oregonstate.edu

#define _USE_MATH_DEFINES // Define constants in <cmath>
#include <math.h>         // Include the math library for sqrt and fabs
#include <stdlib.h>       // Standard library for rand and srand
#include <stdio.h>        // Standard I/O for output
#include <time.h>         // For time functions
#include <omp.h>          // OpenMP library for parallel programming

// Conditional compilation to enable/disable debugging messages
#ifndef DEBUG
#define DEBUG false
#endif

// setting the number of threads:
#ifndef NUMT
#define NUMT 4
#endif

// setting the number of trials in the Monte Carlo simulation:
#ifndef NUMTRIALS
#define NUMTRIALS 1000000
#endif

// how many tries to discover the maximum performance:
#define NUMTRIES 20

#define CSV

// Physical and simulation constants:
#define GRAVITY 32.2f // Acceleration due to gravity, ft/s^2

// Given parameters for the simulation
const float BEFOREY = 80.0f; // Height at which the ball is set in motion
const float AFTERY  = 20.0f; // Height at which the ball exits the ski jump horizontally
const float RADIUS  = 3.0f;  // Radius of the hole
const float DISTX   = 70.0f; // Horizontal distance to the center of the hole from the ski jump

// Tolerances
const float BEFOREYDY = 5.0f;
const float AFTERYDY  = 1.0f;
const float DISTXDX   = 5.0f;
float BeforeY[NUMTRIALS];
float AfterY[NUMTRIALS];
float DistX[NUMTRIALS];

float Ranf(float low, float high) {
    float r = (float) rand();               // 0 - RAND_MAX
    float t = r / (float) RAND_MAX;         // 0. - 1.
    return low + t * (high - low);
}

// Function prototypes
void TimeOfDaySeed() {
    struct tm y2k = {0};
    y2k.tm_hour = 0; y2k.tm_min = 0; y2k.tm_sec = 0;
    y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

    time_t timer;
    time(&timer);
    double seconds = difftime(timer, mktime(&y2k));
    unsigned int seed = (unsigned int)(1000.0 * seconds);
    srand(seed);
}

int main(int argc, char *argv[]) {
    #ifdef _OPENMP
    #ifndef CSV
        fprintf(stderr, "OpenMP is supported -- version = %d\n", _OPENMP);
    #endif
    #else
        fprintf(stderr, "No OpenMP support!\n");
        return 1;
    #endif

    TimeOfDaySeed(); // Seed the random number generator
    omp_set_num_threads(NUMT); // Set the number of threads to use

    // Fill the arrays with random values based on given means and tolerances
    for (int n = 0; n < NUMTRIALS; n++) {
        BeforeY[n] = Ranf(BEFOREY - BEFOREYDY, BEFOREY + BEFOREYDY);
        AfterY[n]  = Ranf(AFTERY - AFTERYDY, AFTERY + AFTERYDY);
        DistX[n]   = Ranf(DISTX - DISTXDX, DISTX + DISTXDX);
    }

    double maxPerformance = 0.; // must be declared outside the NUMTRIES loop
    int numSuccesses = 0;       // must be declared outside the loop to hold cumulative successes

    for (int tries = 0; tries < NUMTRIES; tries++) {
        double time0 = omp_get_wtime();
        numSuccesses = 0;

        #pragma omp parallel for reduction(+:numSuccesses)
        for (int n = 0; n < NUMTRIALS; n++) {
            float vx = sqrt(2 * GRAVITY * (BeforeY[n] - AfterY[n]));
            float t = sqrt(2 * AfterY[n] / GRAVITY);
            float dx = vx * t;
            if (fabs(dx - DistX[n]) <= RADIUS) {
                numSuccesses++;
            }
        }

        double time1 = omp_get_wtime();
        double megaTrialsPerSecond = (double)NUMTRIALS / (time1 - time0) / 1000000.;
        if (megaTrialsPerSecond > maxPerformance) {
            maxPerformance = megaTrialsPerSecond;
        }
    }

    float probability = (float)numSuccesses / (float)(NUMTRIALS);

#ifdef CSV
    fprintf(stderr, "%2d, %8d, %6.2f, %6.2lf\n",
            NUMT, NUMTRIALS, 100.0 * probability, maxPerformance);
#else
    fprintf(stderr, "Threads: %d, Trials: %d, Probability: %.6f%%, MegaTrials/Sec: %.2f\n",
            NUMT, NUMTRIALS, 100.0 * probability, maxPerformance);
#endif

    return 0;
}


