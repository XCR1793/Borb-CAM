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

bool path::Write_File_Last(const std::string& fileName, const std::string& extension, const std::string& content) {
    std::string filePath = fileName + "." + extension;

    if(extension == ""){filePath = fileName;}

    std::ofstream outfile(filePath, std::ios::app); // Open in append mode
    if (!outfile.is_open()) {
        TraceLog(LOG_WARNING, "Failed to open file for writing: %s", filePath.c_str());
        return false;
    }

    // Always write content as a new line at the end
    outfile << content << "\n";
    outfile.close();
    return true;
}

bool path::Default_File(const std::string& fileName, const std::string& extension){
    default_file_path = fileName + "." + extension;
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
    return (Return_LV("A", a) + " " + Return_LV("B", b) + " " + Return_LV("C", c) + " " + Return_LV("K", c));
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

 // Rapid Movement (xyz millimeters / inches) (abc degrees)
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