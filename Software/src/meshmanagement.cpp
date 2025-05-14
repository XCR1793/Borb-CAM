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
 * #          Mesh Helper Functions         #
 * ##########################################*/

float mesh::pointToPointDistance(Vector3 StartPoint, Vector3 EndPoint){
    float dx = EndPoint.x - StartPoint.x;
    float dy = EndPoint.y - StartPoint.y;
    float dz = EndPoint.z - StartPoint.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

float mesh::pointToPointDistance(Point StartPoint, Point EndPoint){
    float dx = EndPoint.Position.x - StartPoint.Position.x;
    float dy = EndPoint.Position.y - StartPoint.Position.y;
    float dz = EndPoint.Position.z - StartPoint.Position.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
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

bool mesh::CheckCollisionPointBox(Vector3 point, BoundingBox box){
    const float epsilon = 1e-6f;

    return (point.x >= box.min.x - epsilon && point.x <= box.max.x + epsilon)
         && (point.y >= box.min.y - epsilon && point.y <= box.max.y + epsilon)
         && (point.z >= box.min.z - epsilon && point.z <= box.max.z + epsilon);
}

bool mesh::Line_Touching(const Line& a, const Line& b){
    Point aStart = a.startLinePoint;
    Point aEnd   = a.endLinePoint;
    Point bStart = b.startLinePoint;
    Point bEnd   = b.endLinePoint;

    return (
        pointToPointDistance(aEnd, bStart) < epsilon ||  // a end → b start
        pointToPointDistance(aEnd, bEnd)   < epsilon ||  // a end → b end
        pointToPointDistance(aStart, bStart) < epsilon || // a start → b start
        pointToPointDistance(aStart, bEnd)   < epsilon    // a start → b end
    );
}

Line mesh::Flip_Line(const Line& line){
    Line flipped = line;
    flipped.startLinePoint = line.endLinePoint;
    flipped.endLinePoint   = line.startLinePoint;
    return flipped;
}

// std::pair<std::pair<Point, Point>, bool> mesh::Intersect_Line_Box(Point startPoint, Point endPoint, BoundingBox box){
//     Vector3 p0   = startPoint.Position;
//     Vector3 dir  = Vector3Subtract(endPoint.Position, p0);
//     float tmin   = 0.0f;
//     float tmax   = 1.0f;
//     Vector3 enterNormal = {};
//     Vector3 exitNormal  = {};

//     for(int i = 0; i < 3; i++){
//         float start = ((float*)&p0)[i];
//         float d     = ((float*)&dir)[i];
//         float minB  = ((float*)&box.min)[i];
//         float maxB  = ((float*)&box.max)[i];

//         if(fabsf(d) < 1e-6f){
//             if(start < minB || start > maxB){
//                 return {{{}, {}}, false};
//             }
//         }else{
//             float t0 = (minB - start) / d;
//             float t1 = (maxB - start) / d;

//             Vector3 thisEnter = {};
//             Vector3 thisExit  = {};
//             ((float*)&thisEnter)[i] = (d > 0) ? -1.0f : 1.0f;
//             ((float*)&thisExit)[i]  = (d > 0) ?  1.0f : -1.0f;

//             if(t0 > t1){
//                 std::swap(t0, t1);
//                 std::swap(thisEnter, thisExit);
//             }

//             if(t0 > tmin){
//                 tmin = t0;
//                 enterNormal = thisEnter;
//             }
//             if(t1 < tmax){
//                 tmax = t1;
//                 exitNormal = thisExit;
//             }

//             if(tmin > tmax){
//                 return {{{}, {}}, false};
//             }
//         }
//     }

//     if(tmin <= 1.0f && tmax >= 0.0f){
//         Vector3 entryPos = Vector3Add(p0, Vector3Scale(dir, tmin));
//         Vector3 exitPos  = Vector3Add(p0, Vector3Scale(dir, tmax));

//         Point entry = { .Position = entryPos, .Normal = enterNormal };
//         Point exit  = { .Position = exitPos,  .Normal = exitNormal };

//         return {{ entry, exit }, true};
//     }

//     return {{{}, {}}, false};
// }

std::pair<std::pair<Point, Point>, bool> mesh::Intersect_Line_Box(Point startPoint, Point endPoint, BoundingBox box){
    Vector3 p0   = startPoint.Position;
    Vector3 dir  = Vector3Subtract(endPoint.Position, p0);
    float tmin   = 0.0f;
    float tmax   = 1.0f;

    float enterT = 0.0f, exitT = 1.0f;

    for(int i = 0; i < 3; i++){
        float start = ((float*)&p0)[i];
        float d     = ((float*)&dir)[i];
        float minB  = ((float*)&box.min)[i];
        float maxB  = ((float*)&box.max)[i];

        if(fabsf(d) < 1e-6f){
            if(start < minB || start > maxB){
                return {{{}, {}}, false};
            }
        } else {
            float t0 = (minB - start) / d;
            float t1 = (maxB - start) / d;

            if(t0 > t1) std::swap(t0, t1);
            if(t0 > tmin) {
                tmin = t0;
                enterT = t0;
            }
            if(t1 < tmax) {
                tmax = t1;
                exitT = t1;
            }

            if(tmin > tmax){
                return {{{}, {}}, false};
            }
        }
    }

    if(tmin <= 1.0f && tmax >= 0.0f){
        Vector3 entryPos = Vector3Add(p0, Vector3Scale(dir, enterT));
        Vector3 exitPos  = Vector3Add(p0, Vector3Scale(dir, exitT));

        Vector3 entryNorm = Vector3Normalize(Vector3Add(
            Vector3Scale(startPoint.Normal, 1.0f - enterT),
            Vector3Scale(endPoint.Normal, enterT)
        ));
        Vector3 exitNorm = Vector3Normalize(Vector3Add(
            Vector3Scale(startPoint.Normal, 1.0f - exitT),
            Vector3Scale(endPoint.Normal, exitT)
        ));

        Point entry = { .Position = entryPos, .Normal = entryNorm };
        Point exit  = { .Position = exitPos,  .Normal = exitNorm };

        return {{ entry, exit }, true};
    }

    return {{{}, {}}, false};
}


Line mesh::Clip_Line_To_Box(const Line& line, BoundingBox box, bool in_out){
    bool startInside = CheckCollisionPointBox(PointToVec3(line.startLinePoint), box);
    bool endInside   = CheckCollisionPointBox(PointToVec3(line.endLinePoint),   box);

    Point newStart = line.startLinePoint;
    Point newEnd   = line.endLinePoint;

    if(startInside != endInside){
        auto result = Intersect_Line_Box(line.startLinePoint, line.endLinePoint, box);

        if(result.second){
            Point entry = result.first.first;
            Point exit  = result.first.second;

            if(in_out){
                if(!startInside) newStart = entry;
                if(!endInside)   newEnd   = exit;
            } else {
                if(startInside) newStart = exit;
                if(endInside)   newEnd   = entry;
            }
        }
    }

    Line result = {
        .startLinePoint = newStart,
        .endLinePoint   = newEnd,
        .type           = line.type,
        .meshNo         = line.meshNo,
        .islandNo       = line.islandNo
    };

    // 🔁 Ensure consistent direction
    float d1 = pointToPointDistance(line.startLinePoint.Position, result.startLinePoint.Position);
    float d2 = pointToPointDistance(line.startLinePoint.Position, result.endLinePoint.Position);
    if (d2 < d1){
        result = Flip_Line(result);
    }

    return result;
}

std::vector<Line> mesh::Orient_Line_Group(const std::vector<Line>& lines){
    if(lines.empty()) return {};

    std::vector<Line> ordered;
    std::vector<bool> used(lines.size(), false);
    size_t total = lines.size();

    ordered.push_back(lines[0]);
    used[0] = true;

    Point forwardEnd = ordered.back().endLinePoint;
    while(ordered.size() < total){
        bool extended = false;
        for(size_t i = 0; i < total; ++i){
            if(used[i]) continue;

            const Line& candidate = lines[i];
            if(pointToPointDistance(candidate.startLinePoint.Position, forwardEnd.Position) < epsilon){
                ordered.push_back(candidate);
                forwardEnd = candidate.endLinePoint;
                used[i] = true;
                extended = true;
                break;
            }
            if(pointToPointDistance(candidate.endLinePoint.Position, forwardEnd.Position) < epsilon){
                Line flipped = Flip_Line(candidate);
                ordered.push_back(flipped);
                forwardEnd = flipped.endLinePoint;
                used[i] = true;
                extended = true;
                break;
            }
        }
        if(!extended) break;
    }

    Point backwardStart = ordered.front().startLinePoint;
    while(ordered.size() < total){
        bool extended = false;
        for(size_t i = 0; i < total; ++i){
            if(used[i]) continue;

            const Line& candidate = lines[i];
            if(pointToPointDistance(candidate.endLinePoint.Position, backwardStart.Position) < epsilon){
                ordered.insert(ordered.begin(), candidate);
                backwardStart = candidate.startLinePoint;
                used[i] = true;
                extended = true;
                break;
            }
            if(pointToPointDistance(candidate.startLinePoint.Position, backwardStart.Position) < epsilon){
                Line flipped = Flip_Line(candidate);
                ordered.insert(ordered.begin(), flipped);
                backwardStart = flipped.startLinePoint;
                used[i] = true;
                extended = true;
                break;
            }
        }
        if(!extended) break;
    }

    return ordered;
}

std::vector<Line> mesh::Chain_Walker(const std::vector<Line>& unordered){
    std::vector<Line> result;
    std::vector<bool> used(unordered.size(), false);
    size_t total = unordered.size();

    if(total == 0) return result;

    for(size_t i = 0; i < total; ++i){
        if(used[i]) continue;

        std::vector<Line> chain;
        chain.push_back(unordered[i]);
        used[i] = true;

        Point forwardEnd = chain.back().endLinePoint;
        while(true){
            bool extended = false;
            for(size_t j = 0; j < total; ++j){
                if(used[j]) continue;

                const Line& candidate = unordered[j];
                if(pointToPointDistance(candidate.startLinePoint.Position, forwardEnd.Position) < 0.001f){
                    chain.push_back(candidate);
                    forwardEnd = candidate.endLinePoint;
                    used[j] = true;
                    extended = true;
                    break;
                }
                if(pointToPointDistance(candidate.endLinePoint.Position, forwardEnd.Position) < 0.001f){
                    Line flipped = Flip_Line(candidate);
                    chain.push_back(flipped);
                    forwardEnd = flipped.endLinePoint;
                    used[j] = true;
                    extended = true;
                    break;
                }
            }
            if(!extended) break;
        }

        Point backwardStart = chain.front().startLinePoint;
        while(true){
            bool extended = false;
            for(size_t j = 0; j < total; ++j){
                if(used[j]) continue;

                const Line& candidate = unordered[j];
                if(pointToPointDistance(candidate.endLinePoint.Position, backwardStart.Position) < 0.001f){
                    chain.insert(chain.begin(), candidate);
                    backwardStart = candidate.startLinePoint;
                    used[j] = true;
                    extended = true;
                    break;
                }
                if(pointToPointDistance(candidate.startLinePoint.Position, backwardStart.Position) < 0.001f){
                    Line flipped = Flip_Line(candidate);
                    chain.insert(chain.begin(), flipped);
                    backwardStart = flipped.startLinePoint;
                    used[j] = true;
                    extended = true;
                    break;
                }
            }
            if(!extended) break;
        }

        result.insert(result.end(), chain.begin(), chain.end());
    }

    return result;
}

std::vector<Line> mesh::Flatten_Culled_Lines(BoundingBox box, const std::vector<Line>& lines, bool in_out) {
    std::vector<Lines> groups = Cull_Lines_ByBox(box, lines, in_out);
    std::vector<Line> flat;
    for (const Lines& group : groups) {
        flat.insert(flat.end(), group.lineList.begin(), group.lineList.end());
    }
    return flat;
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

std::vector<Lines> mesh::Group_Continuous(const std::vector<Line>& lines){
    if(lines.empty()) return {};

    std::vector<Lines> groups;
    std::vector<bool> visited(lines.size(), false);

    for(size_t i = 0; i < lines.size(); ++i){
        if(visited[i]) continue;

        std::vector<Line> group;
        std::vector<size_t> stack;
        stack.push_back(i);
        visited[i] = true;

        while(!stack.empty()){
            size_t current = stack.back();
            stack.pop_back();
            group.push_back(lines[current]);

            for(size_t j = 0; j < lines.size(); ++j){
                if(!visited[j] && Line_Touching(lines[current], lines[j])){
                    stack.push_back(j);
                    visited[j] = true;
                }
            }
        }

        if(!group.empty()){
            Lines continuousGroup;
            continuousGroup.lineList = group;

            // Orient and determine meta info
            continuousGroup.lineList = Chain_Walker(continuousGroup.lineList);
            continuousGroup.startPosition = continuousGroup.lineList.front().startLinePoint.Position;
            continuousGroup.endPosition   = continuousGroup.lineList.back().endLinePoint.Position;
            continuousGroup.distance = pointToPointDistance(
                continuousGroup.lineList.front().startLinePoint,
                continuousGroup.lineList.back().endLinePoint
            );

            groups.push_back(continuousGroup);
        }
    }

    return groups;
}


std::vector<Lines> mesh::Cull_Lines_ByBox(BoundingBox box, const std::vector<Line>& lines, bool in_out){
    std::vector<Line> filtered_Line_List;

    // Filter lines fully inside or fully outside the box
    for(const auto& line : lines){
        bool startInside = CheckCollisionPointBox(PointToVec3(line.startLinePoint), box);
        bool endInside   = CheckCollisionPointBox(PointToVec3(line.endLinePoint), box);

        if((startInside && endInside) == in_out){
            filtered_Line_List.push_back(line);
        }
    }

    // Group continuous lines
    std::vector<Lines> grouped = Group_Continuous(filtered_Line_List);

    // Final output
    std::vector<Lines> clippedGroups;

    for(auto& group : grouped){
        if(group.lineList.empty()) continue;

        Lines clippedGroup;
        clippedGroup.lineList = Orient_Line_Group(group.lineList);

        // Clip the first and last line of the group (preserving direction)
        for (Line& line : group.lineList) {             // MAYBE
            line = Clip_Line_To_Box(line, box, in_out);             // MAYBE
        }               // MAYBE
        clippedGroup.lineList = Orient_Line_Group(group.lineList);              // MAYBE


        // Recalculate start, end and distance
        clippedGroup.startPosition = clippedGroup.lineList.front().startLinePoint.Position;
        clippedGroup.endPosition   = clippedGroup.lineList.back().endLinePoint.Position;
        clippedGroup.distance      = pointToPointDistance(
            clippedGroup.lineList.front().startLinePoint,
            clippedGroup.lineList.back().endLinePoint
        );

        clippedGroups.push_back(clippedGroup);
    }

    return clippedGroups;
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