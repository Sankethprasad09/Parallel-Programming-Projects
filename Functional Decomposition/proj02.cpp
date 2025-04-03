// CS 575 Project 2 Functional Decomposition
// Sanketh Karuturi, karutusa@oregonstate.edu

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <omp.h>
#include <ctime>

using namespace std;

// Constants representing environmental factors
const float GRAIN_GROWS_PER_MONTH = 12.0; // Rate at which grain grows
const float ONE_DEER_EATS_PER_MONTH = 1.0; // Amount of grain a deer eats per month

const float AVG_PRECIP_PER_MONTH = 7.0;   // Average monthly precipitation
const float AMP_PRECIP_PER_MONTH = 6.0;   // Amplitude of variation in precipitation
const float RANDOM_PRECIP = 2.0;          // Random noise in precipitation

const float AVG_TEMP = 60.0;              // Average temperature
const float AMP_TEMP = 20.0;              // Temperature amplitude
const float RANDOM_TEMP = 10.0;           // Random temperature noise

const float MIDTEMP = 40.0; // Midpoint temperature for growth calculations          
const float MIDPRECIP = 10.0;   // Midpoint precipitation for growth calculations

// State variables of the simulation
int NowYear = 2024;
int NowMonth = 0;

float NowPrecip;
float NowTemp;
float NowHeight = 5.0;  // Initial height of the grain
int NowNumDeer = 2; // Initial deer population
int NowNumHumans = 5;  // Initial human population affecting the environment

unsigned int seed = 0; // Seed for random number generation

float Ranf(float low, float high) {
    float r = (float)rand_r(&seed);  // Random goodness
    return (low + r * (high - low) / (float)RAND_MAX);
}

// Barrier synchronization variables
omp_lock_t Lock;
volatile int NumInThreadTeam;
volatile int NumAtBarrier;
volatile int NumGone;

// Initialize the barrier for thread synchronization
void InitBarrier(int n) {
    NumInThreadTeam = n;
    NumAtBarrier = 0;
    omp_init_lock(&Lock);
}

// Synchronize all threads at the barrier
void WaitBarrier() {
    omp_set_lock(&Lock);
    NumAtBarrier++;
    if (NumAtBarrier == NumInThreadTeam) {
        NumGone = 0;
        NumAtBarrier = 0;
        while (NumGone != NumInThreadTeam - 1);
        omp_unset_lock(&Lock);
    } else {
        omp_unset_lock(&Lock);
        while (NumAtBarrier != 0);
        #pragma omp atomic
        NumGone++;
    }
}

// Updates the environmental conditions based on the month
void UpdateEnvironment() {
    float ang = (30.0 * (float)NowMonth + 15.0) * (M_PI / 180.0);

    float temp = AVG_TEMP - AMP_TEMP * cos(ang);
    NowTemp = temp + Ranf(-RANDOM_TEMP, RANDOM_TEMP);

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
    NowPrecip = precip + Ranf(-RANDOM_PRECIP, RANDOM_PRECIP);
    if (NowPrecip < 0.0) {
        NowPrecip = 0.0;
    }
}

// Simulate the deer population dynamics
void Deer() {
    while (NowYear < 2030) {
        int nextNumDeer = NowNumDeer;
        int carryingCapacity = (int)(NowHeight);

        if (nextNumDeer < carryingCapacity)
            nextNumDeer++;
        else if (nextNumDeer > carryingCapacity)
            nextNumDeer--;

        if (nextNumDeer < 0)
            nextNumDeer = 0;

        WaitBarrier();
        NowNumDeer = nextNumDeer;
        WaitBarrier();
        WaitBarrier();
    }
}

// Simulate the growth and consumption of grain
void Grain() {
    while (NowYear < 2030) {
        float tempFactor = exp(-((NowTemp - MIDTEMP) / 10) * ((NowTemp - MIDTEMP) / 10));
        float precipFactor = exp(-((NowPrecip - MIDPRECIP) / 10) * ((NowPrecip - MIDPRECIP) / 10));

        float nextHeight = NowHeight;
        nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
        if (nextHeight < 0.) nextHeight = 0.;

        WaitBarrier();
        NowHeight = nextHeight;
        WaitBarrier();
        WaitBarrier();
    }
}

// Monitor and report the state of the simulation
void Watcher() {
    while (NowYear < 2030) {
        WaitBarrier();
        WaitBarrier();

        cout << "Year: " << NowYear << ", Month: " << NowMonth << "\n";
        cout << "Temperature: " << NowTemp << " F, Precipitation: " << NowPrecip << " in\n";
        cout << "Number of Deer: " << NowNumDeer << ", Grain Height: " << NowHeight << " in\n";
        cout << "Number of Humans: " << NowNumHumans << "\n\n";

        if (++NowMonth > 11) {
            NowMonth = 0;
            NowYear++;
        }

        UpdateEnvironment();

        WaitBarrier();
    }
}

// Human impact on the environment
void Humans() {
    while (NowYear < 2030) {
        int nextNumHumans = NowNumHumans;

        if (NowMonth == 4) {  // Example: increase during a specific month
            nextNumHumans += 1;  
        }

        WaitBarrier();
        NowNumHumans = nextNumHumans;
        WaitBarrier();
        WaitBarrier();
    }
}

int main() {
    srand(time(NULL));  // Seed the random number generator

    omp_set_num_threads(4);  // Set the number of threads in OMP
    InitBarrier(4);          // Initialize the barrier for synchronization

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            Deer();
        }
        #pragma omp section
        {
            Grain();
        }
        #pragma omp section
        {
            Watcher();
        }
        #pragma omp section
        {
            Humans();  // Custom agent affecting the simulation
        }
    }

    return 0;
}
