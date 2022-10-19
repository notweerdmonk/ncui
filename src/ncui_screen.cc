/*
 * @file ncui_screen.cpp
 * @author notweerdmonk
 * @brief Manage CURSES screen.
 */

#include <ncui_screen.h>
#include <ncui_window.h>

using namespace ncui;

class Screen::ScreenImpl {
  bool                    exit_cond;

  scr_cb_t                update_cb;
  scr_cb_data_t           update_cb_data;

  public:

  ScreenImpl() {
    
    void *status = initscr();
    if (status == NULL) {
      throw std::runtime_error("Screen: creation failed!");
    }
    else {
      curs_set(0);
      cbreak();
      noecho();
    }
  }

  ~ScreenImpl() {
  }

  int set_cursor(int visibility) {
    if (visibility < 0) {
      visibility = 0;
    } else if (visibility > 2) {
      visibility = 2;
    }

    return curs_set(visibility);
  }

  void enable_color() {
    start_color();
  }

  void enable_mouse_events() {
    mousemask(ALL_MOUSE_EVENTS, NULL);
  }

  void exit_screen() {
    exit_cond = true;
  }

  bool should_exit() {
    return exit_cond == true;
  }

  void print(int y, int x, std::string& str) {
    mvprintw(y, x, "%s", str.c_str());
  }

  void refresh() {
    ::refresh();
  }

  void clear() {
    ::clear();
  }

  void update() {
    if (update_cb) {
      update_cb(update_cb_data);
    }
  }
};

Screen::Screen() : focused_win(NULL), pimpl(new ScreenImpl()) {

}

Screen::~Screen() {

}

Screen& Screen::get_instance() {
  static Screen instance;
  return instance;
}

int Screen::set_cursor(int visibility) {
  return pimpl->set_cursor(visibility);
}

void Screen::enable_color() {
  pimpl->enable_color();
}

void Screen::enable_mouse_events() {
  pimpl->enable_mouse_events();
}

void Screen::exit_screen() {
  get_instance().pimpl->exit_screen();
}

bool Screen::should_exit() {
  return pimpl->should_exit();
}

void Screen::print(int y, int x, std::string &str) {
  pimpl->print(y, x, str);
}

void Screen::refresh() {
  pimpl->refresh();
}

void Screen::clear() {
  pimpl->clear();
}

void Screen::add_win(Window* win) {
  windows.push_back(win);
  ++num_windows;
  set_focus(win);
}

void Screen::remove_win(Window* win) {
  try {
    windows.erase(
        std::remove( windows.begin(), windows.end(), win),
        windows.end()
      );
    --num_windows;
  }
  catch(std::exception e) {
    return;
  }
}

void Screen::end_screen() {
  while (!windows.empty()) {
    Window* win = windows.back();
    Window::destroy_win(win);
  }
  endwin();
}

void Screen::update() {
  if (num_windows > 0) {
    for (auto w : windows) {
      w->update();
    }
  }
  else {
    pimpl->update();
  }
}

void Screen::mainloop() {
  while(!should_exit()) {
    update();
  }
}

void Screen::set_focus(Window *p_win) {
  if (focused_win) {
    focused_win->draw();
  }
  focused_win = p_win;
  if (p_win->is_textfield()) {
    for (auto w : windows) {
      w->set_focus((w == p_win));
    }
  }
}

void Screen::set_focus_next(Window *p_win) {
  auto res = std::find(windows.begin(), windows.end(), p_win);
  do {
    ++res;
    if (res == windows.end()) {
      res = windows.begin();
    }
  } while (!(*res)->is_textfield());
  set_focus(*res);
}
