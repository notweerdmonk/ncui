/*************************************/
/*               demo                */
/*************************************/

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

void* mouse_cb(win_ev_cb_data_t cb_data, win_ev_user_data_t user_data)
{
  mmask_t input = *(mmask_t*)(cb_data);
  if (input == BUTTON1_CLICKED)
  {
    Screen::exit_screen();
  }
  return 0;
}

int main()
{
  /* initialize */
  Screen &scr = Screen::get_instance();

  //scr.enable_color();
  scr.set_cursor(1);

  /* create windows */
  Window* my_win = Window::create_window(20, 60, 1, 1, true, false);

  Window* banner_win = Window::create_window(my_win, 3, 50, 2, 5, true, false);
  banner_win->print(0, 0, "Banner");

  Window* info_win = Window::create_window(my_win, 5, 50, 5, 5, true, false);
  info_win->print(
      "the quick brown fox jumps over the lazy dog THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG "
      "She sells pwned sea shells on the sea shore"
    );

  Window* textfield_win = Window::create_window(my_win, 5, 50, 10, 5, true, true);
  textfield_win->reg_event_handler(WIN_EV_KEY, &key_cb, textfield_win);
  textfield_win->reg_event_handler(WIN_EV_MOUSE, &mouse_cb, NULL);

  scr.set_focus(textfield_win);

  /* main loop */
  scr.mainloop();

  /* deinitialize */
  Window::destroy_win(textfield_win);
  Window::destroy_win(info_win);
  Window::destroy_win(banner_win);
  Window::destroy_win(my_win);

  scr.end_screen();
  
  return 0;
}
