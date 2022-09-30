/*
 * @file ncui_screen.cpp
 * @author notweerdmonk
 * @brief Manage CURSES screen.
 */

#include <ncui_screen.h>
#include <ncui_window.h>

using namespace ncui;

Screen* Screen::scr = NULL;

class Screen::ScreenImpl {
  bool                    exit_condition;

  std::vector<Window*>    windows;
  int                     num_windows;

  scr_cb_t                update_cb;
  scr_cb_data_t           update_cb_data;

  friend class Screen;
};

Screen::Screen() {

  pimpl = new ScreenImpl();

  void *status = initscr();
  if (status == NULL) {
    throw std::runtime_error("Screen: creation failed!");
  }
  else {
    curs_set(0);
    cbreak();
    noecho();
    mousemask(ALL_MOUSE_EVENTS, NULL);
  }
}

Screen::~Screen() {
  endwin();

  delete pimpl;
}

Screen* Screen::get_instance() {
  return scr;
}

Screen* Screen::init() {
  if (scr == NULL) {
    try {
      scr = new Screen();
    }
    catch(std::exception e) {
      return NULL;
    }
  }

  return scr;
}

int Screen::set_cursor(int visibility) {
  if (visibility < 0) {
    visibility = 0;
  } else if (visibility > 2) {
    visibility = 2;
  }

  return curs_set(visibility);
}

void Screen::enable_color() {
  if (scr != NULL) {
    start_color();
  }
}

void Screen::exit_screen() {
  Screen::get_instance()->pimpl->exit_condition = true;
}

bool Screen::should_exit() {
  return Screen::get_instance()->pimpl->exit_condition;
}

void Screen::print(int y, int x, std::string str) {
  mvprintw(y, x, "%s", str.c_str());
}

void Screen::refresh() {
  ::refresh();
}

void Screen::clear() {
  ::clear();
}

void Screen::add_win(Window* win) {
  pimpl->windows.push_back(win);
  pimpl->num_windows++;
}

void Screen::remove_win(Window* win) {
  try {
    pimpl->windows.erase(std::remove(pimpl->windows.begin(),
                                     pimpl->windows.end(), win),
                                     pimpl->windows.end());
    pimpl->num_windows--;
  }
  catch(std::exception e) {
    return;
  }
}

void Screen::end_screen() {
  if (get_instance != NULL) {
    if (scr->pimpl->num_windows > 0) {
      for (
          auto it = scr->pimpl->windows.begin();
          it != scr->pimpl->windows.end();
          it++
        ) {
        Window* p_win = *it;
        Window::destroy_win(p_win);
      }
    }
    delete scr;
    scr = NULL;
  }
}

void Screen::update() {
  if (scr != NULL) {
    if (scr->pimpl->num_windows > 0) {
      for (auto it = scr->pimpl->windows.begin();
           it != scr->pimpl->windows.end();
           it++) {
        Window* p_win = *it;
        p_win->update();
      }
    }
    else {
      if (scr->pimpl->update_cb != NULL) {
        scr->pimpl->update_cb(scr->pimpl->update_cb_data);
      }
    }
  }
}

void Screen::mainloop()
{
  while(!Screen::should_exit()) {
    Screen::update();
  }
}

void Screen::set_focus(Window *p_win) {
  if (scr != NULL) {
    if (scr->pimpl->num_windows > 0) {
      for (auto it = scr->pimpl->windows.begin();
           it != scr->pimpl->windows.end();
           it++) {
        Window* _win = *it;
        _win->set_focus((_win == p_win));
      }
    }
  }
}
