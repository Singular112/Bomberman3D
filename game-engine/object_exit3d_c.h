#pragma once
#include "game_object_c.h"

class object_exit3d_c
 : public game_object_c
{
public:
 object_exit3d_c();

 void enable_exit();
 void disable_exit();

private:
 typedef std::unique_ptr<sr::texture2d_c> texture_ptr_t;
 std::vector<texture_ptr_t> m_texture_library;

 std::unique_ptr<mesh_base_c> m_mesh_ptr;
};
