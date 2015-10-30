//
// Created by alex on 30/10/2015.
//

#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


bool is_dir(char *path) {
    struct stat s;
    return stat(path, &s) == 0 && S_ISDIR(s.st_mode);
}