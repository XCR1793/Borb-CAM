#ifndef PATHING
#define PATHING

#include <raylib.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class path{
    public:

    /**##########################################
     * #            System Functions            #
     * ##########################################*/

    bool Create_File(const std::string& fileName, const std::string& extension);

    bool Write_File(const std::string& fileName, const std::string& extension, long lineNumber, const std::string& content);

    bool Write_File_Last(const std::string& fileName, const std::string& extension, const std::string& content);

    bool Default_File(const std::string& fileName, const std::string& extension);

    bool Clear_File();
    bool Clear_File(const std::string& fileName, const std::string& extension);

    /**##########################################
     * #               Gcode Tools              #
     * ##########################################*/

    std::string Return_LV(const std::string& letter, float value);

    std::string Return_Gcode_Position(float x, float y, float z);

    std::string Return_Gcode_Rotation(float a);

    std::string Return_Gcode_Rotation(float a, float b);

    std::string Return_Gcode_Rotation(float a, float b, float c);

    std::string Return_EndPos(float x, float y, float z);
    
    std::string Return_EndPos(float x, float y, float z, float a);
    
    std::string Return_EndPos(float x, float y, float z, float a, float b);
    
    std::string Return_EndPos(float x, float y, float z, float a, float b, float c);
    
    
    /**##########################################
     * #              Gcode Commands            #
     * ##########################################*/

    // Rapid Movement (xyz millimeters / inches) (abc degrees), f = feedrate
    bool G0(float f);
    bool G0(float x, float y, float z);
    bool G0(float x, float y, float z, float a);
    bool G0(float x, float y, float z, float a, float b);
    bool G0(float x, float y, float z, float a, float b, float c);

    // Normal Movement (xyz millimeters / inches) (abc degrees), f = feetrate
    bool G1(float f);
    bool G1(float x, float y, float z);
    bool G1(float x, float y, float z, float a);
    bool G1(float x, float y, float z, float a, float b);
    bool G1(float x, float y, float z, float a, float b, float c);

    // Dwell / Wait (0 = millis, 1 = seconds)
    bool G4(bool Millis_or_Seconds, long duration);

    // Set workspace to XY Plane
    bool G17();

    // Set workspace to Metric
    bool G21();

    // Set workspace to Absolute
    bool G90();

    // Set workspace to Relative
    bool G91();

    /**##########################################
     * #              Pathing Tools             #
     * ##########################################*/
    
    // Pair(Position, Rotation)
    bool Path_to_Gcode0(std::vector<std::pair<Vector3, Vector3>>& positions);
    bool Path_to_Gcode1(std::vector<std::pair<Vector3, Vector3>>& positions);

    private:
    std::string default_file_path;
    long line = 0;
};

#endif