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

// Vector3 mesh::NormalToRotation(Vector3 normal){
//     normal = Vector3Normalize(normal);
//     float pitch = asinf(-normal.y);
//     float yaw = atan2f(normal.x, normal.z);
//     pitch = pitch * (180.0f / PI);
//     yaw = yaw * (180.0f / PI);
//     return (Vector3){ pitch, yaw, 0.0f };
// }

Vector3 mesh::NormalToRotation(Vector3 normal){
    normal = Vector3Normalize(normal);
    
    // Optional: Flip if normals are reversed
    // normal = Vector3Negate(normal);

    float pitch = asinf(normal.y);  // Up is positive pitch
    float yaw = atan2f(normal.x, normal.z);

    pitch *= RAD2DEG;
    yaw   *= RAD2DEG;

    return (Vector3){ pitch, yaw, 0.0f };
}


std::pair<Point, bool> mesh::IntersectLinePlane(Vector4 planeNormal, Point lineStart, Point lineEnd){
    Vector3 lineDir = Vector3Subtract(lineEnd.Position, lineStart.Position);
    Vector3 normal = {planeNormal.x, planeNormal.y, planeNormal.z};
    float denom = Vector3DotProduct(normal, lineDir);

    if(fabsf(denom) < 1e-6f){return {{}, false};}

    float numerator = planeNormal.w - Vector3DotProduct(normal, lineStart.Position);
    float t = numerator / denom;

    if(t < 0.0f || t > 1.0f){return {{}, false};}

    Vector3 intersection = Vector3Add(lineStart.Position, Vector3Scale(lineDir, t));
    Vector3 interpolatedNormal = Vector3Normalize(Vector3Add(
        Vector3Scale(lineStart.Normal, 1.0f - t),
        Vector3Scale(lineEnd.Normal, t)
    ));

    return {{.Position = intersection, .Normal = interpolatedNormal}, true};
}

std::pair<Triangle, bool> mesh::IntersectTrianglePlane(Vector4 planeNormal, Triangle triangle){
    std::pair<Triangle, bool> Intersecting = {triangle, 0};
    if(IntersectLinePlane(planeNormal, triangle.Vertex1, triangle.Vertex2).second){Intersecting.second = 1;}
    if(IntersectLinePlane(planeNormal, triangle.Vertex2, triangle.Vertex3).second){Intersecting.second = 1;}
    if(IntersectLinePlane(planeNormal, triangle.Vertex3, triangle.Vertex1).second){Intersecting.second = 1;}
    return Intersecting;
}

Vector3 mesh::MovePointAlongNormal3D(Vector3 startPoint, Vector3 normal, float distance){
    Vector3 unitNormal = Vector3Normalize(normal);
    Vector3 offset = Vector3Scale(unitNormal, distance);
    Vector3 newPoint = Vector3Add(startPoint, offset);
    return newPoint;
}

bool mesh::RayIntersectsAABB(Vector3 rayOrigin, Vector3 rayDir, BoundingBox box, Vector3* out){
    float tmin = -INFINITY;
    float tmax = INFINITY;

    if(fabs(rayDir.x) > 0.0001f){
        float tx1 = (box.min.x - rayOrigin.x) / rayDir.x;
        float tx2 = (box.max.x - rayOrigin.x) / rayDir.x;
        tmin = fmaxf(tmin, fminf(tx1, tx2));
        tmax = fminf(tmax, fmaxf(tx1, tx2));
    }else if(rayOrigin.x < box.min.x || rayOrigin.x > box.max.x){
        return false;
    }

    if(fabs(rayDir.y) > 0.0001f){
        float ty1 = (box.min.y - rayOrigin.y) / rayDir.y;
        float ty2 = (box.max.y - rayOrigin.y) / rayDir.y;
        tmin = fmaxf(tmin, fminf(ty1, ty2));
        tmax = fminf(tmax, fmaxf(ty1, ty2));
    }else if(rayOrigin.y < box.min.y || rayOrigin.y > box.max.y){
        return false;
    }

    if(fabs(rayDir.z) > 0.0001f){
        float tz1 = (box.min.z - rayOrigin.z) / rayDir.z;
        float tz2 = (box.max.z - rayOrigin.z) / rayDir.z;
        tmin = fmaxf(tmin, fminf(tz1, tz2));
        tmax = fminf(tmax, fmaxf(tz1, tz2));
    }else if(rayOrigin.z < box.min.z || rayOrigin.z > box.max.z){
        return false;
    }

    if(tmax >= fmaxf(tmin, 0.0f)){
        if (out) *out = Vector3Add(rayOrigin, Vector3Scale(rayDir, tmin));
        return true;
    }

    return false;
}

Vector3 mesh::PointToVec3(Point point){
    return (Vector3){point.Position.x, point.Position.y, point.Position.z};
}

/**##########################################
 * #       Mesh Manipulation Functions      #
 * ##########################################*/

multimodel mesh::Scale_Model(multimodel &mmodel, float scale){
    mmodel.scale = scale;
    Matrix scaleMat = MatrixScale(scale, scale, scale);
    mmodel.model.transform = MatrixMultiply(scaleMat, mmodel.model.transform);
    return mmodel;
}

multimodel mesh::Position_Model(multimodel &mmodel, Vector3 position){
    mmodel.pos = position;
    Matrix translateMat = MatrixTranslate(position.x, position.y, position.z);
    mmodel.model.transform = MatrixMultiply(mmodel.model.transform, translateMat);
    return mmodel;
}

multimodel mesh::Rotate_Model(multimodel &mmodel, Vector3 rotation){
    mmodel.rot = rotation;
    Matrix rotX = MatrixRotateX(rotation.x);
    Matrix rotY = MatrixRotateY(rotation.y);
    Matrix rotZ = MatrixRotateZ(rotation.z);
    Matrix rotMat = MatrixMultiply(MatrixMultiply(rotX, rotY), rotZ);
    mmodel.model.transform = MatrixMultiply(mmodel.model.transform, rotMat);
    return mmodel;
}

multimodel mesh::Apply_Transformations(multimodel &mmodel){
    Matrix scaleMat = MatrixScale(mmodel.scale, mmodel.scale, mmodel.scale);
    Matrix rotX = MatrixRotateX(mmodel.rot.x);
    Matrix rotY = MatrixRotateY(mmodel.rot.y);
    Matrix rotZ = MatrixRotateZ(mmodel.rot.z);
    Matrix rotMat = MatrixMultiply(MatrixMultiply(rotX, rotY), rotZ);
    Matrix transMat = MatrixTranslate(mmodel.pos.x, mmodel.pos.y, mmodel.pos.z);
    Matrix transform = MatrixMultiply(MatrixMultiply(scaleMat, rotMat), transMat);

    for(int i = 0; i < mmodel.model.meshCount; i++){
        Mesh &mesh = mmodel.model.meshes[i];
        for(int j = 0; j < mesh.vertexCount; j++){
            Vector3 vertex = {
                mesh.vertices[j * 3 + 0],
                mesh.vertices[j * 3 + 1],
                mesh.vertices[j * 3 + 2]
            };
            vertex = Vector3Transform(vertex, transform);
            mesh.vertices[j * 3 + 0] = vertex.x;
            mesh.vertices[j * 3 + 1] = vertex.y;
            mesh.vertices[j * 3 + 2] = vertex.z;
        }

        if(mesh.normals != nullptr){
            Matrix normalTransform = transform;
            normalTransform.m12 = 0;
            normalTransform.m13 = 0;
            normalTransform.m14 = 0;

            for(int j = 0; j < mesh.vertexCount; j++){
                Vector3 normal = {
                    mesh.normals[j * 3 + 0],
                    mesh.normals[j * 3 + 1],
                    mesh.normals[j * 3 + 2]
                };
                normal = Vector3Transform(normal, normalTransform);
                normal = Vector3Normalize(normal);
                mesh.normals[j * 3 + 0] = normal.x;
                mesh.normals[j * 3 + 1] = normal.y;
                mesh.normals[j * 3 + 2] = normal.z;
            }
        }
    }

    return mmodel;
}

std::vector<std::vector<std::pair<int, Triangle>>> mesh::List_Triangles(Model model){
    std::vector<std::vector<std::pair<int, Triangle>>> All_Triangles;

    for(long i = 0; i < model.meshCount; i++){
        std::vector<std::pair<int, Triangle>> Mesh_Triangles;
        float* vertices = model.meshes[i].vertices;
        float* normals = model.meshes[i].normals;
        long vertexCount = model.meshes[i].vertexCount;

        for(long j = 0; j < vertexCount; j += 3){
            Vector3 v1 = {vertices[(j * 3) + 0], vertices[(j * 3) + 1], vertices[(j * 3) + 2]};
            Vector3 v2 = {vertices[(j * 3) + 3], vertices[(j * 3) + 4], vertices[(j * 3) + 5]};
            Vector3 v3 = {vertices[(j * 3) + 6], vertices[(j * 3) + 7], vertices[(j * 3) + 8]};

            Vector3 n1 = {}, n2 = {}, n3 = {};

            if(normals != nullptr){
                n1 = {normals[(j * 3) + 0], normals[(j * 3) + 1], normals[(j * 3) + 2]};
                n2 = {normals[(j * 3) + 3], normals[(j * 3) + 4], normals[(j * 3) + 5]};
                n3 = {normals[(j * 3) + 6], normals[(j * 3) + 7], normals[(j * 3) + 8]};
            }else{
                // Fallback: compute face normal and assign it to all three points
                Vector3 edge1 = Vector3Subtract(v2, v1);
                Vector3 edge2 = Vector3Subtract(v3, v1);
                Vector3 faceNormal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));
                n1 = n2 = n3 = faceNormal;
            }

            Triangle tri = {
                .Vertex1 = {v1, n1},
                .Vertex2 = {v2, n2},
                .Vertex3 = {v3, n3}
            };

            Mesh_Triangles.emplace_back(i, tri);
        }

        All_Triangles.push_back(Mesh_Triangles);
    }

    return All_Triangles;
}

int mesh::Triangle_Touching(Triangle first, Triangle second){
    int i = 0;

    if( (first.Vertex1.Position == second.Vertex1.Position) || 
        (first.Vertex1.Position == second.Vertex2.Position) || 
        (first.Vertex1.Position == second.Vertex3.Position)) i++;

    if( (first.Vertex2.Position == second.Vertex1.Position) || 
        (first.Vertex2.Position == second.Vertex2.Position) || 
        (first.Vertex2.Position == second.Vertex3.Position)) i++;

    if( (first.Vertex3.Position == second.Vertex1.Position) || 
        (first.Vertex3.Position == second.Vertex2.Position) || 
        (first.Vertex3.Position == second.Vertex3.Position)) i++;

    return i;
}

std::vector<std::pair<int, Triangle>> mesh::Sort_Triangles(std::vector<std::pair<int, Triangle>> Unsorted_Triangles) {
    std::vector<std::pair<int, Triangle>> Sorted;

    if(Unsorted_Triangles.empty()){return Sorted;}

    // Start with the first triangle
    Sorted.push_back(Unsorted_Triangles[0]);
    Unsorted_Triangles.erase(Unsorted_Triangles.begin());

    while(!Unsorted_Triangles.empty()){
        int maxTouching = -1;
        size_t bestIndex = 0;

        for(size_t i = 0; i < Unsorted_Triangles.size(); ++i){
            int touching = Triangle_Touching(Sorted.back().second, Unsorted_Triangles[i].second);
            if(touching > maxTouching){
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
    std::vector<Line> Lines;
    std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> Triangle_List = Intersecting_Triangles(model, Coeff_abcd);

    int meshNo = 0;
    int island = 0;

    for(auto &perMesh : Triangle_List){
        island = 0;
        for(auto &perIsland : perMesh){
            for (auto &perTriangle : perIsland){
                Triangle tri = perTriangle.second;
                std::vector<Point> intersections;

                auto i1 = IntersectLinePlane(Coeff_abcd, tri.Vertex1, tri.Vertex3);
                auto i2 = IntersectLinePlane(Coeff_abcd, tri.Vertex2, tri.Vertex1);
                auto i3 = IntersectLinePlane(Coeff_abcd, tri.Vertex3, tri.Vertex2);

                if(i1.second) intersections.push_back(i1.first);
                if(i2.second) intersections.push_back(i2.first);
                if(i3.second) intersections.push_back(i3.first);

                if(intersections.size() == 2){
                    Lines.push_back((Line){
                        .startLinePoint = intersections[0],
                        .endLinePoint = intersections[1],
                        .type = 1,
                        .meshNo = meshNo,
                        .islandNo = island
                    });
                }
            }
            island++;
        }
        meshNo++;
    }

    return Lines;
}

bool mesh::CheckCollisionPointBox(Vector3 point, BoundingBox box){
    return (point.x >= box.min.x && point.x <= box.max.x) &&
           (point.y >= box.min.y && point.y <= box.max.y) &&
           (point.z >= box.min.z && point.z <= box.max.z);
}

std::vector<Line> mesh::Cull_Lines_ByBox(BoundingBox box, const std::vector<Line> &lines){
    std::vector<Line> result;

    for(int i = 0; i < lines.size(); i++){
        Vector3 a = lines[i].startLinePoint.Position;
        Vector3 b = lines[i].endLinePoint.Position;

        bool aInside = CheckCollisionPointBox(a, box);
        bool bInside = CheckCollisionPointBox(b, box);

        if(!aInside && !bInside){
            result.push_back(lines[i]);
        }else if(aInside && bInside){
            continue;
        }else{
            Vector3 outside = aInside ? b : a;
            Vector3 inside  = aInside ? a : b;
            Vector3 dir     = Vector3Normalize(Vector3Subtract(inside, outside));

            Vector3 intersection;
            if(RayIntersectsAABB(outside, dir, box, &intersection)){
                Line clipped = lines[i];
                if(aInside){
                    clipped.startLinePoint.Position = intersection;
                    clipped.endLinePoint.Position   = b;
                }else{
                    clipped.startLinePoint.Position = a;
                    clipped.endLinePoint.Position   = intersection;
                }
                result.push_back(clipped);
            }
        }
    }

    return result;
}

float mesh::pointToPointDistance(Vector3 StartPoint, Vector3 EndPoint){
    float dx = EndPoint.x - StartPoint.x;
    float dy = EndPoint.y - StartPoint.y;
    float dz = EndPoint.z - StartPoint.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

float mesh::distancePoint(std::vector<Line> lineList){
    float distance = 0;
    for(auto line : lineList){
        distance += pointToPointDistance(PointToVec3(line.startLinePoint), PointToVec3(line.endLinePoint));
    }
    return distance;
}

Point mesh::lastPoint(std::vector<Line> lineList, int startNo, bool direction){
    if(lineList.empty() || startNo < 0 || startNo >= (int)lineList.size()){
        return {}; // Return default Point if invalid input
    }

    Point expected = lineList[startNo].endLinePoint;
    int listSize = (int)lineList.size();
    int index = startNo;

    for(int count = 0; count < listSize; ++count){
        index = direction ? (index + 1) % listSize
                          : (index - 1 + listSize) % listSize;

        Line& currentLine = lineList[index];

        if(pointToPointDistance(currentLine.startLinePoint.Position, expected.Position) < 0.001f){
            expected = currentLine.endLinePoint;
        }else if(pointToPointDistance(currentLine.endLinePoint.Position, expected.Position) < 0.001f){
            expected = currentLine.startLinePoint;
        }else{
            break; // Chain is broken
        }
    }

    return expected;
}



bool mesh::IsPointInsideMesh(Vector3 point, const std::vector<Triangle>& triangles){
    Vector3 rayDir = {1.0f, 0.123f, 0.456f}; // Skewed to avoid edge cases
    int hitCount = 0;

    for(const Triangle& tri : triangles){
        Vector3 v0 = tri.Vertex1.Position;
        Vector3 v1 = tri.Vertex2.Position;
        Vector3 v2 = tri.Vertex3.Position;

        Vector3 e1 = Vector3Subtract(v1, v0);
        Vector3 e2 = Vector3Subtract(v2, v0);
        Vector3 h  = Vector3CrossProduct(rayDir, e2);
        float a    = Vector3DotProduct(e1, h);

        if(fabs(a) < 0.00001f) continue;

        float f = 1.0f / a;
        Vector3 s = Vector3Subtract(point, v0);
        float u = f * Vector3DotProduct(s, h);
        if(u < 0.0f || u > 1.0f) continue;

        Vector3 q = Vector3CrossProduct(s, e1);
        float v = f * Vector3DotProduct(rayDir, q);
        if(v < 0.0f || u + v > 1.0f) continue;

        float t = f * Vector3DotProduct(e2, q);
        if(t > 0.0001f) hitCount++;
    }

    return (hitCount % 2) == 1;
}

std::vector<Triangle> mesh::Flatten_Triangles_Excluding(Model model, int excludeMesh){
    std::vector<Triangle> result;
    std::vector<std::vector<std::pair<int, Triangle>>> tris = List_Triangles(model);

    for(int i = 0; i < tris.size(); i++){
        if(i == excludeMesh) continue;
        for(auto &pair : tris[i]){
            result.push_back(pair.second);
        }
    }

    return result;
}

std::vector<Line> mesh::Trim_Lines_ByModel(Model model, const std::vector<Line> &lines){
    std::vector<Line> result;

    std::vector<Triangle> triangles;
    std::vector<std::vector<std::pair<int, Triangle>>> tris = List_Triangles(model);
    for(auto &meshTris : tris){
        for(auto &pair : meshTris){
            triangles.push_back(pair.second);
        }
    }

    for(const Line &line : lines){
        Vector3 a = line.startLinePoint.Position;
        Vector3 b = line.endLinePoint.Position;

        bool aInside = IsPointInsideMesh(a, triangles);
        bool bInside = IsPointInsideMesh(b, triangles);

        if(!aInside && !bInside){
            result.push_back(line);
            continue;
        }

        if(aInside && bInside){
            continue;
        }

        Vector3 outside = aInside ? b : a;
        Vector3 inside  = aInside ? a : b;
        Vector3 dir     = Vector3Normalize(Vector3Subtract(inside, outside));

        float closestDist = FLT_MAX;
        Point hitPoint = {};

        for(const Triangle &tri : triangles){
            Vector3 edge1 = Vector3Subtract(tri.Vertex2.Position, tri.Vertex1.Position);
            Vector3 edge2 = Vector3Subtract(tri.Vertex3.Position, tri.Vertex1.Position);
            Vector3 normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

            float d = Vector3DotProduct(normal, tri.Vertex1.Position);

            std::pair<Point, bool> intersect = IntersectLinePlane(
                (Vector4){ normal.x, normal.y, normal.z, d },
                { outside, {} },
                { inside,  {} }
            );

            if(!intersect.second){continue;}

            Vector3 v0 = edge1;
            Vector3 v1 = edge2;
            Vector3 v2 = Vector3Subtract(intersect.first.Position, tri.Vertex1.Position);

            float d00 = Vector3DotProduct(v0, v0);
            float d01 = Vector3DotProduct(v0, v1);
            float d11 = Vector3DotProduct(v1, v1);
            float d20 = Vector3DotProduct(v2, v0);
            float d21 = Vector3DotProduct(v2, v1);

            float denom = d00 * d11 - d01 * d01;
            if(denom == 0){continue;}

            float v = (d11 * d20 - d01 * d21) / denom;
            float w = (d00 * d21 - d01 * d20) / denom;
            float u = 1.0f - v - w;

            if(u >= 0.0f && v >= 0.0f && w >= 0.0f){
                float dist = Vector3LengthSqr(Vector3Subtract(intersect.first.Position, outside));
                if(dist < closestDist){
                    closestDist = dist;
                    hitPoint = intersect.first;
                }
            }
        }

        if(closestDist < FLT_MAX){
            Line trimmed = line;
            if(aInside){
                trimmed.startLinePoint = hitPoint;
                trimmed.endLinePoint   = line.endLinePoint;
            }else{
                trimmed.startLinePoint = line.startLinePoint;
                trimmed.endLinePoint   = hitPoint;
            }
            result.push_back(trimmed);
        }
    }

    return result;
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