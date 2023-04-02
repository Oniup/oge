#include <ogl/core/serialization.hpp>
#include <ogl/ogl.hpp>

/** list of primitives that need implementing
 * all types with ptr
 * glm::mat
 * 
*/

namespace ogl {

    OGL_SERIALIZABLE_CPP_PRIMITIVE(Int32, int32_t)
    OGL_SERIALIZABLE_CPP_PRIMITIVE(Int64, int64_t)
    OGL_SERIALIZABLE_CPP_PRIMITIVE(Uint32, uint32_t)
    OGL_SERIALIZABLE_CPP_PRIMITIVE(Uint64, uint64_t)
    OGL_SERIALIZABLE_CPP_PRIMITIVE(Float32, float)
    OGL_SERIALIZABLE_CPP_PRIMITIVE(Float64, double)
    OGL_SERIALIZABLE_CPP_PRIMITIVE(Bool, bool)
    
    struct TypeDescriptor_GlmVec2 : public TypeDescriptor { 
        TypeDescriptor_GlmVec2() : TypeDescriptor("glm::vec2", sizeof(glm::vec2)) {} 

        virtual void log(std::string& log_message, const std::byte* ptr, size_t indent) const override {
            glm::vec2& vec = *(glm::vec2*)ptr;
            log_message += "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + "]";
        }
    };

    struct TypeDescriptor_GlmVec3 : public TypeDescriptor { 
        TypeDescriptor_GlmVec3() : TypeDescriptor("glm::vec3", sizeof(glm::vec3)) {} 

        virtual void log(std::string& log_message, const std::byte* ptr, size_t indent) const override {
            glm::vec3& vec = *(glm::vec3*)ptr;
            log_message += "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + "]";
        }
    };

    struct TypeDescriptor_GlmVec4 : public TypeDescriptor { 
        TypeDescriptor_GlmVec4() : TypeDescriptor("glm::vec4", sizeof(glm::vec4)) {} 

        virtual void log(std::string& log_message, const std::byte* ptr, size_t indent) const override {
            glm::vec4& vec = *(glm::vec4*)ptr;
            log_message += "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ", " + std::to_string(vec.w) + "]";
        }
    };

    struct TypeDescriptor_GlmIvec2 : public TypeDescriptor { 
        TypeDescriptor_GlmIvec2() : TypeDescriptor("glm::ivec2", sizeof(glm::ivec2)) {} 

        virtual void log(std::string& log_message, const std::byte* ptr, size_t indent) const override {
            glm::ivec2& vec = *(glm::ivec2*)ptr;
            log_message += "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + "]";
        }
    };

    struct TypeDescriptor_GlmIvec3 : public TypeDescriptor { 
        TypeDescriptor_GlmIvec3() : TypeDescriptor("glm::ivec3", sizeof(glm::ivec3)) {} 

        virtual void log(std::string& log_message, const std::byte* ptr, size_t indent) const override {
            glm::ivec3& vec = *(glm::ivec3*)ptr;
            log_message += "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + "]";
        }
    };

    struct TypeDescriptor_GlmIvec4 : public TypeDescriptor { 
        TypeDescriptor_GlmIvec4() : TypeDescriptor("glm::ivec4", sizeof(glm::ivec4)) {} 

        virtual void log(std::string& log_message, const std::byte* ptr, size_t indent) const override {
            glm::ivec4& vec = *(glm::ivec4*)ptr;
            log_message += "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ", " + std::to_string(vec.w) + "]";
        }
    };

    struct TypeDescriptor_GlmMat4 : public TypeDescriptor {
        TypeDescriptor_GlmMat4() : TypeDescriptor("glm::mat4", sizeof(glm::mat4)) {}

        virtual void log(std::string& log_message, const std::byte* ptr, size_t indent) const override {
            glm::mat4& mat = *(glm::mat4*)ptr;
            for (int i = 0; i < 4; i++) {
                log_message += "[ ";
                for (int j = 0; j < 4; j++) {
                    log_message += std::to_string(mat[i][j]) + " ";
                }
                log_message += "]\n";
            }
        }
    };

    struct TypeDescriptor_StdString : public TypeDescriptor { 
        TypeDescriptor_StdString() : TypeDescriptor("std::string", sizeof(std::string)) {} 

        virtual void log(std::string& log_message, const std::byte* ptr, size_t indent) const override {
            log_message += std::string(name) + "{ " + *(std::string*)ptr + " }";
        }
    };

    template <> TypeDescriptor* DefaultTypeDescriptorResolver::get_primitive<glm::vec2>() { static TypeDescriptor_GlmVec2 descriptor{}; return &descriptor; }
    template <> TypeDescriptor* DefaultTypeDescriptorResolver::get_primitive<glm::vec3>() { static TypeDescriptor_GlmVec3 descriptor{}; return &descriptor; }
    template <> TypeDescriptor* DefaultTypeDescriptorResolver::get_primitive<glm::vec4>() { static TypeDescriptor_GlmVec4 descriptor{}; return &descriptor; }
    template <> TypeDescriptor* DefaultTypeDescriptorResolver::get_primitive<glm::ivec2>() { static TypeDescriptor_GlmIvec2 descriptor{}; return &descriptor; }
    template <> TypeDescriptor* DefaultTypeDescriptorResolver::get_primitive<glm::ivec3>() { static TypeDescriptor_GlmIvec3 descriptor{}; return &descriptor; }
    template <> TypeDescriptor* DefaultTypeDescriptorResolver::get_primitive<glm::ivec4>() { static TypeDescriptor_GlmIvec4 descriptor{}; return &descriptor; }
    template <> TypeDescriptor* DefaultTypeDescriptorResolver::get_primitive<glm::mat4>() { static TypeDescriptor_GlmMat4 descriptor{}; return &descriptor; }

    template <> TypeDescriptor* DefaultTypeDescriptorResolver::get_primitive<std::string>() { static TypeDescriptor_StdString descriptor{}; return &descriptor; }

    OGL_SERIALIZABLE_OBJECT_BEGIN(TransformComponent)
    OGL_SERIALIZABLE_OBJECT_MEMBER(position)
    OGL_SERIALIZABLE_OBJECT_MEMBER(scale)
    OGL_SERIALIZABLE_OBJECT_MEMBER(rotation)
    OGL_SERIALIZABLE_OBJECT_END()

    // OGL_SERIALIZABLE_OBJECT_BEGIN(CameraComponent)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(is_main)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(position)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(up)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(forward)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(clear_color)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(look_at_target)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(projection_type)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(projection_matrix)
    // OGL_SERIALIZABLE_OBJECT_MEMBER(projection_size)
    // OGL_SERIALIZABLE_OBJECT_END()

}
