#include "meshmanagement.h"

/**##########################################
 * #            Button Functions            #
 * ##########################################*/

void mesh::Add_Model(int id, const char *model_path){
    if(!ID_Check(id, models)){
        models.push_back((multimodel){id, LoadModel(model_path)});
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

Model mesh::FaceOffset_Model(Model &model, float offset) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh &mesh = model.meshes[i];

        if (!mesh.vertices || !mesh.normals || mesh.triangleCount == 0)
            continue;

        int vertexCount = mesh.vertexCount;
        std::vector<Vector3> newVertices(vertexCount);

        Vector3 *vertices = (Vector3 *)mesh.vertices;
        Vector3 *normals = (Vector3 *)mesh.normals;

        // Modify vertex positions based on their normals
        for (int j = 0; j < vertexCount; j++) {
            newVertices[j] = Vector3Add(vertices[j], Vector3Scale(normals[j], offset));
        }

        // Allocate new memory for modified vertices
        float *newVertexData = new float[vertexCount * 3];

        // Copy the data manually without memcpy
        for (int j = 0; j < vertexCount; j++) {
            newVertexData[j * 3] = newVertices[j].x;
            newVertexData[j * 3 + 1] = newVertices[j].y;
            newVertexData[j * 3 + 2] = newVertices[j].z;
        }

        // Assign new vertex array to the mesh
        mesh.vertices = newVertexData;
    }

    return model;  // Return modified model
}




bool mesh::ID_Check(int id, std::vector<multimodel> &list){
    if(!list.empty()){
        for(auto item : list){
            if(item.id == id){return true;}
        }
    }
    return false;
}