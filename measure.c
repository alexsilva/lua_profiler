//
// Created by alex on 14/10/2015.
//

#include "measure.h"


float calc_elapsed_time(clock_t start, clock_t end) {
    return ((float)(end - start) / CLOCKS_PER_SEC);
}

float calc_time_spent(Measure *measure) {
    return calc_elapsed_time(measure->begin, measure->end);
}