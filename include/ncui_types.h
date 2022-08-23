/**
 * @file ncui_types.h
 * @author notweerdmonk
 * @brief Declarations for types used by the library.
 */

#ifndef NCUI_TYPES_H
#define NCUI_TYPES_H

namespace ncui {

  typedef void* base_cb_data_t;
  
  typedef void* (*base_cb_t)(base_cb_data_t);
  
  typedef base_cb_data_t scr_cb_data_t;
  
  typedef base_cb_t scr_cb_t;
  
  typedef base_cb_data_t win_cb_data_t;
  
  typedef base_cb_t win_cb_t;
  
  typedef base_cb_data_t win_ev_cb_data_t;
  
  typedef base_cb_data_t win_ev_user_data_t;
  
  typedef void* (*win_ev_cb_t)(win_ev_cb_data_t, win_ev_user_data_t);
}

#endif /* NCUI_TYPES_H */
