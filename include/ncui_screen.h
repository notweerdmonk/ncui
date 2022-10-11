/**
 * @file ncui_screen.h
 * @author notweerdmonk
 * @brief Declaration of ncui::Screen class.
 */

#ifndef NCURSES_SCREEN_H
#define NCURSES_SCREEN_H

#include <ncui_common.h>
#include <ncui_types.h>
#include <ncui_window.h>

namespace ncui {

  /**
   * @brief A class to manage ncurses screen.
   */
  class Screen {
    /* Forward declaration of ncui::Screen::ScreenImpl class */
    class ScreenImpl;

    /**
     * A pointer to an instance of ncui::Screen::ScreenImpl class that
     * implements the functionality.
     */
    std::unique_ptr<ScreenImpl> pimpl;

    std::vector<Window*> windows;

    int num_windows;

    /* Private constructor and destructor to enforce singleton class */
    Screen();
    ~Screen();

    /**
     * @brief Get the exit condition for the event loop.
     */
    bool should_exit();

  public:
    /**
     * @brief Get a pointer to the single instance of ncui::Screen class.
     * @return A pointer to an instance of ncui::Screen class.
     */
    static Screen& get_instance();

    /*
     * @brief Create the instance of ncui::Screen class if it does not exists.
     * @return A pointer to an instance of ncui::Screen class.
     */
    static Screen* init();

    /**
     * @brief Enable use of colors in the ncurses screen.
     */
    void enable_color();

    /**
     * @brief Set the visibility of the cursor.
     * @param visibility An integer stating the visibility.
     * <TABLE>
     * <TR>   <TD>Value</TD>  <TD>Description</TD>  </TR>
     * <TR>   <TD>0</TD>      <TD>Invisible</TD>    </TR>
     * <TR>   <TD>1</TD>      <TD>Normal</TD>       </TR>
     * <TR>   <TD>2</TD>      <TD>Very visible</TD> </TR>
     * </TABLE>
     */
    int set_cursor(int visibility);

    /**
     * @brief Set the exit condition for the event loop.
     */
    static void exit_screen();

    /**
     * @brief Destroy all child windows and the instance of ncui::Screen class.
     */
    void end_screen();

    /**
     * @brief Update each of the child windows.
     */
    void update();

    /**
     * @brief Run the event loop.
     */
    void mainloop();

    /**
     * @brief Give focus to a child window.
     * @param p_win A pointer to an object of ncui::Window class that is a
     * child.
     */
    void set_focus(Window *p_win);

    /**
     * @brief Move focus to next textfield window.
     * @param p_win A pointer to an object of ncui::Window class that is a
     * child. The focus shall move to the next ncui::Window that is a 
     * textfield, in the list of children.
     */
    void set_focus_next(Window *p_win);

    /**
     * @brief Print a string at given location.
     * @param y The row to start printing from.
     * @param x The column to start printing from.
     * @param str The std::string object to print.
     */
    void print(int y, int x, std::string &str);

    /**
     * @brief Draw the ncurses screen.
     */
    void refresh();

    /**
     * @brief Clear the ncurses screen.
     */
    void clear();

    /**
     * @brief Add a child window.
     * @param win A pointer to ncui::Window object to add to the list of
     * children.
     */
    void add_win(Window* win);

    /**
     * @brief Remove a child window.
     * @param win A pointer to ncui::Window object to remove from the list of
     * children.
     */
    void remove_win(Window* win);
  };

}

#endif /* NCURSES_SCREEN_H */
