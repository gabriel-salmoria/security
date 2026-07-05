#include "gui/App.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("IBE Mail", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // --- Load a nicer font ---
    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16.0f);

    // --- Custom dark-teal theme ---
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Geometry
    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 4.0f;
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.PopupRounding     = 4.0f;

    // Spacing & Padding
    style.WindowPadding     = ImVec2(10, 10);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ItemInnerSpacing  = ImVec2(6, 4);
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 12.0f;
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.TabBorderSize     = 1.0f;

    // Colors — sleek dark blue/teal palette
    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg]            = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
    c[ImGuiCol_ChildBg]             = ImVec4(0.10f, 0.10f, 0.15f, 1.00f);
    c[ImGuiCol_PopupBg]             = ImVec4(0.10f, 0.10f, 0.15f, 0.96f);
    c[ImGuiCol_Border]              = ImVec4(0.20f, 0.22f, 0.30f, 0.60f);

    c[ImGuiCol_FrameBg]             = ImVec4(0.14f, 0.15f, 0.22f, 1.00f);
    c[ImGuiCol_FrameBgHovered]      = ImVec4(0.20f, 0.22f, 0.32f, 1.00f);
    c[ImGuiCol_FrameBgActive]       = ImVec4(0.24f, 0.27f, 0.40f, 1.00f);

    c[ImGuiCol_TitleBg]             = ImVec4(0.06f, 0.06f, 0.10f, 1.00f);
    c[ImGuiCol_TitleBgActive]       = ImVec4(0.10f, 0.14f, 0.22f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed]    = ImVec4(0.06f, 0.06f, 0.10f, 0.75f);

    c[ImGuiCol_Tab]                 = ImVec4(0.12f, 0.14f, 0.22f, 1.00f);
    c[ImGuiCol_TabHovered]          = ImVec4(0.22f, 0.42f, 0.58f, 0.90f);
    c[ImGuiCol_TabActive]           = ImVec4(0.16f, 0.34f, 0.50f, 1.00f);
    c[ImGuiCol_TabUnfocused]        = ImVec4(0.10f, 0.10f, 0.16f, 1.00f);
    c[ImGuiCol_TabUnfocusedActive]  = ImVec4(0.14f, 0.24f, 0.36f, 1.00f);

    c[ImGuiCol_Button]              = ImVec4(0.18f, 0.36f, 0.52f, 1.00f);
    c[ImGuiCol_ButtonHovered]       = ImVec4(0.24f, 0.48f, 0.66f, 1.00f);
    c[ImGuiCol_ButtonActive]        = ImVec4(0.14f, 0.30f, 0.46f, 1.00f);

    c[ImGuiCol_Header]              = ImVec4(0.16f, 0.30f, 0.44f, 0.70f);
    c[ImGuiCol_HeaderHovered]       = ImVec4(0.22f, 0.42f, 0.58f, 0.80f);
    c[ImGuiCol_HeaderActive]        = ImVec4(0.18f, 0.38f, 0.54f, 1.00f);

    c[ImGuiCol_Separator]           = ImVec4(0.20f, 0.24f, 0.34f, 0.60f);
    c[ImGuiCol_SeparatorHovered]    = ImVec4(0.26f, 0.46f, 0.64f, 0.80f);
    c[ImGuiCol_SeparatorActive]     = ImVec4(0.26f, 0.46f, 0.64f, 1.00f);

    c[ImGuiCol_ScrollbarBg]         = ImVec4(0.06f, 0.06f, 0.10f, 0.50f);
    c[ImGuiCol_ScrollbarGrab]       = ImVec4(0.20f, 0.24f, 0.34f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered]= ImVec4(0.28f, 0.34f, 0.46f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.34f, 0.42f, 0.56f, 1.00f);

    c[ImGuiCol_CheckMark]           = ImVec4(0.40f, 0.72f, 0.90f, 1.00f);
    c[ImGuiCol_SliderGrab]          = ImVec4(0.30f, 0.56f, 0.76f, 1.00f);
    c[ImGuiCol_SliderGrabActive]    = ImVec4(0.36f, 0.64f, 0.86f, 1.00f);

    c[ImGuiCol_TextSelectedBg]      = ImVec4(0.20f, 0.40f, 0.58f, 0.50f);

    c[ImGuiCol_TableHeaderBg]       = ImVec4(0.12f, 0.16f, 0.24f, 1.00f);
    c[ImGuiCol_TableBorderStrong]   = ImVec4(0.18f, 0.22f, 0.32f, 1.00f);
    c[ImGuiCol_TableBorderLight]    = ImVec4(0.14f, 0.18f, 0.26f, 1.00f);
    c[ImGuiCol_TableRowBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_TableRowBgAlt]       = ImVec4(0.10f, 0.12f, 0.18f, 0.40f);

    c[ImGuiCol_Text]                = ImVec4(0.88f, 0.90f, 0.94f, 1.00f);
    c[ImGuiCol_TextDisabled]        = ImVec4(0.44f, 0.46f, 0.52f, 1.00f);

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ibemail::gui::App app("implementation/data");

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        app.Render();

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.06f, 0.06f, 0.09f, 1.00f);

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
