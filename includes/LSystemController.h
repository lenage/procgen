#ifndef LSYSTEM_CONTROLLER
#define LSYSTEM_CONTROLLER


#include <chrono>
#include <optional>
#include <list>

#include "SFML/Graphics.hpp"

#include "LSystemView.h"

namespace controller
{
    // LSystemController manages the behaviour of the LSystemViews beyond the
    // GUI: selection, dragging and riglt-click for example.
    //
    // This class is a singleton implemented as a static class.
    class LSystemController
    {
    public:
        // Delete the constructor to implement the singleton.
        LSystemController() = delete;

        // If the mouse if above a LSystemView ('under_mouse_'),
        // LSystemController has the priority.
        static bool has_priority();

        // Handle 'event' for the 'views'.
        static void handle_input(std::list<procgui::LSystemView>& views, const sf::Event& event);

        // Handle the dragging behaviour.
        static void handle_delta(sf::Vector2f delta);

        // Interact with a menu when right-clicking on a LSystemView (cloning,
        // duplicating, ...) 
        static void right_click_menu(std::list<procgui::LSystemView>& views);

        // Getters
        static const std::optional<procgui::LSystemView>& saved_view();
        static const procgui::LSystemView* under_mouse();
        static bool is_clone();

    private:
        // Delete the LSystemView with identifier 'id' in 'views'
        static void delete_view(std::list<procgui::LSystemView>& views, int id);
        
        // The LSystemView below the mouse. nullptr if there is
        // nothing. Non-owning pointer.
        static procgui::LSystemView* under_mouse_;

        // When copying or duplicating in a right-click, we save the LSystemView
        // in this variable. optional<> is used to initialize an empty variable
        // when starting up the application.
        static std::optional<procgui::LSystemView> saved_view_;

        // The delay between two click before considering a double-click. Based
        // on the imgui time.
        static const std::chrono::duration<unsigned long long, std::milli> double_click_time_;
        // Previous left-click timestamp.
        static std::chrono::time_point<std::chrono::steady_clock> click_time_;

        // True if saved 'save_view_' is to be cloned. False if it is to be
        // duplicated.
        static bool is_clone_;
    };
}


#endif // LSYSTEM_CONTROLLER
