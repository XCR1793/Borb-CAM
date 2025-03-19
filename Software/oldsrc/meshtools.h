#ifndef MESHTOOLS
#define MESHTOOLS
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>


typedef struct {
    Vector3 *vertices;
    Vector3 *normals;
    int vertexCount;
} STLModel;

class meshtools{
    public:

    bool loadModel(const char *filename);

    

    // bool convertSTL_Mesh(STLModel model);

    Model model;
};

#endif