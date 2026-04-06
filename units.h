#ifndef UNITS_H
#define UNITS_H


#define DISTANCE_UNIT 1

#define SIZE_UNIT 1
#define MASS_UNIT 1

#define VELOCITY_UNIT 1

// Use double for DATA (faster than long double on most platforms)
#define DATA double
// SMALL_DATA for performance-critical temporary math (single-precision)
#define SMALL_DATA float


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif // UNITS_H