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

namespace yg = yourgame; // convenience

namespace mygame
{
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
            .endNamespace();
    }
}
