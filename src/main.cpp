#include <iostream>
#include <fstream>
#include <cstdlib>

#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include "LSystem.h"
#include "RuleMapBuffer.h"
#include "InterpretationMapBuffer.h"
#include "Turtle.h"
#include "helper_math.h"
#include "procgui.h"
#include "WindowController.h"
#include "RenderWindow.h"

using namespace drawing;
using namespace math;
using namespace procgui;
using namespace controller;

int main()
{
    sf::RenderWindow window(sf::VideoMode(window::window_size.x, window::window_size.y), "Procgen");
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);

    auto serpinski = std::make_shared<LSystem>(LSystem { "F", { { 'F', "G-F-G" }, { 'G', "F+G+F" } } });
    auto plant = std::make_shared<LSystem>(LSystem { "X", { { 'X', "F[-X][X]F[-X]+FX" }, { 'F', "FF" } } });
    auto map = std::make_shared<InterpretationMap>(InterpretationMap
                                                     { { 'F', go_forward },
                                                     { 'G', go_forward },
                                                     { '+', turn_left  },
                                                     { '-', turn_right },
                                                     { '[', save_position },
                                                     { ']', load_position } });
    DrawingParameters serpinski_param;
    serpinski_param.starting_position = { 1000, 600 };
    serpinski_param.starting_angle = 0.f;
    serpinski_param.delta_angle = degree_to_rad(60.f);
    serpinski_param.step = 7;
    serpinski_param.n_iter = 5;

    DrawingParameters plant_param;
    plant_param.starting_position = { 400, 800 };
    plant_param.starting_angle = degree_to_rad(-80.f);
    plant_param.delta_angle = degree_to_rad(25.f);
    plant_param.step = 5;
    plant_param.n_iter = 6;

    LSystemView plant_view ("Plant", plant, map, plant_param);
    LSystemView serpinski_view ("Serpinski", serpinski, map, serpinski_param);
    
    std::list<LSystemView> views;
    views.push_back(std::move(plant_view));
    views.push_back(std::move(serpinski_view));

    sf::Clock delta_clock;
    while (window.isOpen())
    {
        window.clear();

        std::vector<sf::Event> events;
        sf::Event event;
        // ImGui has the priority as it is the topmost GUI.
        // The events are then redistributed in the rest of the application.
        while(window.pollEvent(event))
        {
            events.push_back(event);
            ImGui::SFML::ProcessEvent(event);
        }
        
        procgui::new_frame();
        ImGui::SFML::Update(window, delta_clock.restart());
        
        WindowController::handle_input(events, window, views);
 
        for (auto& v : views)
        {
            v.draw(window);
        }
        

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}