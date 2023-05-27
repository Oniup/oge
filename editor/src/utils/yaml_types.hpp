#ifndef __OGE_UTILS_YAML_TYPES_HPP__
#define __OGE_UTILS_YAML_TYPES_HPP__

#include <imgui/imgui.h>
#include <yaml/yaml.hpp>

template<>
struct yaml::Convert<ImVec4> {
    std::string value_to_str(const ImVec4& vec) {
        return std::string(
            "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " +
            std::to_string(vec.z) + "," + std::to_string(vec.w) + "]"
        );
    }

    ImVec4 value(const yaml::Node& node) {
        ImVec4 result = {};

        char number[50];
        std::size_t j = 0;
        std::size_t index = 0;

        const std::string& value = node.get_value();
        for (std::size_t i = 0; i < value.size(); i++) {
            if (value[i] == '[' || value[i] == ']' || value[i] == '\r' || value[i] == ' ') {
                continue;
            } else if (value[i] == ',') {
                number[j] = '\0';
                switch (index) {
                case 0:
                    result.x = std::stof(number);
                    break;
                case 1:
                    result.y = std::stof(number);
                    break;
                case 2:
                    result.z = std::stof(number);
                    break;
                }
                j = 0;
                index++;
            } else {
                number[j] = value[i];
                j++;
            }
        }

        result.w = std::stof(number);
        return result;
    }
};

template<>
struct yaml::Convert<ImVec2> {
    std::string value_to_str(const ImVec2& vec) {
        return std::string("[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + "]");
    }

    ImVec2 value(const yaml::Node& node) {
        ImVec2 result = {};

        char number[50];
        std::size_t j = 0;
        std::size_t index = 0;

        const std::string& value = node.get_value();
        for (std::size_t i = 0; i < value.size(); i++) {
            if (value[i] == '[' || value[i] == ']' || value[i] == '\r' || value[i] == ' ') {
                continue;
            } else if (value[i] == ',') {
                number[j] = '\0';
                switch (index) {
                case 0:
                    result.x = std::stof(number);
                    break;
                }
                j = 0;
                index++;
            } else {
                number[j] = value[i];
                j++;
            }
        }

        result.y = std::stof(number);
        return result;
    }
};

#endif
