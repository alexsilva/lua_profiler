//
// Created by alex on 14/10/2015.
//

#include "measure.h"

double calc_time_spent(Measure *measure) {
    return  ((measure->end - measure->begin) / CLOCKS_PER_SEC);
}