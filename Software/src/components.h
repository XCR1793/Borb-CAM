#ifndef COMPONENTS
#define COMPONENTS
#include <vector>
#include <raylib.h>
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>

struct comp_prop{
    int id;
    int type = 0;
    float width = 10;
    float height = 10;
    float size = 10;
    float xpos = 0;
    float ypos = 0;
    float rounding = 0.0f;
    Color colour = ORANGE;
    Color colour_alt = PINK;
    int state;
    double timer;
    Rectangle rectangle;
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
    bool add_component(comp_prop component_item, int id);

    /** Function to remove a component to an array of components */
    bool remove_component(comp_prop component_id);

    /** Function to modify a component in an array of components */
    bool modify_component(comp_prop component_item);
    bool modify_component(comp_prop component_item, int id);

    /** Function to run a component from an array of components */
    bool run_component(comp_prop component_run);

    /** Function to run all components in an array */
    bool run_components();

    /**##########################################
     * #     Component Execution Operations     #
     * ##########################################*/

    // >>>>>>>>>>>>>>>>> Assets <<<<<<<<<<<<<<<<
    /** Common ways all components are run */
    bool common_component(comp_prop component_common);

    /** Tool to reposition component */
    comp_prop position_component(comp_prop component_input, int xpos, int ypos);

    /** Tool to resize component */
    comp_prop resize_component(comp_prop component_input, int width, int height);

    /** Tool to convert colour to hsla */
    Color hsl_colour(int h, int s, int l, int a);

    /** Tool to id component */
    comp_prop id_component(comp_prop component_input, int id);

    /** Tool to return component position for given id */
    int id_pos_component(int id);

    // >>>>>>>>>>>>>>>>> Outputs <<<<<<<<<<<<<<<<
    /** Numenical Output */
    bool numerical_output_component(int value, int id);
    bool numerical_output_component(int value, int posx, int posy, int id);
    bool numerical_output_component(int value, int posx, int posy, int size, int id);
    bool numerical_output_component(int value, int posx, int posy, int size, Color colour, int id);
    // bool numerical_output_component(float value, int id);
    // bool numerical_output_component(double value, int id);


    // >>>>>>>>>>>>>> Interactable <<<<<<<<<<<<<<
    /** Button */
    // bool button_component(int id);

    /** Toggle Buttons */

    /** Slider */

    /**##########################################
     * #    Component Abstraction Operations    #
     * ##########################################*/

    bool addRectangle(int id);
    bool addNumOut(int id);
    bool addButton(int id);
    bool addButtonToggle(int id);
    bool addSlider(int id);

    bool retButton(int id);
    bool retButtonToggle(int id);
    double retSlider(int id);

    bool remComponent(int type, int id);

    // private:
    std::vector<comp_prop> component_list;
};

#endif