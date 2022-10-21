/**
 * @file test_focus.cc
 * @brief Test focus cycling between textfields.
 */

#include <ncui.h>

using namespace ncui;

void* key_cb(win_ev_cb_data_t cb_data, win_ev_user_data_t user_data)
{
  int input = *(int*)(cb_data);
  Window* my_win = (Window*)user_data;
  switch(input) {
    case KEY_UP:
    {
      my_win->move_cur_rel(-1, 0);
      break;
    }
    case KEY_DOWN:
    {
      my_win->move_cur_rel(1, 0);
      break;
    }
    case KEY_LEFT:
    {
      my_win->move_cur_rel(0, -1);
      break;
    }
    case KEY_RIGHT:
    {
      my_win->move_cur_rel(0, 1);
      break;
    }
    case KEY_F(4):
    {
      Screen::exit_screen();
    }
  }
  return 0;
}

struct app_data {
  Screen *pscr;
  Window *pwindows[4];
};

void* mouse_cb(win_ev_cb_data_t cb_data, win_ev_user_data_t user_data)
{
  MEVENT input = *(MEVENT*)(cb_data);
  struct app_data data = *(struct app_data*)user_data;

  if (input.bstate == BUTTON1_CLICKED)
  {
    if (data.pwindows[0]->enclose(input.y, input.x)) {
      data.pscr->set_focus(data.pwindows[0]);
    } else if (data.pwindows[1]->enclose(input.y, input.x)) {
      data.pscr->set_focus(data.pwindows[1]);
    } else if (data.pwindows[2]->enclose(input.y, input.x)) {
      data.pscr->set_focus(data.pwindows[2]);
    } else if (data.pwindows[3]->enclose(input.y, input.x)) {
      data.pscr->set_focus(data.pwindows[3]);
    }
  }
  return 0;
}

int main()
{
  /* initialize */
  Screen &scr = Screen::get_instance();

  struct app_data data;
  data.pscr = &scr;

  scr.enable_mouse_events();
  scr.set_cursor(1);

  /* create windows */
  Window* my_win = Window::create_window(30, 60, 1, 1, true, false);

  Window* banner_win = Window::create_window(my_win, 3, 50, 2, 5, true, false);
  banner_win->print(0, 0, "Click on window to focus");

  Window *textfield_win_1 = Window::create_window(my_win, 5, 50, 5, 5, true, true);
  textfield_win_1->reg_event_handler(WIN_EV_KEY, &key_cb, textfield_win_1);
  textfield_win_1->reg_event_handler(WIN_EV_MOUSE, &mouse_cb, &data);
  data.pwindows[0] = textfield_win_1;

  Window *textfield_win_2 = Window::create_window(my_win, 5, 50, 10, 5, true, true);
  textfield_win_2->reg_event_handler(WIN_EV_KEY, &key_cb, textfield_win_2);
  textfield_win_2->reg_event_handler(WIN_EV_MOUSE, &mouse_cb, &data);
  data.pwindows[1] = textfield_win_2;

  Window *textfield_win_3 = Window::create_window(my_win, 5, 50, 15, 5, true, true);
  textfield_win_3->reg_event_handler(WIN_EV_KEY, &key_cb, textfield_win_3);
  textfield_win_3->reg_event_handler(WIN_EV_MOUSE, &mouse_cb, &data);
  data.pwindows[2] = textfield_win_3;

  Window *textfield_win_4 = Window::create_window(my_win, 5, 50, 20, 5, true, true);
  textfield_win_4->reg_event_handler(WIN_EV_KEY, &key_cb, textfield_win_4);
  textfield_win_4->reg_event_handler(WIN_EV_MOUSE, &mouse_cb, &data);
  data.pwindows[3] = textfield_win_4;

  scr.set_focus(textfield_win_1);

  /* main loop */
  scr.mainloop();

  /* deinitialize */
  Window::destroy_win(textfield_win_4);
  Window::destroy_win(textfield_win_3);
  Window::destroy_win(textfield_win_2);
  Window::destroy_win(textfield_win_1);
  Window::destroy_win(banner_win);
  Window::destroy_win(my_win);

  scr.end_screen();

  exit_curses(EXIT_SUCCESS);
  
  return 0;
}
