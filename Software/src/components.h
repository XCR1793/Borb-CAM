#ifndef COMPONENTS
#define COMPONENTS
#include <vector>
#include <raylib.h>
#include <raygui.h>
#include <stdio.h>
#include <stdlib.h>

struct comp_prop{
    int id;
    int type;
    float width;
    float height;
    float xpos;
    float ypos;
    float rounding;
    // Color colour;
    // Color colour_alt;
    int state;
    double timer;
};

struct global_prop{
    float window_size_width;
    float window_size_height;
    float window_width_ratio;
    float window_height_ratio;
};

struct mouse_prop{
    float mousex;
    float mousey;
    bool right_button;
    bool left_button;
};

class component{
    public:
    global_prop global_properties;

    /** Updates global properties */
    bool update_properties();

    /** Function to add a component to an array of components */
    bool add_component(comp_prop component_item);

    /** Function to remove a component to an array of components */
    bool remove_component(comp_prop component_id);

    /** Function to run a component to an array of components */
    bool run_component(comp_prop component_run);    

    /** Common ways all components are run */
    bool common_component(comp_prop component_common);

    private:
    std::vector<comp_prop> component_list;
};

#endif