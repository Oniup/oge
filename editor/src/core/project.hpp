#ifndef __OGE_CORE_PROJECT_HPP__
#define __OGE_CORE_PROJECT_HPP__

#include <ogl/ogl.hpp>

namespace oge {

class Project : public ogl::ApplicationLayer {
  public:
    inline static Project* get() { return m_Instance; }
    static void create_new_popup();

  public:
    Project();
    virtual ~Project() override = default;

    inline bool opened() const { return m_name.size() > 0; }
    inline bool is_3d_based() const { return m_3d_based; }
    inline bool& is_3d_based() { return m_3d_based; }
    inline const std::string& get_name() const { return m_name; }
    inline const std::string& get_root_path() const { return m_root_path; }
    inline const std::string& get_project_filename() const { return m_project_filename; }

    bool create(const std::string& name, const std::string& project_root_path);
    bool load(const std::string& project_filename);
    void serialize(const std::string& path);

  private:
    static Project* m_Instance;

  private:
    void _deserialize_scene(ogl::Scene* scene);
    void _serialize_scene(ogl::Scene* scene);

  private:
    std::string m_name = {};
    std::string m_root_path = {};
    std::string m_project_filename = {};
    bool m_3d_based = true;
};

} // namespace oge

#endif
