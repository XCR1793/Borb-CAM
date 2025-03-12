#include "components.h"

bool component::update_properties(){
    global_properties.window_size_width = GetScreenWidth();
    global_properties.window_size_height = GetScreenHeight();
    global_properties.window_width_ratio = global_properties.window_size_width/100;
    global_properties.window_height_ratio = global_properties.window_size_height/100;
}

bool component::add_component(comp_prop component_item){
    component_list.push_back(component_item);
    return true;
}

bool component::remove_component(comp_prop component_id) {
    for(auto it = component_list.begin(); it != component_list.end(); ++it){
        if(component_id.id == it->id){
            component_list.erase(it);
            return true;
        }
    }
    return false;
}

bool component::run_component(comp_prop component_run){
    for(auto it = component_list.begin(); it <= component_list.end(); it++){
        switch(it->id){
            case 0:
                common_component(*it);
                break;
            default:
                return 0;
        }
    }
}

bool component::common_component(comp_prop component_common){
    Rectangle rectangle;
    rectangle.height = component_common.height;
    rectangle.width = component_common.width;
    rectangle.x = component_common.xpos;
    rectangle.y = component_common.ypos;
    DrawRectangleRounded(rectangle, component_common.rounding, 0.0f, ORANGE);
    return true;
}