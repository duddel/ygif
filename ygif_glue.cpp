/*
Copyright (c) 2019-2021 Alexander Scholz

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include "yourgame/yourgame.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/Array.h"

// LuaBridge Stacks for glm
#include "LuaBridge_glmVec3.h"

namespace yg = yourgame; // convenience

namespace mygame
{
    // log ...
    void log_debug(std::string s)
    {
        yg::log::debug("%v", s);
    }

    void log_info(std::string s)
    {
        yg::log::info("%v", s);
    }

    void log_warn(std::string s)
    {
        yg::log::warn("%v", s);
    }

    void log_error(std::string s)
    {
        yg::log::error("%v", s);
    }

    // input ...
    // todo: move this mapping to yourgamelib
    const std::map<std::string, yg::input::Source> str2input = {
        {"KEY_ESCAPE", yg::input::KEY_ESCAPE},
        {"KEY_RIGHT", yg::input::KEY_RIGHT},
        {"KEY_LEFT", yg::input::KEY_LEFT},
        {"KEY_DOWN", yg::input::KEY_DOWN},
        {"KEY_UP", yg::input::KEY_UP}};

    float input_get(std::string source)
    {
        auto i = str2input.find(source);
        return (i == str2input.end()) ? 0.0f : yg::input::get(i->second);
    }

    int input_geti(std::string source)
    {
        auto i = str2input.find(source);
        return (i == str2input.end()) ? 0.0f : yg::input::geti(i->second);
    }

    float input_getDelta(std::string source)
    {
        auto i = str2input.find(source);
        return (i == str2input.end()) ? 0.0f : yg::input::getDelta(i->second);
    }

    // gl ...
    yg::gl::Shader *loadVertFragShader(std::string vertFilename, std::string fragFilename)
    {
        return yg::gl::loadShader({{GL_VERTEX_SHADER, vertFilename},
                                   {GL_FRAGMENT_SHADER, fragFilename}});
    }

    void gl_draw(yg::gl::Geometry *geo,
                 yg::gl::Lightsource *light,
                 yg::gl::Shader *shader,
                 yg::math::Camera *camera,
                 yg::math::Trafo *trafo)
    {
        camera->setPerspective(40.0f, yg::input::get(yg::input::WINDOW_ASPECT_RATIO), 1.0f, 100.0f);
        camera->setAspect(yg::input::get(yg::input::WINDOW_ASPECT_RATIO));

        shader->useProgram(light, camera);
        yg::gl::DrawConfig cfg;
        cfg.camera = camera;
        cfg.modelMat = trafo->mat();
        cfg.shader = shader;
        yg::gl::drawGeo(geo, cfg);
    }

    void registerLua(lua_State *L)
    {
        luabridge::getGlobalNamespace(L)
            .beginNamespace("yg")
            // namespace log ...
            .beginNamespace("log")
            .addFunction("debug", log_debug)
            .addFunction("info", log_info)
            .addFunction("warn", log_warn)
            .addFunction("error", log_error)
            .endNamespace()
            // namespace control ...
            .beginNamespace("control")
            .addFunction("exit", yg::control::exit)
            .addFunction("sendCmdToEnv", yg::control::sendCmdToEnv)
            .addFunction("enableFullscreen", yg::control::enableFullscreen)
            .addFunction("enableVSync", yg::control::enableVSync)
            .addFunction("catchMouse", yg::control::catchMouse)
            .endNamespace()
            // namespace input ...
            .beginNamespace("input")
            .addFunction("get", input_get)
            .addFunction("geti", input_geti)
            .addFunction("getDelta", input_getDelta)
            .endNamespace()
            // namespace math ...
            .beginNamespace("math")
            .beginClass<yg::math::Trafo>("Trafo")
            .addConstructor<void (*)()>()
            .addFunction("setTranslation", &yg::math::Trafo::setTranslation)
            .addFunction("lookAt", &yg::math::Trafo::lookAt)
            .addFunction("setIdentity", &yg::math::Trafo::setIdentity)
            .endClass()
            .beginClass<yg::math::Camera>("Camera")
            .addConstructor<void (*)()>()
            .addFunction("trafo", &yg::math::Camera::trafo)
            .addFunction("setFovy", &yg::math::Camera::setFovy)
            .endClass()
            .endNamespace()
            // namespace gl ...
            .beginNamespace("gl")
            .addFunction("draw", gl_draw)
            .addFunction("loadGeometry", yg::gl::loadGeometry)
            .addFunction("loadVertFragShader", loadVertFragShader)
            .beginClass<yg::gl::Geometry>("Geometry")
            .endClass()
            .beginClass<yg::gl::Lightsource>("Lightsource")
            .addConstructor<void (*)()>()
            .addFunction("setAmbient", &yg::gl::Lightsource::setAmbient)
            .addFunction("setDiffuse", &yg::gl::Lightsource::setDiffuse)
            .addFunction("setSpecular", &yg::gl::Lightsource::setSpecular)
            .addFunction("setPosition", &yg::gl::Lightsource::setPosition)
            .endClass()
            .beginClass<yg::gl::Shader>("Shader")
            .endClass()
            .endNamespace()
            // end of namespace yg
            .endNamespace();
    }
}
