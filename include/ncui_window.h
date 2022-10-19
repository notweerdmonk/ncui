/**
 * @file ncui_window.h
 * @author notweerdmonk
 * @brief Declaration of ncui::Window class.
 */

#ifndef NCURSES_WINDOW_H
#define NCURSES_WINDOW_H

#include <ncui_common.h>
#include <ncui_types.h>
#include <ncui_field_buffer.h>

namespace ncui {

  /**
   * Events that can occur on an window.
   */
  typedef enum {
    WIN_EV_NONE = -1,
    WIN_EV_KEY,       /**< Arrow and function keys */
    WIN_EV_TERM,      /**< ASCII characters from NUL to DEL */
    WIN_EV_MOUSE,     /**< Mouse events */
    WIN_EV_RESIZE,    /**< Resize event */
    WIN_EV_MAX        /**< Guard value */
  } win_event_t;

  /**
   * @brief A class to manage ncurses windows.
   */
  class Window {
    /* Forward declaration of ncui::Window::WindowImpl class */
    class WindowImpl;

    /**
     * A pointer to an instance of ncui::Window::WindowImpl class that
     * implements the functionality.
     */
    std::unique_ptr<WindowImpl> pimpl;

    Window* parent_window;

    std::vector<Window*> children;

  private:
    /**
     * @brief Get the ncurses WINDOW pointer for the current window.
     * @return A ncurses WINDOW pointer.
     */
    WINDOW *get_win_handle();

    /**
     * @brief Add a child window.
     * @pram child A pointer to ncui::Window object to add to the list of
     * children.
     */
    void add_child(Window* child);

    /**
     * @brief Remove a child window.
     * @param child A pointer to ncui::Window object to remove from the list of
     * children.
     */
    void del_child(Window* child);

    /**
     * @brief Draw a border around the window.
     */
    void box();

    /**
     * @brief Constructor.
     * Creates a new window without a parent.
     * @param _h The height of the window.
     * @param _w The width of the window.
     * @param _y The ordinate of the window.
     * @param _x The abscissa of the window.
     * @param _is_bordered Boolean flag specifying whether the window should be
     * bordered or not.
     * @param _is_textfield Boolean flag specifying whether the window is a
     * text field or not.
     */
    Window(
        const int _h, const int _w,
        const int _y, const int _x,
        bool _is_bordered = 0,
        bool _is_textfield = 0
      );

    /**
     * @brief Constructor.
     * Creates a new ncui::Window object as a child of given parent window.
     * @param p_parent_win A pointer to parent ncui::Window object.
     * @param _h The height of the window.
     * @param _w The width of the window.
     * @param _y The ordinate of the window.
     * @param _x The abscissa of the window.
     * @param _is_bordered Boolean flag specifying whether the window should be
     * bordered or not.
     * @param _is_textfield Boolean flag specifying whether the window is a
     * text field or not.
     */
    Window(
        Window* p_parent_win,
        const int _h, const int _w,
        const int _y, const int _x,
        bool _is_bordered = 0,
        bool _is_textfield = 0
      );

    /**
     * @brief Destructor.
     * Destroy a ncui::Window object.
     */
    ~Window();

  public:
    /**
     * @brief A static function to create a new window without a parent.
     * @param _h The height of the window.
     * @param _w The width of the window.
     * @param _y The ordinate of the window.
     * @param _x The abscissa of the window.
     * @param _is_bordered Boolean flag specifying whether the window should be
     * bordered or not.
     * @param _is_textfield Boolean flag specifying whether the window is a
     * text field or not.
     * @return A pointer to ncui::Window object.
     */
    static Window* create_window(
        const int _h, const int _w,
        const int _y, const int _x,
        bool _is_bordered = 0,
        bool _is_textfield = 0
      );

    /**
     * @brief A static function to creates a new ncui::Window object as a child
     * of given parent window.
     * @param p_parent_win A pointer to parent ncui::Window object.
     * @param _h The height of the window.
     * @param _w The width of the window.
     * @param _y The ordinate of the window.
     * @param _x The abscissa of the window.
     * @param _is_bordered Boolean flag specifying whether the window should be
     * bordered or not.
     * @param _is_textfield Boolean flag specifying whether the window is a
     * text field or not.
     * @return A pointer to ncui::Window object.
     */
    static Window* create_window(
        Window* p_parent_win,
        const int _h, const int _w,
        const int _y, const int _x,
        bool _is_bordered = 0,
        bool _is_textfield = 0
      );

    /**
     * @brief A static function to destroy a ncui::Window object.
     * @param _win A pointer to ncui::Window object.
     */
    static void destroy_win(Window *_win);

    /**
     * @brief Regsiter window update callback.
     * @param cb The callback function.
     * @param cb_data The data passed to the callback function.
     */
    void reg_cb(win_cb_t cb, win_cb_data_t cb_data);

    /**
     * @breif Deregister the current window update callback.
     */
    void dereg_cb();

    /**
     * @brief Register a callback for an window event.
     * @param ev The window event.
     * @param cb The callback function.
     * @param user_data The data passed to the callback function.
     */
    void reg_event_handler(
        win_event_t ev,
        win_ev_cb_t cb,
        win_ev_user_data_t user_data
      );

    /**
     * @brief Deregister the current callback for an window event.
     * @param ev The window event.
     */
    void dereg_cb(win_event_t ev);

    /**
     * @brief Print a string in the window at given coordinates.
     * @param y The ordinate.
     * @param x The abscissa.
     * @param str The std::string object to print.
     */
    void print(int y, int x, std::string str);

    /**
     * @brief Print a string in the window at 0, 0.
     * @param str The std::string to print.
     */
    void print(std::string str);

    /**
     * @brief Move the ncurses window to given coordinates.
     * @param _y The ordinate.
     * @param _x The abscissa.
     */
    void move(int _y, int _x);

    /**
     * @brief Move the curser to given coordinates.
     * @param _y The ordinate.
     * @param _x The abscissa.
     */
    void move_cur(int _y, int _x);

    /**
     * @brief Move the cursor relative to its current position.
     * @param _y_off The ordinate offset.
     * @param _x_off The abscissa offset.
     */
    void move_cur_rel(int _y_off, int _x_off);

    /**
     * @brief Get the current coordinates of the cursor.
     * @param[out] y The ordinate.
     * @param[out] x The abscissa.
     */
    void get_cur(int& y, int& x);

    /**
     * @brief Clear the ncurses window.
     */
    void clear();

    /**
     * @brief Draw the ncurses window.
     */
    void draw();

    /**
     * @brief Call the window update callback and draw the ncurses window if
     * required.
     */
    void update();

    /**
     * @brief Get a character from the ncurses window.
     * @return The input character.
     */
    int getchar();

    /**
     * @brief Print a character to the ncurses window at the current cursor.
     * @return Integer ERR on error, OK otherwise.
     */
    int addchar(char c);

    /**
     * @brief Put a backspace character in the ncurses window. Emulate a
     * backspace character in the text field buffer if the window is a
     * textfield.
     */
    void bksp();

    /**
     * @brief Set or unset focus on the ncurses window.
     * @param focus Boolean flag to set or unset focus.
     */
    void set_focus(bool focus);

    /**
     * @brief Check if the ncui::Window is a textfield.
     * @return true or false.
     */
    bool is_textfield();

    /**
     * @brief Mark a ncui::Window as dirty. Dirty windows are redrawn.
     */
    void mark_dirty();

    /**
     * @brief Tests whether a given pair of screen-relative character-cell
     * coordinates is enclosed by a given window.
     * @return true or falase.
     */
    bool enclose(int y, int x);
  };

}

#endif /* NCURSES_WINDOW_H */
