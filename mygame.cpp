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
#include <algorithm> // std::replace()
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
    struct TextEditor
    {
        char *buffer;
        bool *winOpened;
        enum
        {
            bufferSize = 250000
        };
        TextEditor()
        {
            buffer = new char[bufferSize];
            std::memset(buffer, 0, bufferSize);
            winOpened = new bool{true};
        }
        ~TextEditor()
        {
            delete[] buffer;
            delete winOpened;
        }
    };

    // initial Lua script name to execute.
    // if project path set: try to load p//<g_luaScriptName>
    // else: try to load a//<g_luaScriptName>.
    std::string g_luaScriptName = "main.lua";

    std::map<std::string, TextEditor> g_openedEditors;
    std::string *g_licenseStr = nullptr;
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

        // assuming argv[1] is a path to a directory: set it as project directory
        if (argc > 1)
        {
            std::string projFilePathFromArgv = argv[1];
            std::replace(projFilePathFromArgv.begin(), projFilePathFromArgv.end(), '\\', '/');
            yg::file::setProjectPath(projFilePathFromArgv);
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
        float mainMenuBarHeight = 0.0f;
        static bool showLicenseWindow = false;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit"))
                {
                    yg::control::exit();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("License"))
                {
                    showLicenseWindow = true;
                }
                ImGui::EndMenu();
            }
            mainMenuBarHeight = ImGui::GetWindowSize().y;
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

        // Explorer
        {
            // get asset files
            std::vector<std::string> assetFiles = yg::file::ls("a//*");

            // get project files
            std::vector<std::string> projectFiles;
            if (yg::file::getProjectFilePath("") != "")
            {
                projectFiles = yg::file::ls("p//*");
            }

            ImGui::SetNextWindowPos({0.0f, mainMenuBarHeight});
            ImGui::Begin("Explorer", nullptr, (0));

            // lambda for button drawing, used multiple times below (collapsing headers)
            auto drawButtons = [](const std::vector<std::string> &filenames, const std::string &filePrefix)
            {
                for (const auto &f : filenames)
                {
                    std::string file = filePrefix + f;

                    if (ImGui::Button((f + "##" + filePrefix).c_str()))
                    {
                        // open new Code Editor window
                        if (g_openedEditors.find(file) == g_openedEditors.end())
                        {
                            // read file
                            std::vector<uint8_t> data;
                            yg::file::readFile(file, data);

                            // copy file data into TextEditor buffer
                            if (data.size() <= TextEditor::bufferSize)
                            {
                                // insert new default-constructed TextEditor
                                g_openedEditors[file];

                                std::memcpy(g_openedEditors[file].buffer, &data[0], data.size());
                                yg::log::debug("Buffer filled with data: %v >= %v", TextEditor::bufferSize, data.size());
                            }
                            else
                            {
                                yg::log::warn("Buffer too small for data: %v < %v", TextEditor::bufferSize, data.size());
                            }
                        }
                    }
                }
            };

            if (ImGui::CollapsingHeader("Assets", ImGuiTreeNodeFlags_DefaultOpen))
            {
                drawButtons(assetFiles, "a//");
            }

            if (ImGui::CollapsingHeader("Project", ImGuiTreeNodeFlags_DefaultOpen))
            {
                drawButtons(projectFiles, "p//");
            }

            ImGui::End();
        }

        // Code Editor windows
        for (const auto &w : g_openedEditors)
        {
            ImGui::Begin(w.first.c_str(), w.second.winOpened, (0));
            if (ImGui::Button((std::string("save##") + w.first).c_str()))
            {
                // find potential zero ('\0') in text buffer and only write
                // content before that to file
                size_t numBytesToWrite = w.second.bufferSize;
                for (size_t i = 0; i < w.second.bufferSize; i++)
                {
                    if (w.second.buffer[i] == 0)
                    {
                        numBytesToWrite = i;
                        break;
                    }
                }
                yg::file::writeAssetFile(yg::file::getFileName(w.first), w.second.buffer, numBytesToWrite);
            }

            ImGui::InputTextMultiline((std::string("##") + w.first).c_str(),
                                      w.second.buffer,
                                      w.second.bufferSize,
                                      ImVec2(800, 600),
                                      ImGuiInputTextFlags_AllowTabInput);
            ImGui::End();
        }

        // remove closed Code Editor windows
        for (auto it = g_openedEditors.cbegin(); it != g_openedEditors.cend();)
        {
            if (!*(it->second.winOpened))
            {
                it = g_openedEditors.erase(it);
            }
            else
            {
                ++it;
            }
        }
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
                // load initial Lua script from assets, or project path, if set
                std::string luaScriptName = "a//" + g_luaScriptName;
                if (yg::file::getProjectFilePath("") != "")
                {
                    luaScriptName = "p//" + g_luaScriptName;
                }

                // load lua script
                std::vector<uint8_t> data;
                if (yg::file::readFile(luaScriptName, data) == 0)
                {
                    // add null terminator for luaL_dostring()
                    data.push_back((uint8_t)0);
                }
                else
                {
                    yg::log::error("failed to load Lua code from file %v", luaScriptName);
                }

                if (luaL_dostring(g_Lua, (char *)(&data[0])) != 0)
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
