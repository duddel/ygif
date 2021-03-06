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
#include <set>
#include "yourgame/yourgame.h"
#include "nlohmann/json.hpp"
#include "mygame_version.h"
#include "ygif_glue.h"
#include "imgui.h"
#include "TextEditor.h" // this is ImGuiColorTextEdit
#include "imgui_memory_editor.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "LuaBridge/LuaBridge.h"

using json = nlohmann::json;
namespace yg = yourgame; // convenience

namespace mygame
{
    struct FileTextEditor
    {
        bool *winOpened;
        TextEditor editor;
        FileTextEditor()
        {
            winOpened = new bool{true};
        }
        ~FileTextEditor()
        {
            delete winOpened;
        }
    };

    struct FileHexEditor
    {
        bool *winOpened;
        MemoryEditor editor;
        std::vector<uint8_t> data;
        FileHexEditor()
        {
            winOpened = new bool{true};
        }
        ~FileHexEditor()
        {
            delete winOpened;
        }
    };

    const std::set<std::string> g_excludeFiles = {
        "./",
        "../",
        "LICENSE_desktop.txt",
        "LICENSE_android.txt",
        "LICENSE_web.txt"};

    // initial Lua script name to execute.
    // if project path set: try to load p//<g_luaScriptName>
    // else: try to load a//<g_luaScriptName>.
    std::string g_luaScriptName = "main.lua";

    // initial flavor to load.
    // if project path set: try to load p//<g_flavorName>
    // else: try to load a//<g_flavorName>.
    std::string g_flavorName = "main_flavor.json";

    std::map<std::string, FileTextEditor> g_openedEditors;
    std::map<std::string, FileHexEditor> g_openedHexEditors;
    std::string *g_licenseStr = nullptr;
    lua_State *g_Lua = nullptr;
    json g_flavor;

    bool g_renderImgui = true;

    // forward declarations
    void renderImgui();
    void initLua();
    void tickLua();
    void shutdownLua();
    void loadFlavor();

    void init(int argc, char *argv[])
    {
        yg::log::info("project: %v (%v)", mygame::version::PROJECT_NAME, mygame::version::git_commit);
        yg::log::info("based on: %v (%v)", yg::version::PROJECT_NAME, yg::version::git_commit);

        // load license info file
        {
            std::vector<uint8_t> data;
#if defined(YOURGAME_PLATFORM_DESKTOP)
            yg::file::readFile("a//LICENSE_desktop.txt", data);
#elif defined(YOURGAME_PLATFORM_ANDROID)
            yg::file::readFile("a//LICENSE_android.txt", data);
#elif defined(YOURGAME_PLATFORM_WEB)
            yg::file::readFile("a//LICENSE_web.txt", data);
#endif
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

        loadFlavor();
        initLua();
    }

    void tick()
    {
        // reinit Lua if F5 was hit
        if (yg::input::getDelta(yg::input::KEY_F5) > 0.0f)
        {
            shutdownLua();
            loadFlavor();
            initLua();
        }

        // toggle GUI
        if (yg::input::getDelta(yg::input::KEY_TAB) > 0.0f)
        {
            g_renderImgui = !g_renderImgui;
        }

        // fullscreen
        if (yg::input::getDelta(yg::input::KEY_F11) > 0.0f)
        {
            yg::control::enableFullscreen(!yg::input::geti(yg::input::WINDOW_FULLSCREEN));
        }

        // set gl for this frame (viewport from window size, and clear gl buffers)
        glViewport(0,
                   0,
                   yg::input::geti(yg::input::WINDOW_WIDTH),
                   yg::input::geti(yg::input::WINDOW_HEIGHT));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (g_renderImgui)
        {
            renderImgui();
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

        // remove closed Code Hex Editor windows
        for (auto it = g_openedHexEditors.cbegin(); it != g_openedHexEditors.cend();)
        {
            if (!*(it->second.winOpened))
            {
                it = g_openedHexEditors.erase(it);
            }
            else
            {
                ++it;
            }
        }

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
        float sideBarHeight = 0.0f;

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
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Render GUI", "TAB", &g_renderImgui);
                if (ImGui::MenuItem("Fullscreen", "F11", yg::input::geti(yg::input::WINDOW_FULLSCREEN)))
                {
                    yg::control::enableFullscreen(!yg::input::geti(yg::input::WINDOW_FULLSCREEN));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Run"))
            {
                if (ImGui::MenuItem("Reload and Start", "F5"))
                {
                    shutdownLua();
                    loadFlavor();
                    initLua();
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

        sideBarHeight = yg::input::get(yg::input::WINDOW_HEIGHT) - mainMenuBarHeight;

        if (showLicenseWindow)
        {
            ImGui::Begin("License", &showLicenseWindow, (ImGuiWindowFlags_NoCollapse));
            ImGui::SetWindowSize(ImVec2(yg::input::get(yg::input::WINDOW_WIDTH) * 0.5f,
                                        yg::input::get(yg::input::WINDOW_HEIGHT) * 0.5f),
                                 ImGuiCond_FirstUseEver);
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
            ImGui::SetNextWindowSizeConstraints({200.0f, sideBarHeight}, {500.0f, sideBarHeight});
            ImGui::Begin("Explorer", nullptr, (0));

            // lambda for button drawing, used multiple times below (collapsing headers)
            auto drawButtons = [](const std::vector<std::string> &filenames, const std::string &filePrefix)
            {
                for (const auto &f : filenames)
                {
                    if (g_excludeFiles.find(f) != g_excludeFiles.end())
                    {
                        continue;
                    }

                    std::string file = filePrefix + f;

                    if (ImGui::Button((std::string("txt##") + f + filePrefix).c_str()))
                    {
                        // open new Code Editor window
                        if (g_openedEditors.find(file) == g_openedEditors.end())
                        {
                            // read file
                            std::vector<uint8_t> data;
                            yg::file::readFile(file, data);

                            // insert new default-constructed FileTextEditor
                            g_openedEditors[file];

                            // set editor language
                            if (yg::file::getFileExtension(file).compare("lua") == 0)
                            {
                                g_openedEditors[file].editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
                            }
                            else if ((yg::file::getFileExtension(file).compare("vert") == 0) ||
                                     (yg::file::getFileExtension(file).compare("frag") == 0))
                            {
                                g_openedEditors[file].editor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
                            }

                            // fill editor with initial content
                            std::string dataStr = std::string(data.begin(), data.end());
                            g_openedEditors[file].editor.SetText(dataStr);
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button((std::string("bin##") + f + filePrefix).c_str()))
                    {
                        // insert new default-constructed FileHexEditor
                        g_openedHexEditors[file];

                        // read file
                        yg::file::readFile(file, g_openedHexEditors[file].data);
                    }
                    ImGui::SameLine();
                    ImGui::Text("%s", f.c_str());
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

        // Flavor Editor
        {
            ImGui::SetNextWindowSizeConstraints({200.0f, sideBarHeight}, {500.0f, sideBarHeight});
            ImGui::Begin("Flavor", nullptr, (0));
            ImGui::SetWindowPos({yg::input::get(yg::input::WINDOW_WIDTH) - ImGui::GetWindowSize().x, mainMenuBarHeight});

            for (auto &el : g_flavor.items())
            {
                auto &v = el.value();

                // todo: simplify entire json logic by validating with schema
                if (!v.contains("type") || !v.contains("data"))
                {
                    continue;
                }

                std::string type = v["type"].get<std::string>();
                std::string name = el.key();

                ImGui::PushID(name.c_str());
                if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    // todo: assuming data matches type. prefer validating with schema
                    if (type.compare("number") == 0)
                    {
                        float data = v["data"].get<float>();

                        std::string unit = v.contains("unit") ? v["unit"].get<std::string>() : "";
                        ImGui::DragFloat(unit.c_str(), &data, 0.001f);

                        v["data"] = data;
                    }
                    else if (type.compare("vec3") == 0)
                    {
                        float data[3];
                        data[0] = v["data"][0].get<float>();
                        data[1] = v["data"][1].get<float>();
                        data[2] = v["data"][2].get<float>();

                        std::string usage = v.contains("usage") ? v["usage"].get<std::string>() : "";
                        if (usage.compare("color") == 0)
                        {
                            ImGui::ColorPicker3("", data);
                        }
                        else
                        {
                            ImGui::DragFloat("X", &(data[0]), 0.001f);
                            ImGui::DragFloat("Y", &(data[1]), 0.001f);
                            ImGui::DragFloat("Z", &(data[2]), 0.001f);
                        }

                        v["data"][0] = data[0];
                        v["data"][1] = data[1];
                        v["data"][2] = data[2];
                    }
                }
                ImGui::PopID();
            }

            ImGui::End();
        }

        // Code Editor windows
        for (auto &w : g_openedEditors)
        {
            ImGui::Begin((w.first + "##txt").c_str(), w.second.winOpened,
                         (ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar));
            ImGui::SetWindowSize(ImVec2(yg::input::get(yg::input::WINDOW_WIDTH) * 0.5f,
                                        yg::input::get(yg::input::WINDOW_HEIGHT) * 0.75f),
                                 ImGuiCond_FirstUseEver);

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save"))
                    {
                        std::string textToSave = w.second.editor.GetText();
                        yg::file::writeFile(w.first, &(textToSave[0]), textToSave.size());
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            w.second.editor.Render("TextEditor");
            ImGui::End();
        }

        // Code Hex Editor windows
        for (auto &w : g_openedHexEditors)
        {
            ImGui::Begin((w.first + "##hex").c_str(), w.second.winOpened,
                         (ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar));
            ImGui::SetWindowSize(ImVec2(yg::input::get(yg::input::WINDOW_WIDTH) * 0.5f,
                                        yg::input::get(yg::input::WINDOW_HEIGHT) * 0.75f),
                                 ImGuiCond_FirstUseEver);

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save"))
                    {
                        yg::file::writeFile(w.first, &(w.second.data[0]), w.second.data.size());
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            w.second.editor.DrawContents(&(w.second.data[0]), w.second.data.size());
            ImGui::End();
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

    void loadFlavor()
    {
        {
            // load initial flavor file from assets, or project path, if set
            std::string flavorName = "a//" + g_flavorName;
            if (yg::file::getProjectFilePath("") != "")
            {
                flavorName = "p//" + g_flavorName;
            }

            // load flavor file
            std::vector<uint8_t> data;
            if (yg::file::readFile(flavorName, data) == 0)
            {
                // add null terminator
                data.push_back((uint8_t)0);
            }
            else
            {
                yg::log::error("failed to load flavor from file %v", flavorName);
                return;
            }

            try
            {
                g_flavor = json::parse(&(data[0]));
            }
            catch (json::parse_error &e)
            {
                yg::log::warn("failed to parse json (flavor): %v", std::string(e.what()));
                return;
            }
        }
    }
} // namespace mygame
