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

Vector3 mesh::RotXYD_XYZ(Vector3 distance_rotX_rotY) {
    float d = distance_rotX_rotY.x;
    float rx = distance_rotX_rotY.y;
    float ry = distance_rotX_rotY.z;

    // Compute unit normal vector
    float a = sin(ry) * cos(rx);
    float b = -sin(rx);
    float c = cos(ry) * cos(rx);

    // Normalize the normal
    float length = sqrt(a*a + b*b + c*c);
    a /= length;
    b /= length;
    c /= length;

    // Return unit normal vector; the plane equation is ax + by + cz = d
    return (Vector3){a, b, c};
}



std::pair<Vector3, bool> mesh::IntersectLinePlane(Vector4 planeNormal, Vector3 lineStart, Vector3 lineEnd) {
    Vector3 lineDir = Vector3Subtract(lineEnd, lineStart);  // Direction of the line
    Vector3 normal = { planeNormal.x, planeNormal.y, planeNormal.z };  // Extract plane normal (a, b, c)
    float denom = Vector3DotProduct(normal, lineDir);  // Dot product between plane normal and line direction

    if (fabsf(denom) < 1e-6f) {
        // Line is parallel to the plane
        return { Vector3Zero(), false };
    }

    // Compute t using the plane equation ax + by + cz = d
    float numerator = planeNormal.w - Vector3DotProduct(normal, lineStart);
    float t = numerator / denom;

    if (t < 0.0f || t > 1.0f) {
        // Intersection is outside the segment
        return { Vector3Zero(), false };
    }

    Vector3 intersection = Vector3Add(lineStart, Vector3Scale(lineDir, t));
    return { intersection, true };
}

std::pair<Triangle, bool> mesh::IntersectTrianglePlane(Vector4 planeNormal, Triangle triangle){
    std::pair<Triangle, bool> Intersecting = {triangle, 0};
    if(IntersectLinePlane(planeNormal, triangle.Vertex1, triangle.Vertex2).second){Intersecting.second = 1;}
    if(IntersectLinePlane(planeNormal, triangle.Vertex2, triangle.Vertex3).second){Intersecting.second = 1;}
    if(IntersectLinePlane(planeNormal, triangle.Vertex3, triangle.Vertex1).second){Intersecting.second = 1;}
    return Intersecting;
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


int mesh::Triangle_Touching(Triangle first, Triangle second){
    int i = 0;

    if( (first.Vertex1 == second.Vertex1) || 
        (first.Vertex1 == second.Vertex2) || 
        (first.Vertex1 == second.Vertex3)) i++;

    if( (first.Vertex2 == second.Vertex1) || 
        (first.Vertex2 == second.Vertex2) || 
        (first.Vertex2 == second.Vertex3)) i++;

    if( (first.Vertex3 == second.Vertex1) || 
        (first.Vertex3 == second.Vertex2) || 
        (first.Vertex3 == second.Vertex3)) i++;

    return i;
}


std::vector<std::pair<int, Triangle>> mesh::Sort_Triangles(std::vector<std::pair<int, Triangle>> Unsorted_Triangles) {
    std::vector<std::pair<int, Triangle>> Sorted;

    if (Unsorted_Triangles.empty()) return Sorted;

    // Start with the first triangle
    Sorted.push_back(Unsorted_Triangles[0]);
    Unsorted_Triangles.erase(Unsorted_Triangles.begin());

    while (!Unsorted_Triangles.empty()) {
        int maxTouching = -1;
        size_t bestIndex = 0;

        for (size_t i = 0; i < Unsorted_Triangles.size(); ++i) {
            int touching = Triangle_Touching(Sorted.back().second, Unsorted_Triangles[i].second);
            if (touching > maxTouching) {
                maxTouching = touching;
                bestIndex = i;
            }
        }

        // Add the best matching triangle to the sorted list
        Sorted.push_back(Unsorted_Triangles[bestIndex]);
        Unsorted_Triangles.erase(Unsorted_Triangles.begin() + bestIndex);
    }

    return Sorted;
}


std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> mesh::Intersecting_Triangles(Model &model, Vector4 Coeff_abcd){
    std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> Sorted_Triangles_Islands_Accounted;
    std::vector<std::vector<std::pair<int, Triangle>>> Sorted_Triangle_List;
    std::vector<std::pair<int, Triangle>> Unsorted_List_Intersecting_Triangles;
    std::vector<std::pair<int, Triangle>> Sorted_List_Intersecting_Triangles;
    std::vector<std::vector<std::pair<int, Triangle>>> Triangle_List = List_Triangles(model);

    for(auto perMesh : Triangle_List){
        int i = 0;
        Unsorted_List_Intersecting_Triangles.clear();
        Sorted_List_Intersecting_Triangles.clear();
        for(auto perTriangle : perMesh){
            std::pair<Triangle, bool> Intersecting_Triangle = IntersectTrianglePlane(Coeff_abcd, perTriangle.second);
            if(Intersecting_Triangle.second){Unsorted_List_Intersecting_Triangles.push_back(std::make_pair(i, Intersecting_Triangle.first));}
        }

        Sorted_List_Intersecting_Triangles = Sort_Triangles(Unsorted_List_Intersecting_Triangles);

        Sorted_Triangle_List.push_back(Sorted_List_Intersecting_Triangles);
        
        i++;
    }

    std::vector<std::vector<std::pair<int, Triangle>>> Islands;
    std::vector<std::pair<int, Triangle>> Island;

    for(auto& perMesh : Sorted_Triangle_List){
        if(perMesh.empty()) continue;
    
        Island.push_back(perMesh.at(0));
        for(size_t i = 1; i < perMesh.size(); i++){
            if(Triangle_Touching(perMesh.at(i).second, perMesh.at(i-1).second)){
                Island.push_back(perMesh.at(i));
            }else{
                Islands.push_back(Island);
                Island.clear();
                Island.push_back(perMesh.at(i));
            }
        }

        Islands.push_back(Island);
        Island.clear();
    
        Sorted_Triangles_Islands_Accounted.push_back(Islands);
        Islands.clear();
    }
    

    return Sorted_Triangles_Islands_Accounted;
}



std::vector<Line> mesh::Intersect_Model(Model &model, Vector4 Coeff_abcd){
    std::vector<std::pair<std::pair<std::pair<Vector3, Vector3>, int>, int>> intersectionList;
    std::vector<Line> Lines;

    std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> Triangle_List = Intersecting_Triangles(model, Coeff_abcd);

    int meshNo = 0;
    int island = 0;

    for(auto perMesh : Triangle_List){
        for(auto perIsland : perMesh){
            for(auto perTriangle : perIsland){
                std::pair<Vector3, bool> Intersection1 = IntersectLinePlane(Coeff_abcd, perTriangle.second.Vertex1, perTriangle.second.Vertex3);
                std::pair<Vector3, bool> Intersection2 = IntersectLinePlane(Coeff_abcd, perTriangle.second.Vertex2, perTriangle.second.Vertex1);
                std::pair<Vector3, bool> Intersection3 = IntersectLinePlane(Coeff_abcd, perTriangle.second.Vertex3, perTriangle.second.Vertex2);
            
                if(Intersection1.second){intersectionList.push_back(std::make_pair(std::make_pair(std::make_pair(Intersection1.first, (Vector3){}), island), meshNo));}
                if(Intersection2.second){intersectionList.push_back(std::make_pair(std::make_pair(std::make_pair(Intersection2.first, (Vector3){}), island), meshNo));}
                if(Intersection3.second){intersectionList.push_back(std::make_pair(std::make_pair(std::make_pair(Intersection3.first, (Vector3){}), island), meshNo));}
            }
            island++;
        }
        meshNo++;
    }

    if(intersectionList.size() < 2){
        return Lines;
    }
    
    for (size_t i = 0; i < intersectionList.size() - 1; i++) {
        auto& current = intersectionList[i];
        auto& next = intersectionList[i + 1];
    
        if (current.second == next.second &&
            current.first.second == next.first.second) {
    
            Lines.push_back((Line){
                .startLinePos = current.first.first.first,
                .startLineRot = current.first.first.second,
                .endLinePos = next.first.first.first,
                .endLineRot = next.first.first.second,
                .type = 1,
                .meshNo = current.second,
                .islandNo = current.first.second
            });
        }
    }

    return Lines;
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