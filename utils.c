//
// Created by alex on 30/10/2015.
//

#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>


bool is_dir(char *path) {
    struct stat s;
    return stat(path, &s) == 0 && S_ISDIR(s.st_mode);
}

int lua_gettop(lua_State *L) {
    return L->Cstack.num;
}