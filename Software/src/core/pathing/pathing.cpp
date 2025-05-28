#include "pathing.h"

/**##########################################
 * #            System Functions            #
 * ##########################################*/

bool path::Create_File(const std::string& fileName, const std::string& extension){
    std::string filePath = fileName + "." + extension;

    if(FileExists(filePath.c_str())){TraceLog(LOG_INFO, "Overriding Existing File: %s", filePath.c_str());}

    Default_File(fileName, extension);

    return SaveFileData(filePath.c_str(), nullptr, 0);
}

bool path::Write_File(const std::string& fileName, const std::string& extension, long lineNumber, const std::string& content){
    std::string filePath = fileName + "." + extension;

    // Load existing lines
    std::vector<std::string> lines;
    std::ifstream infile(filePath);
    std::string line;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    // Extend file if necessary
    if (lineNumber >= static_cast<long>(lines.size())) {
        lines.resize(lineNumber + 1, "");
    }

    // Set the line
    lines[lineNumber] = content;

    // Write lines back to file
    std::ofstream outfile(filePath);
    if (!outfile.is_open()) {
        TraceLog(LOG_WARNING, "Failed to open file for writing: %s", filePath.c_str());
        return false;
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        outfile << lines[i];
        if (i != lines.size() - 1) {
            outfile << "\n";
        }
    }

    outfile.close();
    return true;
}

// bool path::Write_File_Last(const std::string& fileName, const std::string& extension, const std::string& content) {
//     std::string filePath = fileName + "." + extension;

//     if(extension == ""){filePath = fileName;}

//     std::ofstream outfile(filePath, std::ios::app); // Open in append mode
//     if (!outfile.is_open()) {
//         TraceLog(LOG_WARNING, "Failed to open file for writing: %s", filePath.c_str());
//         return false;
//     }

//     // Always write content as a new line at the end
//     outfile << content << "\n";
//     outfile.close();
//     return true;
// }

bool path::Write_File_Last(const std::string& fileName, const std::string& extension, const std::string& content) {
    std::string filePath = fileName + (extension.empty() ? "" : "." + extension);

    std::ofstream outfile(filePath, std::ios::app); // Open in append mode
    if (!outfile.is_open()) {
        TraceLog(LOG_WARNING, "Failed to open file for writing: %s", filePath.c_str());
        return false;
    }

    // Write with N line number
    outfile << "N" << n_line_counter++ << " " << content << "\n";
    outfile.close();
    return true;
}

void path::Reset_N(){
    n_line_counter = 1;
}

bool path::Default_File(const std::string& fileName, const std::string& extension){
    default_file_path = fileName + "." + extension;
    return true;
}

bool path::Clear_File() {
    // Open file to clear its contents
    std::ofstream outfile(default_file_path, std::ios::trunc); // `std::ios::trunc` ensures the file is cleared

    // Since we're clearing the file, there's no need to write anything back
    outfile.close();
    return true;
}

bool path::Clear_File(const std::string& fileName, const std::string& extension) {
    std::string filePath = fileName + "." + extension;

    // Open file to clear its contents
    std::ofstream outfile(filePath, std::ios::trunc); // `std::ios::trunc` ensures the file is cleared

    // Since we're clearing the file, there's no need to write anything back
    outfile.close();
    return true;
}

/**##########################################
 * #               Gcode Tools              #
 * ##########################################*/

std::string path::Return_LV(const std::string& letter, float value){
    return (letter + std::to_string(value));
}

std::string path::Return_Gcode_Position(float x, float y, float z){
    return (Return_LV("X", x) + " " + Return_LV("Y", y) + " " + Return_LV("Z", z));
}

std::string path::Return_Gcode_Rotation(float a){
    return (Return_LV("A", a));
}

std::string path::Return_Gcode_Rotation(float a, float b){
    return (Return_LV("A", a) + " " + Return_LV("B", b));
}

std::string path::Return_Gcode_Rotation(float a, float b, float c){
    return (Return_LV("A", a) + " " + Return_LV("B", b) + " " + Return_LV("C", c));
}

std::string path::Return_EndPos(float x, float y, float z){
    return (Return_Gcode_Position(x, y, z));
}

std::string path::Return_EndPos(float x, float y, float z, float a){
    return (Return_Gcode_Position(x, y, z) + " " + Return_Gcode_Rotation(a));
}

std::string path::Return_EndPos(float x, float y, float z, float a, float b){
    return (Return_Gcode_Position(x, y, z) + " " + Return_Gcode_Rotation(a, b));
}

std::string path::Return_EndPos(float x, float y, float z, float a, float b, float c){
    return (Return_Gcode_Position(x, y, z) + " " + Return_Gcode_Rotation(a, b, c));
}


/**##########################################
 * #              Gcode Commands            #
 * ##########################################*/

// Set Feedrate
bool path::Feedrate(float f){
    Write_File_Last(default_file_path, "", ("F" + std::to_string(f)));
    return true;
}

// Rapid Movement (xyz millimeters / inches) (abc degrees), f = feedrate
bool path::G0(float f){
    Write_File_Last(default_file_path, "", ("G0 " + std::to_string(f)));
    return true;
}

bool path::G0(float x, float y, float z){
    Write_File_Last(default_file_path, "", ("G0 " + Return_EndPos(x, y, z)));
    return true;
}

bool path::G0(float x, float y, float z, float a){
    Write_File_Last(default_file_path, "", ("G0 " + Return_EndPos(x, y, z, a)));
    return true;
}

bool path::G0(float x, float y, float z, float a, float b){
    Write_File_Last(default_file_path, "", ("G0 " + Return_EndPos(x, y, z, a, b)));
    return true;
}

bool path::G0(float x, float y, float z, float a, float b, float c){
    Write_File_Last(default_file_path, "", ("G0 " + Return_EndPos(x, y, z, a, b, c)));
    return true;
}

// Normal Movement (xyz millimeters / inches) (abc degrees)
bool path::G1(float f){
    Write_File_Last(default_file_path, "", ("G1 " + std::to_string(f)));
    return true;
}

bool path::G1(float x, float y, float z){
    Write_File_Last(default_file_path, "", ("G1 " + Return_EndPos(x, y, z)));
    return true;
}

bool path::G1(float x, float y, float z, float a){
    Write_File_Last(default_file_path, "", ("G1 " + Return_EndPos(x, y, z, a)));
    return true;
}

bool path::G1(float x, float y, float z, float a, float b){
    Write_File_Last(default_file_path, "", ("G1 " + Return_EndPos(x, y, z, a, b)));
    return true;
}

bool path::G1(float x, float y, float z, float a, float b, float c){
    Write_File_Last(default_file_path, "", ("G1 " + Return_EndPos(x, y, z, a, b, c)));
    return true;
}

// Dwell / Wait (0 = millis, 1 = seconds)
bool path::G4(bool Millis_or_Seconds, long duration){
    if(Millis_or_Seconds){
        Write_File_Last(default_file_path, "", ("G4 P" + std::to_string(duration)));
        return true;
    }
    
    Write_File_Last(default_file_path, "", ("G4 S" + std::to_string(duration)));
   
    return true;
}

// Set workspace to XY Plane
bool path::G17(){
    Write_File_Last(default_file_path, "", ("G17"));
    return true;
}

// Set workspace to Metric
bool path::G21(){
    Write_File_Last(default_file_path, "", ("G21"));
    return true;
}

// Set workspace to Absolute
bool path::G90(){
    Write_File_Last(default_file_path, "", ("G90"));
    return true;
}

// Set workspace to Relative
bool path::G91(){
    Write_File_Last(default_file_path, "", ("G91"));
    return true;
}

/**##########################################
 * #              Pathing Tools             #
 * ##########################################*/

bool path::Path_to_Gcode0(std::vector<std::pair<Vector3, Vector3>>& positions){
    for(std::size_t it = 0; it < positions.size(); it++){
        Vector3 position = positions.at(it).first;
        Vector3 rotation = positions.at(it).second;
        G0(position.x, position.y, position.z, rotation.x, rotation.y, rotation.z);
    }
    return true;
}

bool path::Path_to_Gcode1(std::vector<std::pair<Vector3, Vector3>>& positions){
    for(std::size_t it = 0; it < positions.size(); it++){
        Vector3 position = positions.at(it).first;
        Vector3 rotation = positions.at(it).second;
        G1(position.x, position.y, position.z, rotation.x, rotation.y, rotation.z);
    }
    return true;
}

bool path::Gcode_to_Path(const std::string& fileName, const std::string& extension, std::vector<std::pair<Vector3, Vector3>>& outPositions) {
    std::string filePath = fileName + (extension.empty() ? "" : "." + extension);
    std::ifstream infile(filePath);
    if (!infile.is_open()) {
        TraceLog(LOG_WARNING, "Failed to open G-code file: %s", filePath.c_str());
        return false;
    }

    Vector3 currentPos = {0, 0, 0};
    Vector3 currentRot = {0, 0, 0};
    std::string line;

    while (std::getline(infile, line)) {
        if (line.empty()) continue;

        // Only care about motion commands
        if (line.find("G0") == std::string::npos && line.find("G1") == std::string::npos)
            continue;

        float x = currentPos.x, y = currentPos.y, z = currentPos.z;
        float a = currentRot.x, b = currentRot.y, c = currentRot.z;

        std::istringstream ss(line);
        std::string token;

        while (ss >> token) {
            if (token.length() < 2) continue;

            char prefix = token[0];
            std::string valueStr = token.substr(1);

            // Skip non-axis tokens like N, G, M
            if (prefix != 'X' && prefix != 'Y' && prefix != 'Z' &&
                prefix != 'A' && prefix != 'B' && prefix != 'C') {
                continue;
            }

            try {
                float value = std::stof(valueStr);
                switch (prefix) {
                    case 'X': x = value; break;
                    case 'Y': y = value; break;
                    case 'Z': z = value; break;
                    case 'A': a = value; break;
                    case 'B': b = value; break;
                    case 'C': c = value; break;
                }
            } catch (...) {
                // ignore malformed values
                continue;
            }
        }

        currentPos = { x, y, z };
        currentRot = { a, b, c };
        outPositions.emplace_back(currentPos, currentRot);
    }

    infile.close();
    return true;
}
