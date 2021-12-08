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
#include <vector>
#include "yourgame/yourgame.h"
#include "mygame_version.h"
#include "ygif_glue.h"

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
    lua_State *g_Lua = nullptr;

    void initLua()
    {
        if (g_Lua != nullptr)
        {
            lua_close(g_Lua);
        }

        g_Lua = luaL_newstate();
        luaL_openlibs(g_Lua);
        mygame::registerLua(g_Lua);

        // run Lua from file
        std::vector<uint8_t> luaCode;
        if (yg::file::readFile("a//main.lua", luaCode) == 0)
        {
            std::string luaCodeStr = std::string(luaCode.begin(), luaCode.end());
            if (luaL_dostring(g_Lua, luaCodeStr.c_str()) != 0)
            {
                yg::log::error("Lua error: %v", lua_tostring(g_Lua, -1));
            }
        }

        // call init() from Lua if present
        auto lInit = luabridge::getGlobal(g_Lua, "init");
        if (lInit.isFunction())
        {
            lInit();
        }
        else
        {
            yg::log::warn("no init() found in main.lua");
        }
    }

    void init(int argc, char *argv[])
    {
        initLua();

        yg::log::info("project: %v (%v)", mygame::version::PROJECT_NAME, mygame::version::git_commit);
        yg::log::info("based on: %v (%v)", yg::version::PROJECT_NAME, yg::version::git_commit);

        glClearColor(0.275f, 0.275f, 0.275f, 1.0f);
        glEnable(GL_DEPTH_TEST);
    }

    void tick()
    {
        // reinit Lua if F5 was hit
        if (yg::input::getDelta(yg::input::KEY_F5) > 0.0f)
        {
            initLua();
        }

        // set gl for this frame (viewport from window size, and clear fl buffers)
        glViewport(0,
                   0,
                   yg::input::geti(yg::input::WINDOW_WIDTH),
                   yg::input::geti(yg::input::WINDOW_HEIGHT));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // call tick() from Lua if present
        // todo: "sometimes", getting tick() via LuaBridge causes segfault
        // if Lua was reinitialized (close() ... newState()) before.
        // not further investigated yet.
        luabridge::LuaRef lTick = luabridge::getGlobal(g_Lua, "tick");
        if (lTick.isFunction())
        {
            lTick();
        }
    }

    void shutdown()
    {
        lua_close(g_Lua);
    }

} // namespace mygame
