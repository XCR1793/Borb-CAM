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
    return false;
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

comp_prop component::resize_component(comp_prop component_input, int width, int height){
    component_input.width = width;
    component_input.height = height;
    return component_input;
}

void hsl_to_rgb(float H, float S, float L, unsigned char& r, unsigned char& g, unsigned char& b) {
    float R, G, B;

    if (S == 0) {
        // Achromatic (grayscale)
        R = G = B = L;
    } else {
        auto hue_to_rgb = [](float p, float q, float t) -> float {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1.0 / 6) return p + (q - p) * 6 * t;
            if (t < 1.0 / 2) return q;
            if (t < 2.0 / 3) return p + (q - p) * (2.0 / 3 - t) * 6;
            return p;
        };

        float q = (L < 0.5f) ? (L * (1 + S)) : (L + S - L * S);
        float p = 2 * L - q;

        R = hue_to_rgb(p, q, H + 1.0f / 3.0f);
        G = hue_to_rgb(p, q, H);
        B = hue_to_rgb(p, q, H - 1.0f / 3.0f);
    }

    r = static_cast<unsigned char>(R * 255);
    g = static_cast<unsigned char>(G * 255);
    b = static_cast<unsigned char>(B * 255);
}

Color component::hsl_colour(int h, int s, int l, int a) {
    float H = static_cast<float>(h) / 360.0f;
    float S = static_cast<float>(s) / 100.0f;
    float L = static_cast<float>(l) / 100.0f;

    unsigned char r, g, b;
    hsl_to_rgb(H, S, L, r, g, b);

    return Color{r, g, b, static_cast<unsigned char>(a)};
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

// bool component::button_component(int id){
//     int pos = id_pos_component(id);
//     if(GuiButton((Rectangle){component_list.at(pos).width, component_list.at(pos).height, component_list.at(pos).xpos, component_list.at(pos).ypos}, "Click Me")){return true;};
//     return false;
// }