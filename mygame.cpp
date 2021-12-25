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
#include <cstring>
#include <vector>
#include "yourgame/yourgame.h"
#include "mygame_version.h"
#include "ygif_glue.h"
#include "imgui.h"

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
    const std::string g_luaScriptName = "a//main.lua";

    std::string *g_licenseStr = nullptr;
    char g_luaCode[500000];

    lua_State *g_Lua = nullptr;

    // forward declarations
    void renderImgui();
    void initLua();
    void tickLua();
    void shutdownLua();

    void init(int argc, char *argv[])
    {
        yg::log::info("project: %v (%v)", mygame::version::PROJECT_NAME, mygame::version::git_commit);
        yg::log::info("based on: %v (%v)", yg::version::PROJECT_NAME, yg::version::git_commit);

        // load license info file
        {
            std::vector<uint8_t> data;
            yg::file::readFile("a//LICENSE_desktop.txt", data);
            g_licenseStr = new std::string(data.begin(), data.end());
        }

        // load Lua code
        {
            std::vector<uint8_t> data;
            yg::file::readFile(g_luaScriptName, data);
            // todo check data.size() against g_luaCode size
            std::memcpy(g_luaCode, &data[0], data.size());
        }

        glClearColor(0.275f, 0.275f, 0.275f, 1.0f);
        glEnable(GL_DEPTH_TEST);

        initLua();
    }

    void tick()
    {
        // reinit Lua if F5 was hit
        if (yg::input::getDelta(yg::input::KEY_F5) > 0.0f)
        {
            shutdownLua();
            initLua();
        }

        // exit if ESCAPE was hit
        if (yg::input::getDelta(yg::input::KEY_ESCAPE) > 0.0f)
        {
            yg::control::exit();
        }

        // set gl for this frame (viewport from window size, and clear gl buffers)
        glViewport(0,
                   0,
                   yg::input::geti(yg::input::WINDOW_WIDTH),
                   yg::input::geti(yg::input::WINDOW_HEIGHT));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderImgui();

        tickLua();
    }

    void shutdown()
    {
        if (g_licenseStr != nullptr)
        {
            delete g_licenseStr;
        }

        shutdownLua();
    }

    void renderImgui()
    {
        static bool showLicenseWindow = false;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("License"))
                {
                    showLicenseWindow = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (showLicenseWindow)
        {
            ImGui::SetNextWindowSizeConstraints(ImVec2(yg::input::get(yg::input::WINDOW_WIDTH) * 0.5f,
                                                       yg::input::get(yg::input::WINDOW_HEIGHT) * 0.5f),
                                                ImVec2(yg::input::get(yg::input::WINDOW_WIDTH) * 0.8f,
                                                       yg::input::get(yg::input::WINDOW_HEIGHT) * 0.8f));
            ImGui::Begin("License", &showLicenseWindow, (ImGuiWindowFlags_NoCollapse));
            /* The following procedure allows displaying long wrapped text,
               whereas ImGui::TextWrapped() has a size limit and cuts the content. */
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextUnformatted(g_licenseStr->c_str());
            ImGui::PopTextWrapPos();
            ImGui::End();
        }

        ImGui::Begin(g_luaScriptName.c_str(), nullptr, (0));
        ImGui::InputTextMultiline("##source",
                                  g_luaCode,
                                  IM_ARRAYSIZE(g_luaCode),
                                  ImVec2(800, 600),
                                  ImGuiInputTextFlags_AllowTabInput);
        ImGui::End();
    }

    void initLua()
    {
        if (g_Lua == nullptr)
        {
            // initialize Lua, register C++ components
            g_Lua = luaL_newstate();
            luaL_openlibs(g_Lua);
            mygame::registerLua(g_Lua);

            // run Lua code
            {
                if (luaL_dostring(g_Lua, g_luaCode) != 0)
                {
                    yg::log::error("Lua error: %v", lua_tostring(g_Lua, -1));
                    shutdownLua();
                }
                else
                {
                    // Lua: call init()
                    luabridge::LuaRef lInit = luabridge::getGlobal(g_Lua, "init");
                    if (lInit.isFunction())
                    {
                        try
                        {
                            lInit();
                        }
                        catch (luabridge::LuaException const &e)
                        {
                            yg::log::error("initLua(): Lua exception: %v", std::string(e.what()));
                            shutdownLua();
                        }
                    }
                }
            }
        }
        else
        {
            yg::log::error("initLua(): g_Lua != nullptr");
        }
    }

    void tickLua()
    {
        if (g_Lua != nullptr)
        {
            // Lua: call tick()
            luabridge::LuaRef lTick = luabridge::getGlobal(g_Lua, "tick");
            if (lTick.isFunction())
            {
                try
                {
                    lTick();
                }
                catch (luabridge::LuaException const &e)
                {
                    yg::log::error("tickLua(): Lua exception: %v", std::string(e.what()));
                    shutdownLua();
                }
            }
        }
    }

    void shutdownLua()
    {
        if (g_Lua != nullptr)
        {
            // Lua: call shutdown()
            // the extra scope is crucial. lShutdown has to be destroyed before
            // lua_close() is called.
            {
                luabridge::LuaRef lShutdown = luabridge::getGlobal(g_Lua, "shutdown");
                if (lShutdown.isFunction())
                {
                    try
                    {
                        lShutdown();
                    }
                    catch (luabridge::LuaException const &e)
                    {
                        yg::log::error("shutdownLua(): Lua exception: %v", std::string(e.what()));
                    }
                }
            }

            lua_close(g_Lua);
            g_Lua = nullptr;
        }
        else
        {
            yg::log::error("shutdownLua(): g_Lua == nullptr");
        }
    }
} // namespace mygame
