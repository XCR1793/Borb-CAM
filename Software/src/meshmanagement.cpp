#include "meshmanagement.h"

/**##########################################
 * #            Button Functions            #
 * ##########################################*/

void mesh::Add_Model(int id, const char *model_path){
    if(!ID_Check(id, models)){
        models.push_back((multimodel){id, LoadModel(model_path)});
        UploadMesh(&Ret_Model(id).meshes[0], true);
    }
}

void mesh::Add_Model(int id, Model model){
    if(!ID_Check(id, models)){
        models.push_back((multimodel){id, model});

        UploadMesh(&model.meshes[0], true);
    }
}

void mesh::Rem_Model(int id){
    if(!models.empty()){
        for(std::vector<multimodel>::size_type it = 0; it < models.size(); it++){
            if(models.at(it).id == id){
                UnloadModel(models.at(it).model);
                models.erase(models.begin() + it);
                it--;
            }
        }
    }
}

Model mesh::Ret_Model(int id){
    if(!models.empty()){
        for(std::vector<multimodel>::size_type it = 0; it < models.size(); it++){
            if(models.at(it).id == id){
                return models.at(it).model;
            }
        }
    }
    return Model();
}

void mesh::Rep_Model(int id, Vector3 position){
    if(!models.empty()){
        for(std::vector<multimodel>::size_type it = 0; it < models.size(); it++){
            if(models.at(it).id == id){
                models.at(it).pos = position;
            }
        }
    }
}

void mesh::Reu_Model(int id, Model model){
    if(!models.empty()){
        for(std::vector<multimodel>::size_type it = 0; it < models.size(); it++){
            if(models.at(it).id == id){
                models.at(it).model = model;
            }
        }
    }
}

void mesh::Sha_Model(Shader shader){
    for(auto it : models){
        it.model.materials[0].shader = shader;
    }
}

void mesh::Sha_Model(int id, Shader shader){
    Model model = Ret_Model(id);
    model.materials[0].shader = shader;
    Reu_Model(id, model);
}

int mesh::CNT_Models(){
    if(!models.empty()){
        return models.size();
    }
    return -1;
}

void mesh::Run_Models(){
    if(!models.empty()){
        for(auto it : models){
            DrawModel(it.model, (Vector3){it.pos.x, it.pos.y, it.pos.z}, it.scale, it.colour);
        }
    }
}

void mesh::Stop_Models(){
    if(!models.empty()){
        for(auto it : models){
            UnloadModel(it.model);
        }
    }
}

/**##########################################
 * #             Maths Functions            #
 * ##########################################*/

float mesh::Rad_Deg(float radians){
    return (radians*(180/PI));
}

Vector3 mesh::Rad_Deg(Vector3 angles){
    return ((Vector3){Rad_Deg(angles.x), Rad_Deg(angles.y), Rad_Deg(angles.z)});
}

float mesh::Deg_Rad(float degrees){
    return (degrees*(PI/180));
}

Vector3 mesh::Deg_Rad(Vector3 angles){
    return ((Vector3){Deg_Rad(angles.x), Deg_Rad(angles.y), Deg_Rad(angles.z)});
}

Vector3 mesh::RotXYD_XYZ(Vector3 distance_xrot_yrot){
    return ((Vector3){sin(distance_xrot_yrot.y)*cos(distance_xrot_yrot.x), -sin(distance_xrot_yrot.x), cos(distance_xrot_yrot.y)*cos(distance_xrot_yrot.x)});
}

std::pair<Vector3, bool> mesh::IntersectLinePlane(Vector3 planeNormal, Vector3 lineStart, Vector3 lineEnd) {
    Vector3 lineDir = Vector3Subtract(lineEnd, lineStart);
    float denom = Vector3DotProduct(planeNormal, lineDir);

    if (fabsf(denom) < 1e-6f) {
        // Line is parallel to the plane
        return { Vector3Zero(), false };
    }

    // Plane goes through origin: planeNormal · P = 0
    float t = -Vector3DotProduct(planeNormal, lineStart) / denom;

    if (t < 0.0f || t > 1.0f) {
        // Intersection not on the segment
        return { Vector3Zero(), false };
    }

    Vector3 intersection = Vector3Add(lineStart, Vector3Scale(lineDir, t));
    return { intersection, true };
}



/**##########################################
 * #       Mesh Manipulation Functions      #
 * ##########################################*/

Model mesh::Scale_Model(Model &model, float scale){
    model.transform = MatrixScale(scale, scale, scale);
    return model;
}

Model mesh::Position_Model(Model &model, Vector3 position){
    model.transform = MatrixMultiply(MatrixTranslate(position.x, position.y, position.z),model.transform);
    return model;
}

Model mesh::Rotate_Model(Model &model, Vector3 rotatiton){
    Matrix rotx = MatrixRotateX(rotatiton.x);
    Matrix roty = MatrixRotateY(rotatiton.y);
    Matrix rotz = MatrixRotateZ(rotatiton.z);
    model.transform = MatrixMultiply(model.transform, MatrixMultiply(MatrixMultiply(rotx, roty), rotz));
    return model;
}


std::vector<std::vector<std::pair<int, Triangle>>> mesh::List_Triangles(Model model){
    std::vector<std::vector<std::pair<int, Triangle>>> All_Triangles;

    for (long i = 0; i < model.meshCount; i++){
        std::vector<std::pair<int, Triangle>> Mesh_Triangles;
        float* vertices = model.meshes[i].vertices;
        long vertexCount = model.meshes[i].vertexCount;

        for (long j = 0; j < vertexCount; j += 3){
            Triangle tri = {
                (Vector3){vertices[(j * 3) + 0], vertices[(j * 3) + 1], vertices[(j * 3) + 2]},
                (Vector3){vertices[(j * 3) + 3], vertices[(j * 3) + 4], vertices[(j * 3) + 5]},
                (Vector3){vertices[(j * 3) + 6], vertices[(j * 3) + 7], vertices[(j * 3) + 8]},
            };

            Mesh_Triangles.emplace_back(i, tri);
        }

        All_Triangles.push_back(Mesh_Triangles);
    }

    return All_Triangles;
}

std::vector<std::pair<Vector3, Vector3>> mesh::Intersect_Model(Model &model, Vector3 distance_xrot_yrot){
    std::vector<std::pair<Vector3, Vector3>> intersectionList;
    Mesh mesh = model.meshes[0];

    for(long i = 0; i < model.meshes->vertexCount; i+= 3){
        if((model.meshes->vertexCount - i) < 3){return intersectionList;}
        if(model.meshes->indices != NULL){

            unsigned short I0 = model.meshes->indices[i];
            unsigned short I1 = model.meshes->indices[i + 3];
            unsigned short I2 = model.meshes->indices[i + 9];

            Vector3 v0 = {mesh.vertices[I0 + 0], mesh.vertices[I0 + 1], mesh.vertices[I0 + 2]};
            Vector3 v1 = {mesh.vertices[I1 + 0], mesh.vertices[I1 + 1], mesh.vertices[I1 + 2]};
            Vector3 v2 = {mesh.vertices[I2 + 0], mesh.vertices[I2 + 1], mesh.vertices[I2 + 2]};
            
            std::pair<Vector3, bool> Intersection0 = IntersectLinePlane(distance_xrot_yrot, v0, v1);
            std::pair<Vector3, bool> Intersection1 = IntersectLinePlane(distance_xrot_yrot, v1, v2);
            std::pair<Vector3, bool> Intersection2 = IntersectLinePlane(distance_xrot_yrot, v2, v0);

            if(Intersection0.second){intersectionList.push_back(std::make_pair(Intersection0.first, Intersection0.first));}
            if(Intersection1.second){intersectionList.push_back(std::make_pair(Intersection1.first, Intersection1.first));}
            if(Intersection2.second){intersectionList.push_back(std::make_pair(Intersection2.first, Intersection2.first));}
        }
    }

    return intersectionList;
}

/**##########################################
 * #            Private Functions           #
 * ##########################################*/

bool mesh::ID_Check(int id, std::vector<multimodel> &list){
    if(!list.empty()){
        for(auto item : list){
            if(item.id == id){return true;}
        }
    }
    return false;
}