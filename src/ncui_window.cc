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

using namespace ncui;

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

class Window::WindowImpl {
  WINDOW*               win;

  Window*               parent_win;
  WINDOW*               parent_win_handle;
  std::vector<Window*>  children;

  bool                  is_bordered  : 1;
  bool                  is_dirty     : 1;
  bool                  is_textfield : 1;
  bool                  was_resized  : 1;
  bool                  has_focus    : 1;

  field_buf_t*          p_text_buf;

  int                   h, w, y, x;

  dim_t                 win_dim;
  coord_t               win_coord;
  cursor_t              cur;
  dim_t                 resize_dim;

  win_ev_entry_t        ev_lookup[WIN_EV_MAX];
  win_cb_t              update_cb;
  win_cb_data_t         update_cb_data;

  pthread_t             event_handler_thread;

  friend class Window;
};

WINDOW* Window::get_win_handle() {
  return pimpl->win;
}

void Window::add_child(Window* child) {
  pimpl->children.push_back(child);
}

void Window::del_child(Window* child) {
  pimpl->children.erase(std::remove(
        pimpl->children.begin(),
        pimpl->children.end(), child),
        pimpl->children.end()
      );
}

void* Window::event_handler(void *param) {
#if 0    
  Window *p_me = (Window*)param;
#else
  Window& me = *( (Window*)param );
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
    case KEY_MOUSE:
    {
      if (getmouse(&ev) == OK) {
        win_ev = WIN_EV_MOUSE;
        /* copy data */
        me.pimpl->ev_lookup[win_ev].cb_data = &ev.bstate;
      }
      break;
    }
    case KEY_RESIZE:
    {
      if (me.pimpl->was_resized == true) {
        win_ev = WIN_EV_RESIZE;
        me.pimpl->ev_lookup[win_ev].cb_data = &me.pimpl->resize_dim;
      }
      break;
    }
    default:
    {
      if (me.pimpl->is_textfield) {
        /* Backspace needs special handling */
        if (key == KEY_BACKSPACE) {
          win_ev = WIN_EV_TERM;
          me.pimpl->ev_lookup[win_ev].cb_data = &key;
          me.bksp();
        }
        /* Keyboard events */
        else if ((key >= 0) &&
                 (key <= 127)) {
          win_ev = WIN_EV_TERM;
          me.pimpl->ev_lookup[win_ev].cb_data = &key;

          if (me.pimpl->is_textfield == true) {
            me.addchar(key);

            if (key == 10)
              me.pimpl->p_text_buf->newline();
            else
              me.pimpl->p_text_buf->put(key);
          }
          if (me.pimpl->is_bordered == true) {
            /* Enter/Return key */
            if (key == 10) {
              if (me.pimpl->cur.y < me.pimpl->win_dim.h)
                me.move_cur(++me.pimpl->cur.y, 1);
              else if (me.pimpl->cur.y == me.pimpl->win_dim.h)
                me.move_cur(me.pimpl->cur.y, --me.pimpl->cur.x);
              me.box();
            }
            else if (me.pimpl->cur.x > me.pimpl->win_dim.w) {
              if (me.pimpl->cur.y < me.pimpl->win_dim.h) {
                me.move_cur(++me.pimpl->cur.y, 1);
              }
              else if (me.pimpl->cur.y == me.pimpl->win_dim.h) {
                me.move_cur(me.pimpl->win_dim.h, me.pimpl->win_dim.w);
                if (me.pimpl->is_textfield) {
                  me.pimpl->p_text_buf->move_col_rel(-1);
                }
              }
            }
          }
        }
        /* Movement keys */
        else if ((key >= KEY_DOWN) &&
                 (key < KEY_MOUSE)) {
          win_ev = WIN_EV_KEY;
          me.pimpl->ev_lookup[win_ev].cb_data = &key;
        }
      } /* if (is_textfield) */
    }
  }

  if (win_ev != WIN_EV_NONE) {
    win_ev_entry_t ev_data = me.pimpl->ev_lookup[win_ev];
    if (ev_data.cb != NULL) {
      ev_data.cb(ev_data.cb_data, ev_data.user_data);
    }
  }

  return NULL;
}

Window::Window(const int _h, const int _w,
               const int _y, const int _x,
               bool _is_bordered,
               bool _is_textfield) {

  pimpl = new WindowImpl();

  pimpl->parent_win_handle = NULL;

  pimpl->win = newwin(_h, _w, _y, _x);
  if (pimpl->win == NULL) {
    throw std::runtime_error("Window: creation failed!");
  }

  for (int i = 0; i < WIN_EV_MAX; i++) {
    pimpl->ev_lookup[i].cb = NULL;
    pimpl->ev_lookup[i].cb_data = NULL;
    pimpl->ev_lookup[i].user_data = NULL;
  }

  pimpl->update_cb = &Window::event_handler;
  pimpl->update_cb_data = this;

  pimpl->win_dim.h = _h;
  pimpl->win_dim.w = _w;
  pimpl->win_coord.y = _y;
  pimpl->win_coord.x = _x;
  pimpl->cur.x = pimpl->cur.y = 0;

  pimpl->is_bordered = _is_bordered;
  if (pimpl->is_bordered == TRUE) {
    pimpl->win_dim.h -= 2;
    pimpl->win_dim.w -= 2;
    move_cur(1,1);
  }

  pimpl->is_textfield = _is_textfield;
  if (pimpl->is_textfield == TRUE) {
    keypad(pimpl->win, TRUE);
    nodelay(pimpl->win, TRUE);

    pimpl->p_text_buf = new field_buf_t(pimpl->win_dim.h, pimpl->win_dim.w);
  }
  else {
    pimpl->p_text_buf = NULL;
  }

  clear();
  if (pimpl->is_bordered == TRUE) {
    box();
  }

  Screen::get_instance()->add_win(this);
}

Window::Window(Window* p_parent_win,
               const int _h, const int _w,
               const int _y, const int _x,
               bool _is_bordered,
               bool _is_textfield) {

  pimpl = new WindowImpl();

  WINDOW *_parent_win_handle = p_parent_win->get_win_handle();
  if (_parent_win_handle == NULL) {
    throw std::runtime_error("Window: invalid parent!");
  }
  pimpl->win = derwin(_parent_win_handle, _h, _w, _y, _x);
  if (pimpl->win == NULL) {
    throw std::runtime_error("Window: creation failed!");
  }

  pimpl->update_cb = &Window::event_handler;
  pimpl->update_cb_data = this;

  pimpl->parent_win_handle = _parent_win_handle;
  pimpl->parent_win = p_parent_win;

  for (int i = 0; i < WIN_EV_MAX; i++) {
    pimpl->ev_lookup[i].cb = NULL;
    pimpl->ev_lookup[i].cb_data = NULL;
    pimpl->ev_lookup[i].user_data = NULL;
  }

  pimpl->win_dim.h = _h;
  pimpl->win_dim.w = _w;
  pimpl->win_coord.y = _y;
  pimpl->win_coord.x = _x;
  pimpl->cur.x = pimpl->cur.y = 0;

  pimpl->is_bordered = _is_bordered;
  if (pimpl->is_bordered == TRUE) {
    pimpl->win_dim.h -= 2;
    pimpl->win_dim.w -= 2;
    move_cur(1,1);
  }

  pimpl->is_textfield = _is_textfield;
  if (pimpl->is_textfield == TRUE) {
    keypad(pimpl->win, TRUE);
    nodelay(pimpl->win, TRUE);

    pimpl->p_text_buf = new field_buf_t(pimpl->win_dim.h, pimpl->win_dim.w);
  }
  else {
    pimpl->p_text_buf = NULL;
  }

  clear();
  if (pimpl->is_bordered == TRUE) {
    box();
  }

  p_parent_win->add_child(this);
  Screen::get_instance()->add_win(this);
}

Window::~Window() {

  wclear(pimpl->win);
  wrefresh(pimpl->win);
  if (pimpl->p_text_buf != NULL) {
    delete pimpl->p_text_buf;
    pimpl->p_text_buf = NULL;
  }
  if (pimpl->parent_win != NULL) {
    pimpl->parent_win->del_child(this);
  }
  Screen::get_instance()->remove_win(this);
  delwin(pimpl->win);

  delete pimpl;
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

Window* Window::create_window(Window* p_parent_win,
                              const int _h, const int _w,
                              const int _y, const int _x,
                              bool _is_bordered,
                              bool _is_textfield) {

  Window* new_win = NULL;
  try {
    new_win  = new Window(p_parent_win,
                          _h, _w,
                          _y, _x,
                          _is_bordered,
                          _is_textfield);
  }
  catch(std::exception e) {
    return NULL;
  }
  return new_win;
}

void Window::destroy_win(Window *_win) {
  if (_win != NULL) {
    delete _win;
    _win = NULL;
  }
}

void Window::reg_cb(win_cb_t cb, win_cb_data_t cb_data) {
  pimpl->update_cb = cb;
  pimpl->update_cb_data = cb_data;
}

void Window::dereg_cb() {
  pimpl->update_cb = NULL;
  pimpl->update_cb_data = NULL;
}

void Window::reg_event_handler(win_event_t ev, win_ev_cb_t cb, win_ev_user_data_t user_data) {
  pimpl->ev_lookup[ev].cb = cb;
  pimpl->ev_lookup[ev].user_data = user_data;
}

void Window::dereg_cb(win_event_t ev) {
  pimpl->ev_lookup[ev].cb = NULL;
  pimpl->ev_lookup[ev].user_data = NULL;
}

void Window::box() {
  ::box(pimpl->win, 0, 0);    
  pimpl->is_dirty = true;
}

void Window::print(int y, int x, std::string str) {
  if (!pimpl->is_textfield) {

    if (!pimpl->is_bordered) {
      mvwprintw(pimpl->win, y, x, str.c_str());

    } else {
      int height = pimpl->win_dim.h;
      std::size_t len = str.length();
      std::size_t pos = 0;

      if (pimpl->is_bordered) {
        y++, x++;
      }

      bool first_line = true;
      while (len && y <= height) {
        std::size_t count = pimpl->win_dim.w - (x - 1);
        std::string tmp = str.substr(pos, count);
        count = tmp.length();
        mvwprintw(pimpl->win, y++, x, tmp.c_str());
        len -= count;
        pos += count;
        if (first_line) {
          x = (pimpl->is_bordered) ? 1: 0;
          first_line = false;
        }
      }
    }

    pimpl->is_dirty = true;
  }
}

void Window::print(std::string str) {
  print(0, 0, str);
#if 0
  if (!pimpl->is_textfield) {

    if (!pimpl->is_bordered) {
      wprintw(pimpl->win, str.c_str());

    } else {
      int y = 0, x = 0;
      int height = pimpl->win_dim.h;
      std::size_t len = str.length();
      std::size_t pos = 0;

      if (pimpl->is_bordered) {
        y = x = 1;
      }

      while (len && y <= height) {
        std::size_t count = pimpl->win_dim.w;
        std::string tmp = str.substr(pos, count);
        count = tmp.length();
        mvwprintw(pimpl->win, y++, x, tmp.c_str());
        len -= count;
        pos += count;
      }
    }

    pimpl->is_dirty = true;
  }
#endif
}

void Window::move(int _y, int _x) {
  pimpl->win_coord.y = _y;
  pimpl->win_coord.x = _x;
  mvwin(pimpl->win, pimpl->win_coord.y, pimpl->win_coord.x);
  pimpl->is_dirty = true;
}

void Window::move_cur(int _y, int _x) {
  pimpl->cur.y = _y;
  pimpl->cur.x = _x;
  if (pimpl->is_bordered) {
    if (pimpl->cur.y < 1)
      pimpl->cur.y = 1;
    if (pimpl->cur.x < 1)
      pimpl->cur.x = 1;
  }
  else {
    if (pimpl->cur.y < 0)
      pimpl->cur.y = 0;
    if (pimpl->cur.x < 0)
      pimpl->cur.x = 0;
  }
  if (pimpl->cur.y > pimpl->win_dim.h)
    pimpl->cur.y = pimpl->win_dim.h;
  if (pimpl->cur.x > pimpl->win_dim.w)
    pimpl->cur.x = pimpl->win_dim.w;
  wmove(pimpl->win, pimpl->cur.y, pimpl->cur.x);
  pimpl->is_dirty = true;
}

void Window::move_cur_rel(int _y_offset, int _x_offset) {
  pimpl->cur.y += _y_offset;
  pimpl->cur.x += _x_offset;
  if (pimpl->is_bordered) {
    if (pimpl->cur.y < 1)
      pimpl->cur.y = 1;
    if (pimpl->cur.x < 1)
      pimpl->cur.x = 1;
  }
  else {
    if (pimpl->cur.y < 0)
      pimpl->cur.y = 0;
    if (pimpl->cur.x < 0)
      pimpl->cur.x = 0;
  }
  if (pimpl->cur.y > pimpl->win_dim.h)
    pimpl->cur.y = pimpl->win_dim.h;
  if (pimpl->cur.x > pimpl->win_dim.w)
    pimpl->cur.x = pimpl->win_dim.w;
  wmove(pimpl->win, pimpl->cur.y, pimpl->cur.x);
  pimpl->is_dirty = true;
}


void Window::get_cur(int& _y, int& _x) {
  getyx(pimpl->win, _y, _x);
}

void Window::clear() {
  wclear(pimpl->win);
  if (pimpl->is_bordered == true)
    move_cur(1, 1);
  else
    move_cur(0, 0);
}

void Window::draw() {
  //if (is_bordered == TRUE)
  //{
  //  box();
  //}
  pimpl->is_dirty = false;
  if (pimpl->parent_win_handle) {
    touchwin(pimpl->parent_win_handle);
  }
  wrefresh(pimpl->win);
}

void Window::update() {
  if (pimpl->has_focus) {
    if(pimpl->update_cb != NULL) {
      pimpl->update_cb(pimpl->update_cb_data);
    }
  }

  if (pimpl->is_dirty) {
    draw();
  }
}

int Window::getchar()
{
  return wgetch(pimpl->win);
}

int Window::addchar(char c) {
  pimpl->is_dirty = true;
  pimpl->cur.x++;
  return waddch(pimpl->win, c);
}

void Window::bksp() {
  if (((pimpl->is_bordered == true) && (pimpl->cur.x > 1)) &&
      (pimpl->cur.x > 0)) {
    mvwaddch(pimpl->win, pimpl->cur.y, --pimpl->cur.x, ' ');
    wmove(pimpl->win, pimpl->cur.y, pimpl->cur.x);
    pimpl->is_dirty = true;
  }
  else if ((pimpl->is_bordered == true && pimpl->cur.y > 1) &&
           pimpl->cur.y > 0) {
    pimpl->cur.x = pimpl->win_dim.w;
    mvwaddch(pimpl->win, --pimpl->cur.y, pimpl->cur.x, ' ');
    wmove(pimpl->win, pimpl->cur.y, pimpl->cur.x);
    pimpl->is_dirty = true;
  }
  if (pimpl->is_textfield == true) {
    pimpl->p_text_buf->bksp();
  }
}

void Window::set_focus(bool focus) {
  pimpl->has_focus = focus;
}
