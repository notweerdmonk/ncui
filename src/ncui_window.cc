/**
 * @file ncui_window.cpp
 * @author notweerdmonk
 * @brief Draw and manage windows with borders and text fields.
 */

#include <ncui_window.h>
#include <ncui_screen.h>

/* TODO: use references instead of pointers */
/* TODO: use forms library */
/* TODO: invisible borders */
/* TODO: title for windows */
/* TODO: use ncursesw and support wide characters */

using namespace ncui;

class Window::WindowImpl {

  typedef struct {
    win_ev_cb_t       cb;
    win_ev_cb_data_t  cb_data;
    win_ev_user_data_t user_data;
  } win_ev_entry_t;

  typedef struct {
    int h, w;
  } dim_t;

  typedef struct {
    int y, x;
  } coord_t;

  typedef struct {
    int x, y;
  } cursor_t;

  WINDOW*        win_handle;

  WINDOW*        parent_win_handle;

  Window*        win;

  bool           bordered  : 1;
  bool           textfield : 1;
  bool           dirty     : 1;
  bool           was_resized  : 1;
  bool           has_focus    : 1;

  field_buf_t*   p_text_buf;

  dim_t          win_dim;
  coord_t        win_coord;
  cursor_t       cur;
  dim_t          resize_dim;

  win_ev_entry_t ev_lookup[WIN_EV_MAX];
  win_cb_t       update_cb;
  win_cb_data_t  update_cb_data;

  pthread_t      event_handler_thread;

  public:

  WindowImpl(
      Window* win,
      WINDOW* parent_win,
      const int h, const int w,
      const int y, const int x,
      bool bordered,
      bool textfield
    ) {

    this->win = win;

    parent_win_handle = parent_win;

    if (parent_win) {

      win_handle = derwin(parent_win, h, w, y, x);
      if (win_handle == NULL) {
        throw std::runtime_error("WindowImpl: derwin failed");
      }
    } else {

      win_handle = newwin(h, w, y, x);
      if (win_handle == NULL) {
        throw std::runtime_error("WindowImpl: newwin failed");
      }
    }

    for (int i = 0; i < WIN_EV_MAX; i++) {
      ev_lookup[i].cb = NULL;
      ev_lookup[i].cb_data = NULL;
      ev_lookup[i].user_data = NULL;
    }

    update_cb = &event_handler;
    update_cb_data = this;

    win_dim.h = h;
    win_dim.w = w;
    win_coord.y = y;
    win_coord.x = x;
    cur.x = cur.y = 0;

    this->bordered = bordered;
    if (bordered == TRUE) {
      win_dim.h -= 2;
      win_dim.w -= 2;
    }

    this->textfield = textfield;
    if (textfield == TRUE) {
      keypad(win_handle, TRUE);
      nodelay(win_handle, TRUE);

      p_text_buf = new field_buf_t(win_dim.h, win_dim.w);
    }
    else {
      p_text_buf = NULL;
    }

    clear();

    if (bordered == TRUE) {
      box();
    }
  }

  ~WindowImpl() {
    wclear(win_handle);
    wrefresh(win_handle);
    if (p_text_buf != NULL) {
      delete p_text_buf;
      p_text_buf = NULL;
    }
    delwin(win_handle);
  }

  WINDOW* get_win_handle() {
    return win_handle;
  }

  /**
   * @brief 
   * @param param An opaque pointer.
   */
  static void* event_handler(void *param) {
#if 0    
    Window *p_me = (Window*)param;
#else
    WindowImpl& me = *((WindowImpl*)param);
#endif    

    MEVENT ev;
    win_event_t win_ev;
    int key;

    win_ev = WIN_EV_NONE;
    key = me.getchar();

    if (key == ERR) {
      return NULL;
    }

    switch(key) {
      case 9:
        {
          Screen::get_instance().set_focus_next(me.win);
          break;
        }
      case KEY_MOUSE:
        {
          if (getmouse(&ev) == OK) {
            win_ev = WIN_EV_MOUSE;
            /* copy data */
            me.ev_lookup[win_ev].cb_data = &ev.bstate;
          }
          break;
        }
      case KEY_RESIZE:
        {
          if (me.was_resized == true) {
            win_ev = WIN_EV_RESIZE;
            me.ev_lookup[win_ev].cb_data = &me.resize_dim;
          }
          break;
        }
      default:
        {
          if (me.textfield) {
            /* Backspace needs special handling */
            if (key == KEY_BACKSPACE) {
              win_ev = WIN_EV_TERM;
              me.ev_lookup[win_ev].cb_data = &key;
              me.bksp();
            }
            /* Keyboard events */
            else if ((key >= 0) &&
                (key <= 127)) {
              win_ev = WIN_EV_TERM;
              me.ev_lookup[win_ev].cb_data = &key;

              if (me.cur.x <= me.win_dim.w) {
                if (key != 9) {
                  me.addchar(key);
                }
              }

              if (key == 10) {
                me.p_text_buf->newline();
              }
              else {
                if (key != 9) {
                  me.p_text_buf->put(key);
                }
              }

              if (me.bordered == true) {
                /* Enter/Return key */
                if (key == 10) {
                  if (me.cur.y < me.win_dim.h) {
                    me.move_cur(++me.cur.y, 1);
                  } else if (me.cur.y == me.win_dim.h) {
                    me.move_cur(me.cur.y, --me.cur.x);
                  }
                  me.box();
                }
                else if (me.cur.x > me.win_dim.w) {
                  if (me.cur.y < me.win_dim.h) {
                    me.move_cur(++me.cur.y, 1);
                  }
                  else if (me.cur.y == me.win_dim.h) {
                    if (me.textfield) {
                      me.p_text_buf->move_col_rel(-1);
                    }
                  }
                }
              }

            }
            /* Movement keys */
            else if ((key >= KEY_DOWN) &&
                (key < KEY_MOUSE)) {
              win_ev = WIN_EV_KEY;
              me.ev_lookup[win_ev].cb_data = &key;
            }
          } /* if (textfield) */
        }
    }

    if (win_ev != WIN_EV_NONE) {
      win_ev_entry_t ev_data = me.ev_lookup[win_ev];
      if (ev_data.cb != NULL) {
        ev_data.cb(ev_data.cb_data, ev_data.user_data);
      }
    }

    return NULL;
  }

  void reg_cb(win_cb_t cb, win_cb_data_t cb_data) {
    update_cb = cb;
    update_cb_data = cb_data;
  }

  void dereg_cb() {
    update_cb = NULL;
    update_cb_data = NULL;
  }

  void reg_event_handler(
      win_event_t ev,
      win_ev_cb_t cb,
      win_ev_user_data_t user_data
    ) {

    ev_lookup[ev].cb = cb;
    ev_lookup[ev].user_data = user_data;
  }

  void dereg_cb(win_event_t ev) {
    ev_lookup[ev].cb = NULL;
    ev_lookup[ev].user_data = NULL;
  }

  void box() {
    ::box(win_handle, 0, 0);
    dirty = true;
  }

  void print(int y, int x, std::string str) {
    if (!textfield) {

      if (!bordered) {
        mvwprintw(win_handle, y, x, "%s", str.c_str());

      } else {
        int height = win_dim.h;
        std::size_t len = str.length();
        std::size_t pos = 0;

        if (bordered) {
          y++, x++;
        }

        bool first_line = true;
        while (len && y <= height) {
          std::size_t count = win_dim.w - (x - 1);
          std::string tmp = str.substr(pos, count);
          count = tmp.length();
          mvwprintw(win_handle, y++, x, "%s", tmp.c_str());
          len -= count;
          pos += count;
          if (first_line) {
            x = (bordered) ? 1: 0;
            first_line = false;
          }
        }
      }

      dirty = true;
    }
  }

  void move(int y, int x) {
    win_coord.y = y;
    win_coord.x = x;
    mvwin(win_handle, win_coord.y, win_coord.x);
    dirty = true;
  }

  void move_cur(int y, int x) {
    if (bordered) {
      if (y < 1) {
        y = 1;
      }
      if (x < 1) {
        x = 1;
      }
    }
    else {
      if (y < 0) {
        y = 0;
      }
      if (x < 0) {
        x = 0;
      }
    }

    if (y > win_dim.h) {
      y = win_dim.h;
    }
    if (x > win_dim.w) {
      x = win_dim.w;
    }

    wmove(win_handle, y, x);

    cur.y = y;
    cur.x = x;

    dirty = true;
  }

  void move_cur_rel(int y_offset, int x_offset) {
    cur.y += y_offset;
    cur.x += x_offset;

    if (bordered) {
      if (cur.y < 1) {
        cur.y = 1;
      }
      if (cur.x < 1) {
        cur.x = 1;
      }
    }
    else {
      if (cur.y < 0) {
        cur.y = 0;
      }
      if (cur.x < 0) {
        cur.x = 0;
      }
    }

    if (cur.y > win_dim.h) {
      cur.y = win_dim.h;
    }
    if (cur.x > win_dim.w) {
      cur.x = win_dim.w;
    }

    wmove(win_handle, cur.y, cur.x);

    dirty = true;
  }

  void get_cur(int& y, int& x) {
    getyx(win_handle, y, x);
  }

  void clear() {
    wclear(win_handle);
    if (bordered) {
      move_cur(1, 1);
    } else {
      move_cur(0, 0);
    }
  }

  void draw() {
    //if (bordered == TRUE) {
    //  box();
    //}
    dirty = false;
    if (parent_win_handle) {
      touchwin(parent_win_handle);
    }
    wrefresh(win_handle);
  }

  void update() {
    if (has_focus) {
      if(update_cb != NULL) {
        update_cb(update_cb_data);
      }
    }

    if (dirty) {
      draw();
    }
  }

  int getchar() {
    return wgetch(win_handle);
  }

  int addchar(char c) {
    dirty = true;
    ++cur.x;
    return waddch(win_handle, c);
  }

  void bksp() {
    if (((bordered == true) && (cur.x > 1)) &&
        (cur.x > 0)) {
      mvwaddch(win_handle, cur.y, --cur.x, ' ');
      wmove(win_handle, cur.y, cur.x);
      dirty = true;
    }
    else if ((bordered == true && cur.y > 1) &&
             cur.y > 0) {
      cur.x = win_dim.w;
      mvwaddch(win_handle, --cur.y, cur.x, ' ');
      wmove(win_handle, cur.y, cur.x);
      dirty = true;
    }

    if (textfield == true) {
      p_text_buf->bksp();
    }
  }

  void set_focus(bool focus) {
    if (has_focus = focus) {
      dirty = true;
    }
  }

  bool is_textfield() {
    return textfield;
  }

  void mark_dirty() {
    dirty = true;
  }
};

WINDOW* Window::get_win_handle() {
  return pimpl->get_win_handle();
}

void Window::add_child(Window* child) {
  children.push_back(child);
}

void Window::del_child(Window* child) {
  children.erase(
      std::remove(children.begin(), children.end(), child),
      children.end()
    );
}


Window::Window(
    const int h, const int w,
    const int y, const int x,
    bool bordered,
    bool textfield
  ) : pimpl(
        new WindowImpl(
          this,
          NULL,
          h, w,
          y, x,
          bordered,
          textfield
        )
      ) {

  Screen::get_instance().add_win(this);
}

Window::Window(
    Window* parent_window,
    const int h, const int w,
    const int y, const int x,
    bool bordered,
    bool textfield
  ) : pimpl(
    new WindowImpl(
      this,
      parent_window->get_win_handle(),
      h, w,
      y, x,
      bordered,
      textfield
    )
  ) {

  parent_window->add_child(this);
  Screen::get_instance().add_win(this);
}

Window::~Window() {
  if (parent_window != NULL) {
    parent_window->del_child(this);
  }
  Screen::get_instance().remove_win(this);
}

Window* Window::create_window(const int _h, const int _w,
    const int _y, const int _x,
    bool _is_bordered,
    bool _is_textfield) {

  Window* new_win = NULL;
  try {
    new_win  = new Window(_h, _w,
        _y, _x,
        _is_bordered,
        _is_textfield);
  }
  catch(std::exception e) {
    return NULL;
  }
  return new_win;
}

Window* Window::create_window(
    Window* p_parent_win,
    const int h, const int w,
    const int y, const int x,
    bool bordered,
    bool textfield
  ) {

  Window* new_win = NULL;
  try {
    new_win = new Window(
        p_parent_win,
        h, w,
        y, x,
        bordered,
        textfield
      );
  } catch(std::exception e) {
    return NULL;
  }
  return new_win;
}

void Window::destroy_win(Window *win) {
  if (win != NULL) {
    delete win;
    win = NULL;
  }
}

void Window::reg_cb(win_cb_t cb, win_cb_data_t cb_data) {
  pimpl->reg_cb(cb, cb_data);
}

void Window::dereg_cb() {
  pimpl->dereg_cb();
}

void Window::reg_event_handler(
    win_event_t ev,
    win_ev_cb_t cb,
    win_ev_user_data_t user_data
  ) {

  pimpl->reg_event_handler(ev, cb, user_data);
}

void Window::dereg_cb(win_event_t ev) {
  pimpl->dereg_cb(ev);
}

void Window::box() {
  pimpl->box();
}

void Window::print(int y, int x, std::string str) {
  pimpl->print(y, x, str);
}

void Window::print(std::string str) {
  pimpl->print(0, 0, str);
}

void Window::move(int y, int x) {
  pimpl->move(y, x);
}

void Window::move_cur(int y, int x) {
  pimpl->move_cur(y, x);
}

void Window::move_cur_rel(int y_offset, int x_offset) {
  pimpl->move_cur_rel(y_offset, x_offset);
}


void Window::get_cur(int& y, int& x) {
  pimpl->get_cur(y, x);
}

void Window::clear() {
  pimpl->clear();
}

void Window::draw() {
  pimpl->draw();
}

void Window::update() {
  pimpl->update();
}

int Window::getchar() {
  return pimpl->getchar();
}

int Window::addchar(char c) {
  return pimpl->addchar(c);
}

void Window::bksp() {
  pimpl->bksp();
}

void Window::set_focus(bool focus) {
  pimpl->set_focus(focus);
}

bool Window::is_textfield() {
  return pimpl->is_textfield();
}

void Window::mark_dirty() {
  pimpl->mark_dirty();
}
