#include "mycube.hxx"
#include "drawable_frame.hxx"

#include <glm/fwd.hpp>

using glm::mat4;
using glm::vec3;
using std::shared_ptr;
using namespace glwrapper;

MyCube::MyCube(vec3 tangent, vec3 bitangent, vec3 pos) :
    ebo(GL_ELEMENT_ARRAY_BUFFER), pos_(pos), tangent_(tangent), bitangent_(bitangent) {
    // update loc
    tangent_   = glm::normalize(tangent_);
    auto vec_z = glm::normalize(glm::cross(tangent_, bitangent_));
    bitangent_ = glm::normalize(glm::cross(vec_z, tangent_));
    // set render info
    vertices = {
        0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1,
        0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
    };
    indices = {
        0, 1, 3, 0, 3, 2, 4, 7, 5, 4, 6, 7, 0, 5, 1, 0, 4, 5,
        2, 3, 7, 2, 7, 6, 0, 4, 6, 0, 6, 2, 1, 7, 5, 1, 3, 7,
    };

    diffuse.from_image("tex_diffuse.jpg");
    specular.from_image("tex_specular.jpg");

    MY_CHECK_FAIL
    vao.bind();
    MY_CHECK_FAIL
    vbo.bind();
    vbo.SetBufferData(vertices.size() * sizeof(float), vertices.data());
    vbo.SetAttribPointer(0, 3, GL_FLOAT);
    vbo.SetAttribPointer(1, 2, GL_FLOAT, false, 0, (void *)(24 * sizeof(float)));
    ebo.bind();
    ebo.SetBufferData(indices.size() * sizeof(indices), indices.data());
    vao.unbind();
}

MyWorld::MyWorld() {
    MY_CHECK_FAIL
    prog = std::make_shared<ShaderProgram>("shader3.vs", "shader3.fs", "", "", "shader3.gs", "");
    MY_CHECK_FAIL
    auto cube = std::make_shared<MyCube>();
    MY_CHECK_FAIL
    // auto cube=std::make_shared<MyCube>(vec3(0,0,0),vec3(1,1,0),vec3(0,1,1));
    cubes.push_back(cube);
    type = mf::WORLD_VIEW;
}
bool MyWorld::draw(mf::DrawableFrame &fbo) {
    fbo.clear_color(cur_rect);
    glEnable(GL_DEPTH_TEST);
    MY_CHECK_FAIL
    for (auto cube : cubes) {
        cube->vao.bind();
        MY_CHECK_FAIL
        prog->use();
        MY_CHECK_FAIL
        prog->set_value("world2view", camera.world2view());
        prog->set_value("view2clip", camera.view2clip());
        // prog->set_value("world2clip", mat4(0.1));

        MY_CHECK_FAIL
        prog->set_value("model2world", cube->model2world());
        prog->set_value("ambient", vec3(0.1));
        prog->set_value("viewPos", camera.viewpoint_);
        prog->set_value("light.pos", vec3(0, 10, 0));
        prog->set_value("light.diffuse", vec3(1));
        prog->set_value("light.specular", vec3(1));
        cube->diffuse.activate_sampler(prog, "material.diffuse", 0);
        cube->specular.activate_sampler(prog, "material.specular", 1);
        prog->set_value("material.shininess", 2.f);
        MY_CHECK_FAIL

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        MY_CHECK_FAIL
    }
    glDisable(GL_DEPTH_TEST);
    return false;
}
