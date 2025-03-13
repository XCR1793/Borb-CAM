#include "components.h"

bool component::update_properties(){
    global_properties.window_size_width = GetScreenWidth();
    global_properties.window_size_height = GetScreenHeight();
    global_properties.window_width_ratio = global_properties.window_size_width/100;
    global_properties.window_height_ratio = global_properties.window_size_height/100;
    return true;
}

bool component::add_component(comp_prop component_item){
    for (auto& comp : component_list) {
        if (comp.id == component_item.id) {
            comp = component_item;
            return true;
        }
    }
    component_list.push_back(component_item);
    return true;
}

bool component::add_component(comp_prop component_item, int id){
    component_item.id = id;
    for (auto& comp : component_list) {
        if (comp.id == component_item.id) {
            comp = component_item;
            return true;
        }
    }
    component_list.push_back(component_item);
    return true;
}

bool component::remove_component(comp_prop component_id){
    for(auto it = component_list.begin(); it != component_list.end(); ++it){
        if(component_id.id == it->id){
            component_list.erase(it);
            return true;
        }
    }
    return false;
}

bool component::modify_component(comp_prop component_item){
    for (auto& comp : component_list){
        if(comp.id == component_item.id){
            comp = component_item;
            return true;
        }
    }
    component_list.push_back(component_item);
    return true;
}

bool component::modify_component(comp_prop component_item, int id){
    component_item.id = id;
    for (auto& comp : component_list){
        if(comp.id == id){
            comp = component_item;
            return true;
        }
    }
    component_list.push_back(component_item);
    return true;
}

bool component::run_component(comp_prop component_run){
    for(const auto& comp : component_list){
        switch(comp.type){
            case 0:
                common_component(comp);
                break;
            default:
                return false;
        }
    }
    return true;
}


bool component::run_components(){
    for(comp_prop &component: component_list){
        component::run_component(component);
    }
    return true;
}

bool component::common_component(comp_prop component_common){
    component_common.rectangle.height = component_common.height;
    component_common.rectangle.width = component_common.width;
    component_common.rectangle.x = component_common.xpos;
    component_common.rectangle.y = component_common.ypos;
    DrawRectangleRounded(component_common.rectangle, component_common.rounding, 0.0f, component_common.colour);
    return true;
}

comp_prop component::position_component(comp_prop component_input, int xpos, int ypos){
    component_input.xpos = xpos;
    component_input.ypos = ypos;
    return component_input;
}

comp_prop component::id_component(comp_prop component_input, int id){
    component_input.id = id;
    return component_input;
}

int component::id_pos_component(int id){
    for(size_t i = 0; i < component_list.size(); ++i){
        if(component_list[i].id == id){
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool component::numerical_output_component(int value, int id){
    int vecPosition = id_pos_component(id);
    char buffer[20];
    sprintf(buffer, "%d", value);
    DrawText(buffer, component_list.at(vecPosition).xpos, component_list.at(vecPosition).ypos, component_list.at(vecPosition).size, component_list.at(vecPosition).colour);
    return true;
}

bool component::numerical_output_component(int value, int posx, int posy, int id){
    int vecPosition = id_pos_component(id);
    char buffer[20];
    sprintf(buffer, "%d", value);
    DrawText(buffer, posx, posy, component_list.at(vecPosition).size, component_list.at(vecPosition).colour);
    return true;
}

bool component::numerical_output_component(int value, int posx, int posy, int size, int id){
    int vecPosition = id_pos_component(id);
    char buffer[20];
    sprintf(buffer, "%d", value);
    DrawText(buffer, posx, posy, size, component_list.at(vecPosition).colour);
    return true;
}

bool component::numerical_output_component(int value, int posx, int posy, int size, Color colour, int id){
    char buffer[20];
    sprintf(buffer, "%d", value);
    DrawText(buffer, posx, posy, size, colour);
    return true;
}
