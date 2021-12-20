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
#include "ygif_trafo.h"
#include "ygif_camera.h"

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
#include "LuaBridge_glmQuat.h"

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
    const std::map<std::string, yg::input::Source> str2input = {
        {"KEY_UNKNOWN", yg::input::KEY_UNKNOWN},
        {"KEY_SPACE", yg::input::KEY_SPACE},
        {"KEY_APOSTROPHE", yg::input::KEY_APOSTROPHE},
        {"KEY_COMMA", yg::input::KEY_COMMA},
        {"KEY_MINUS", yg::input::KEY_MINUS},
        {"KEY_PERIOD", yg::input::KEY_PERIOD},
        {"KEY_SLASH", yg::input::KEY_SLASH},
        {"KEY_0", yg::input::KEY_0},
        {"KEY_1", yg::input::KEY_1},
        {"KEY_2", yg::input::KEY_2},
        {"KEY_3", yg::input::KEY_3},
        {"KEY_4", yg::input::KEY_4},
        {"KEY_5", yg::input::KEY_5},
        {"KEY_6", yg::input::KEY_6},
        {"KEY_7", yg::input::KEY_7},
        {"KEY_8", yg::input::KEY_8},
        {"KEY_9", yg::input::KEY_9},
        {"KEY_SEMICOLON", yg::input::KEY_SEMICOLON},
        {"KEY_EQUAL", yg::input::KEY_EQUAL},
        {"KEY_A", yg::input::KEY_A},
        {"KEY_B", yg::input::KEY_B},
        {"KEY_C", yg::input::KEY_C},
        {"KEY_D", yg::input::KEY_D},
        {"KEY_E", yg::input::KEY_E},
        {"KEY_F", yg::input::KEY_F},
        {"KEY_G", yg::input::KEY_G},
        {"KEY_H", yg::input::KEY_H},
        {"KEY_I", yg::input::KEY_I},
        {"KEY_J", yg::input::KEY_J},
        {"KEY_K", yg::input::KEY_K},
        {"KEY_L", yg::input::KEY_L},
        {"KEY_M", yg::input::KEY_M},
        {"KEY_N", yg::input::KEY_N},
        {"KEY_O", yg::input::KEY_O},
        {"KEY_P", yg::input::KEY_P},
        {"KEY_Q", yg::input::KEY_Q},
        {"KEY_R", yg::input::KEY_R},
        {"KEY_S", yg::input::KEY_S},
        {"KEY_T", yg::input::KEY_T},
        {"KEY_U", yg::input::KEY_U},
        {"KEY_V", yg::input::KEY_V},
        {"KEY_W", yg::input::KEY_W},
        {"KEY_X", yg::input::KEY_X},
        {"KEY_Y", yg::input::KEY_Y},
        {"KEY_Z", yg::input::KEY_Z},
        {"KEY_LEFT_BRACKET", yg::input::KEY_LEFT_BRACKET},
        {"KEY_BACKSLASH", yg::input::KEY_BACKSLASH},
        {"KEY_RIGHT_BRACKET", yg::input::KEY_RIGHT_BRACKET},
        {"KEY_GRAVE_ACCENT", yg::input::KEY_GRAVE_ACCENT},
        {"KEY_WORLD_1", yg::input::KEY_WORLD_1},
        {"KEY_WORLD_2", yg::input::KEY_WORLD_2},
        {"KEY_ESCAPE", yg::input::KEY_ESCAPE},
        {"KEY_ENTER", yg::input::KEY_ENTER},
        {"KEY_TAB", yg::input::KEY_TAB},
        {"KEY_BACKSPACE", yg::input::KEY_BACKSPACE},
        {"KEY_INSERT", yg::input::KEY_INSERT},
        {"KEY_DELETE", yg::input::KEY_DELETE},
        {"KEY_RIGHT", yg::input::KEY_RIGHT},
        {"KEY_LEFT", yg::input::KEY_LEFT},
        {"KEY_DOWN", yg::input::KEY_DOWN},
        {"KEY_UP", yg::input::KEY_UP},
        {"KEY_PAGE_UP", yg::input::KEY_PAGE_UP},
        {"KEY_PAGE_DOWN", yg::input::KEY_PAGE_DOWN},
        {"KEY_HOME", yg::input::KEY_HOME},
        {"KEY_END", yg::input::KEY_END},
        {"KEY_CAPS_LOCK", yg::input::KEY_CAPS_LOCK},
        {"KEY_SCROLL_LOCK", yg::input::KEY_SCROLL_LOCK},
        {"KEY_NUM_LOCK", yg::input::KEY_NUM_LOCK},
        {"KEY_PRINT_SCREEN", yg::input::KEY_PRINT_SCREEN},
        {"KEY_PAUSE", yg::input::KEY_PAUSE},
        {"KEY_F1", yg::input::KEY_F1},
        {"KEY_F2", yg::input::KEY_F2},
        {"KEY_F3", yg::input::KEY_F3},
        {"KEY_F4", yg::input::KEY_F4},
        {"KEY_F5", yg::input::KEY_F5},
        {"KEY_F6", yg::input::KEY_F6},
        {"KEY_F7", yg::input::KEY_F7},
        {"KEY_F8", yg::input::KEY_F8},
        {"KEY_F9", yg::input::KEY_F9},
        {"KEY_F10", yg::input::KEY_F10},
        {"KEY_F11", yg::input::KEY_F11},
        {"KEY_F12", yg::input::KEY_F12},
        {"KEY_F13", yg::input::KEY_F13},
        {"KEY_F14", yg::input::KEY_F14},
        {"KEY_F15", yg::input::KEY_F15},
        {"KEY_F16", yg::input::KEY_F16},
        {"KEY_F17", yg::input::KEY_F17},
        {"KEY_F18", yg::input::KEY_F18},
        {"KEY_F19", yg::input::KEY_F19},
        {"KEY_F20", yg::input::KEY_F20},
        {"KEY_F21", yg::input::KEY_F21},
        {"KEY_F22", yg::input::KEY_F22},
        {"KEY_F23", yg::input::KEY_F23},
        {"KEY_F24", yg::input::KEY_F24},
        {"KEY_F25", yg::input::KEY_F25},
        {"KEY_KP_0", yg::input::KEY_KP_0},
        {"KEY_KP_1", yg::input::KEY_KP_1},
        {"KEY_KP_2", yg::input::KEY_KP_2},
        {"KEY_KP_3", yg::input::KEY_KP_3},
        {"KEY_KP_4", yg::input::KEY_KP_4},
        {"KEY_KP_5", yg::input::KEY_KP_5},
        {"KEY_KP_6", yg::input::KEY_KP_6},
        {"KEY_KP_7", yg::input::KEY_KP_7},
        {"KEY_KP_8", yg::input::KEY_KP_8},
        {"KEY_KP_9", yg::input::KEY_KP_9},
        {"KEY_KP_DECIMAL", yg::input::KEY_KP_DECIMAL},
        {"KEY_KP_DIVIDE", yg::input::KEY_KP_DIVIDE},
        {"KEY_KP_MULTIPLY", yg::input::KEY_KP_MULTIPLY},
        {"KEY_KP_SUBTRACT", yg::input::KEY_KP_SUBTRACT},
        {"KEY_KP_ADD", yg::input::KEY_KP_ADD},
        {"KEY_KP_ENTER", yg::input::KEY_KP_ENTER},
        {"KEY_KP_EQUAL", yg::input::KEY_KP_EQUAL},
        {"KEY_LEFT_SHIFT", yg::input::KEY_LEFT_SHIFT},
        {"KEY_LEFT_CONTROL", yg::input::KEY_LEFT_CONTROL},
        {"KEY_LEFT_ALT", yg::input::KEY_LEFT_ALT},
        {"KEY_LEFT_SUPER", yg::input::KEY_LEFT_SUPER},
        {"KEY_RIGHT_SHIFT", yg::input::KEY_RIGHT_SHIFT},
        {"KEY_RIGHT_CONTROL", yg::input::KEY_RIGHT_CONTROL},
        {"KEY_RIGHT_ALT", yg::input::KEY_RIGHT_ALT},
        {"KEY_RIGHT_SUPER", yg::input::KEY_RIGHT_SUPER},
        {"KEY_MENU", yg::input::KEY_MENU},
        {"MOUSE_X", yg::input::MOUSE_X},
        {"MOUSE_Y", yg::input::MOUSE_Y},
        {"MOUSE_BUTTON_1", yg::input::MOUSE_BUTTON_1},
        {"MOUSE_BUTTON_2", yg::input::MOUSE_BUTTON_2},
        {"MOUSE_BUTTON_3", yg::input::MOUSE_BUTTON_3},
        {"MOUSE_BUTTON_4", yg::input::MOUSE_BUTTON_4},
        {"MOUSE_BUTTON_5", yg::input::MOUSE_BUTTON_5},
        {"MOUSE_BUTTON_6", yg::input::MOUSE_BUTTON_6},
        {"MOUSE_BUTTON_7", yg::input::MOUSE_BUTTON_7},
        {"MOUSE_BUTTON_8", yg::input::MOUSE_BUTTON_8},
        {"SCROLL_OFFSET_X", yg::input::SCROLL_OFFSET_X},
        {"SCROLL_OFFSET_Y", yg::input::SCROLL_OFFSET_Y},
        {"TOUCH_0_DOWN", yg::input::TOUCH_0_DOWN},
        {"TOUCH_0_X", yg::input::TOUCH_0_X},
        {"TOUCH_0_Y", yg::input::TOUCH_0_Y},
        {"TOUCH_1_DOWN", yg::input::TOUCH_1_DOWN},
        {"TOUCH_1_X", yg::input::TOUCH_1_X},
        {"TOUCH_1_Y", yg::input::TOUCH_1_Y},
        {"TOUCH_2_DOWN", yg::input::TOUCH_2_DOWN},
        {"TOUCH_2_X", yg::input::TOUCH_2_X},
        {"TOUCH_2_Y", yg::input::TOUCH_2_Y},
        {"TOUCH_3_DOWN", yg::input::TOUCH_3_DOWN},
        {"TOUCH_3_X", yg::input::TOUCH_3_X},
        {"TOUCH_3_Y", yg::input::TOUCH_3_Y},
        {"TOUCH_4_DOWN", yg::input::TOUCH_4_DOWN},
        {"TOUCH_4_X", yg::input::TOUCH_4_X},
        {"TOUCH_4_Y", yg::input::TOUCH_4_Y},
        {"TOUCH_5_DOWN", yg::input::TOUCH_5_DOWN},
        {"TOUCH_5_X", yg::input::TOUCH_5_X},
        {"TOUCH_5_Y", yg::input::TOUCH_5_Y},
        {"TOUCH_6_DOWN", yg::input::TOUCH_6_DOWN},
        {"TOUCH_6_X", yg::input::TOUCH_6_X},
        {"TOUCH_6_Y", yg::input::TOUCH_6_Y},
        {"TOUCH_7_DOWN", yg::input::TOUCH_7_DOWN},
        {"TOUCH_7_X", yg::input::TOUCH_7_X},
        {"TOUCH_7_Y", yg::input::TOUCH_7_Y},
        {"TOUCH_8_DOWN", yg::input::TOUCH_8_DOWN},
        {"TOUCH_8_X", yg::input::TOUCH_8_X},
        {"TOUCH_8_Y", yg::input::TOUCH_8_Y},
        {"TOUCH_9_DOWN", yg::input::TOUCH_9_DOWN},
        {"TOUCH_9_X", yg::input::TOUCH_9_X},
        {"TOUCH_9_Y", yg::input::TOUCH_9_Y},
        {"GAMEPAD_0_CONNECTED", yg::input::GAMEPAD_0_CONNECTED},
        {"GAMEPAD_0_BUTTON_A", yg::input::GAMEPAD_0_BUTTON_A},
        {"GAMEPAD_0_BUTTON_B", yg::input::GAMEPAD_0_BUTTON_B},
        {"GAMEPAD_0_BUTTON_X", yg::input::GAMEPAD_0_BUTTON_X},
        {"GAMEPAD_0_BUTTON_Y", yg::input::GAMEPAD_0_BUTTON_Y},
        {"GAMEPAD_0_BUTTON_LEFT_BUMPER", yg::input::GAMEPAD_0_BUTTON_LEFT_BUMPER},
        {"GAMEPAD_0_BUTTON_RIGHT_BUMPER", yg::input::GAMEPAD_0_BUTTON_RIGHT_BUMPER},
        {"GAMEPAD_0_BUTTON_BACK", yg::input::GAMEPAD_0_BUTTON_BACK},
        {"GAMEPAD_0_BUTTON_START", yg::input::GAMEPAD_0_BUTTON_START},
        {"GAMEPAD_0_BUTTON_GUIDE", yg::input::GAMEPAD_0_BUTTON_GUIDE},
        {"GAMEPAD_0_BUTTON_LEFT_THUMB", yg::input::GAMEPAD_0_BUTTON_LEFT_THUMB},
        {"GAMEPAD_0_BUTTON_RIGHT_THUMB", yg::input::GAMEPAD_0_BUTTON_RIGHT_THUMB},
        {"GAMEPAD_0_BUTTON_DPAD_UP", yg::input::GAMEPAD_0_BUTTON_DPAD_UP},
        {"GAMEPAD_0_BUTTON_DPAD_RIGHT", yg::input::GAMEPAD_0_BUTTON_DPAD_RIGHT},
        {"GAMEPAD_0_BUTTON_DPAD_DOWN", yg::input::GAMEPAD_0_BUTTON_DPAD_DOWN},
        {"GAMEPAD_0_BUTTON_DPAD_LEFT", yg::input::GAMEPAD_0_BUTTON_DPAD_LEFT},
        {"GAMEPAD_0_AXIS_LEFT_X", yg::input::GAMEPAD_0_AXIS_LEFT_X},
        {"GAMEPAD_0_AXIS_LEFT_Y", yg::input::GAMEPAD_0_AXIS_LEFT_Y},
        {"GAMEPAD_0_AXIS_RIGHT_X", yg::input::GAMEPAD_0_AXIS_RIGHT_X},
        {"GAMEPAD_0_AXIS_RIGHT_Y", yg::input::GAMEPAD_0_AXIS_RIGHT_Y},
        {"GAMEPAD_0_AXIS_LEFT_TRIGGER", yg::input::GAMEPAD_0_AXIS_LEFT_TRIGGER},
        {"GAMEPAD_0_AXIS_RIGHT_TRIGGER", yg::input::GAMEPAD_0_AXIS_RIGHT_TRIGGER},
        {"GAMEPAD_1_CONNECTED", yg::input::GAMEPAD_1_CONNECTED},
        {"GAMEPAD_1_BUTTON_A", yg::input::GAMEPAD_1_BUTTON_A},
        {"GAMEPAD_1_BUTTON_B", yg::input::GAMEPAD_1_BUTTON_B},
        {"GAMEPAD_1_BUTTON_X", yg::input::GAMEPAD_1_BUTTON_X},
        {"GAMEPAD_1_BUTTON_Y", yg::input::GAMEPAD_1_BUTTON_Y},
        {"GAMEPAD_1_BUTTON_LEFT_BUMPER", yg::input::GAMEPAD_1_BUTTON_LEFT_BUMPER},
        {"GAMEPAD_1_BUTTON_RIGHT_BUMPER", yg::input::GAMEPAD_1_BUTTON_RIGHT_BUMPER},
        {"GAMEPAD_1_BUTTON_BACK", yg::input::GAMEPAD_1_BUTTON_BACK},
        {"GAMEPAD_1_BUTTON_START", yg::input::GAMEPAD_1_BUTTON_START},
        {"GAMEPAD_1_BUTTON_GUIDE", yg::input::GAMEPAD_1_BUTTON_GUIDE},
        {"GAMEPAD_1_BUTTON_LEFT_THUMB", yg::input::GAMEPAD_1_BUTTON_LEFT_THUMB},
        {"GAMEPAD_1_BUTTON_RIGHT_THUMB", yg::input::GAMEPAD_1_BUTTON_RIGHT_THUMB},
        {"GAMEPAD_1_BUTTON_DPAD_UP", yg::input::GAMEPAD_1_BUTTON_DPAD_UP},
        {"GAMEPAD_1_BUTTON_DPAD_RIGHT", yg::input::GAMEPAD_1_BUTTON_DPAD_RIGHT},
        {"GAMEPAD_1_BUTTON_DPAD_DOWN", yg::input::GAMEPAD_1_BUTTON_DPAD_DOWN},
        {"GAMEPAD_1_BUTTON_DPAD_LEFT", yg::input::GAMEPAD_1_BUTTON_DPAD_LEFT},
        {"GAMEPAD_1_AXIS_LEFT_X", yg::input::GAMEPAD_1_AXIS_LEFT_X},
        {"GAMEPAD_1_AXIS_LEFT_Y", yg::input::GAMEPAD_1_AXIS_LEFT_Y},
        {"GAMEPAD_1_AXIS_RIGHT_X", yg::input::GAMEPAD_1_AXIS_RIGHT_X},
        {"GAMEPAD_1_AXIS_RIGHT_Y", yg::input::GAMEPAD_1_AXIS_RIGHT_Y},
        {"GAMEPAD_1_AXIS_LEFT_TRIGGER", yg::input::GAMEPAD_1_AXIS_LEFT_TRIGGER},
        {"GAMEPAD_1_AXIS_RIGHT_TRIGGER", yg::input::GAMEPAD_1_AXIS_RIGHT_TRIGGER},
        {"GAMEPAD_2_CONNECTED", yg::input::GAMEPAD_2_CONNECTED},
        {"GAMEPAD_2_BUTTON_A", yg::input::GAMEPAD_2_BUTTON_A},
        {"GAMEPAD_2_BUTTON_B", yg::input::GAMEPAD_2_BUTTON_B},
        {"GAMEPAD_2_BUTTON_X", yg::input::GAMEPAD_2_BUTTON_X},
        {"GAMEPAD_2_BUTTON_Y", yg::input::GAMEPAD_2_BUTTON_Y},
        {"GAMEPAD_2_BUTTON_LEFT_BUMPER", yg::input::GAMEPAD_2_BUTTON_LEFT_BUMPER},
        {"GAMEPAD_2_BUTTON_RIGHT_BUMPER", yg::input::GAMEPAD_2_BUTTON_RIGHT_BUMPER},
        {"GAMEPAD_2_BUTTON_BACK", yg::input::GAMEPAD_2_BUTTON_BACK},
        {"GAMEPAD_2_BUTTON_START", yg::input::GAMEPAD_2_BUTTON_START},
        {"GAMEPAD_2_BUTTON_GUIDE", yg::input::GAMEPAD_2_BUTTON_GUIDE},
        {"GAMEPAD_2_BUTTON_LEFT_THUMB", yg::input::GAMEPAD_2_BUTTON_LEFT_THUMB},
        {"GAMEPAD_2_BUTTON_RIGHT_THUMB", yg::input::GAMEPAD_2_BUTTON_RIGHT_THUMB},
        {"GAMEPAD_2_BUTTON_DPAD_UP", yg::input::GAMEPAD_2_BUTTON_DPAD_UP},
        {"GAMEPAD_2_BUTTON_DPAD_RIGHT", yg::input::GAMEPAD_2_BUTTON_DPAD_RIGHT},
        {"GAMEPAD_2_BUTTON_DPAD_DOWN", yg::input::GAMEPAD_2_BUTTON_DPAD_DOWN},
        {"GAMEPAD_2_BUTTON_DPAD_LEFT", yg::input::GAMEPAD_2_BUTTON_DPAD_LEFT},
        {"GAMEPAD_2_AXIS_LEFT_X", yg::input::GAMEPAD_2_AXIS_LEFT_X},
        {"GAMEPAD_2_AXIS_LEFT_Y", yg::input::GAMEPAD_2_AXIS_LEFT_Y},
        {"GAMEPAD_2_AXIS_RIGHT_X", yg::input::GAMEPAD_2_AXIS_RIGHT_X},
        {"GAMEPAD_2_AXIS_RIGHT_Y", yg::input::GAMEPAD_2_AXIS_RIGHT_Y},
        {"GAMEPAD_2_AXIS_LEFT_TRIGGER", yg::input::GAMEPAD_2_AXIS_LEFT_TRIGGER},
        {"GAMEPAD_2_AXIS_RIGHT_TRIGGER", yg::input::GAMEPAD_2_AXIS_RIGHT_TRIGGER},
        {"GAMEPAD_3_CONNECTED", yg::input::GAMEPAD_3_CONNECTED},
        {"GAMEPAD_3_BUTTON_A", yg::input::GAMEPAD_3_BUTTON_A},
        {"GAMEPAD_3_BUTTON_B", yg::input::GAMEPAD_3_BUTTON_B},
        {"GAMEPAD_3_BUTTON_X", yg::input::GAMEPAD_3_BUTTON_X},
        {"GAMEPAD_3_BUTTON_Y", yg::input::GAMEPAD_3_BUTTON_Y},
        {"GAMEPAD_3_BUTTON_LEFT_BUMPER", yg::input::GAMEPAD_3_BUTTON_LEFT_BUMPER},
        {"GAMEPAD_3_BUTTON_RIGHT_BUMPER", yg::input::GAMEPAD_3_BUTTON_RIGHT_BUMPER},
        {"GAMEPAD_3_BUTTON_BACK", yg::input::GAMEPAD_3_BUTTON_BACK},
        {"GAMEPAD_3_BUTTON_START", yg::input::GAMEPAD_3_BUTTON_START},
        {"GAMEPAD_3_BUTTON_GUIDE", yg::input::GAMEPAD_3_BUTTON_GUIDE},
        {"GAMEPAD_3_BUTTON_LEFT_THUMB", yg::input::GAMEPAD_3_BUTTON_LEFT_THUMB},
        {"GAMEPAD_3_BUTTON_RIGHT_THUMB", yg::input::GAMEPAD_3_BUTTON_RIGHT_THUMB},
        {"GAMEPAD_3_BUTTON_DPAD_UP", yg::input::GAMEPAD_3_BUTTON_DPAD_UP},
        {"GAMEPAD_3_BUTTON_DPAD_RIGHT", yg::input::GAMEPAD_3_BUTTON_DPAD_RIGHT},
        {"GAMEPAD_3_BUTTON_DPAD_DOWN", yg::input::GAMEPAD_3_BUTTON_DPAD_DOWN},
        {"GAMEPAD_3_BUTTON_DPAD_LEFT", yg::input::GAMEPAD_3_BUTTON_DPAD_LEFT},
        {"GAMEPAD_3_AXIS_LEFT_X", yg::input::GAMEPAD_3_AXIS_LEFT_X},
        {"GAMEPAD_3_AXIS_LEFT_Y", yg::input::GAMEPAD_3_AXIS_LEFT_Y},
        {"GAMEPAD_3_AXIS_RIGHT_X", yg::input::GAMEPAD_3_AXIS_RIGHT_X},
        {"GAMEPAD_3_AXIS_RIGHT_Y", yg::input::GAMEPAD_3_AXIS_RIGHT_Y},
        {"GAMEPAD_3_AXIS_LEFT_TRIGGER", yg::input::GAMEPAD_3_AXIS_LEFT_TRIGGER},
        {"GAMEPAD_3_AXIS_RIGHT_TRIGGER", yg::input::GAMEPAD_3_AXIS_RIGHT_TRIGGER},
        {"WINDOW_WIDTH", yg::input::WINDOW_WIDTH},
        {"WINDOW_HEIGHT", yg::input::WINDOW_HEIGHT},
        {"WINDOW_ASPECT_RATIO", yg::input::WINDOW_ASPECT_RATIO},
        {"WINDOW_FULLSCREEN", yg::input::WINDOW_FULLSCREEN},
        {"VSYNC_ON", yg::input::VSYNC_ON},
        {"MOUSE_CATCHED", yg::input::MOUSE_CATCHED}};

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
        shader->useProgram(light, camera);
        yg::gl::DrawConfig cfg;
        cfg.camera = camera;
        if (trafo != nullptr)
        {
            cfg.modelMat = trafo->mat();
        }
        cfg.shader = shader;
        yg::gl::drawGeo(geo, cfg);
    }

    void registerLua(lua_State *L)
    {
        luabridge::getGlobalNamespace(L)
            .beginNamespace("yg")
            // namespace audio ...
            .beginNamespace("audio")
            .addFunction("init", yg::audio::init)
            .addFunction("shutdown", yg::audio::shutdown)
            .addFunction("isInitialized", yg::audio::isInitialized)
            .addFunction("storeFile", yg::audio::storeFile)
            .addFunction("play", yg::audio::play)
            .addFunction("stop", yg::audio::stop)
            .addFunction("pause", yg::audio::pause)
            .addFunction("setChannelGains", yg::audio::setChannelGains)
            .endNamespace()
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
            // Lua class yg.math.Trafo (C++ class YgifTrafo) is derived from yg::math::Trafo
            .beginClass<yg::math::Trafo>("TrafoBase")
            .addConstructor<void (*)()>()
            .addFunction("setRotation", &yg::math::Trafo::setRotation)
            .addFunction("setTranslation", &yg::math::Trafo::setTranslation)
            .addFunction("pointTo", &yg::math::Trafo::pointTo)
            .addFunction("lookAt", &yg::math::Trafo::lookAt)
            .addFunction("lerp", &yg::math::Trafo::lerp)
            .addFunction("setIdentity", &yg::math::Trafo::setIdentity)
            .addFunction("getAxisLocal", &yg::math::Trafo::getAxisLocal)
            .addFunction("getAxisGlobal", &yg::math::Trafo::getAxisGlobal)
            .addFunction("getEye", &yg::math::Trafo::getEye)
            .addFunction("getRotation", &yg::math::Trafo::getRotation)
            .addFunction("getScale", &yg::math::Trafo::getScale)
            .endClass()
            .deriveClass<YgifTrafo, yg::math::Trafo>("Trafo")
            .addConstructor<void (*)()>()
            .addFunction("rotateLocal", &YgifTrafo::rotateLocal)
            .addFunction("rotateGlobal", &YgifTrafo::rotateGlobal)
            .addFunction("translateLocal", &YgifTrafo::translateLocal)
            .addFunction("translateGlobal", &YgifTrafo::translateGlobal)
            .addFunction("setScaleLocal", &YgifTrafo::setScaleLocal)
            .endClass()
            // Lua class yg.math.Camera (C++ class YgifCamera) is derived from yg::math::Camera
            .beginClass<yg::math::Camera>("CameraBase")
            .addConstructor<void (*)()>()
            .addFunction("setPerspective", &yg::math::Camera::setPerspective)
            .addFunction("setOrthographic", &yg::math::Camera::setOrthographic)
            .addFunction("setFovy", &yg::math::Camera::setFovy)
            .addFunction("setHeight", &yg::math::Camera::setHeight)
            .addFunction("setAspect", &yg::math::Camera::setAspect)
            .addFunction("setZNear", &yg::math::Camera::setZNear)
            .addFunction("setZFar", &yg::math::Camera::setZFar)
            .endClass()
            .deriveClass<YgifCamera, yg::math::Camera>("Camera")
            .addConstructor<void (*)()>()
            .addFunction("trafo", &YgifCamera::trafo)
            .addFunction("castRay", &YgifCamera::castRay)
            .endClass()
            .endNamespace()
            // namespace time ...
            .beginNamespace("time")
            .addFunction("getClockPeriod", yg::time::getClockPeriod)
            .addFunction("getDelta", yg::time::getDelta)
            .addFunction("getTime", yg::time::getTime)
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
