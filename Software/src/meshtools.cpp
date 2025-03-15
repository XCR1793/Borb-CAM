#include "meshtools.h"

bool meshtools::loadModel(const char *filename){
    model = LoadModel(filename);
    return true;
}

