/**
 * @file ncui_field_buffer.h
 * @author notweerdmonk
 * @brief Line buffer and field buffer for text field.
 */

#ifndef NCUI_FIELD_BUFFER_H
#define NCUI_FIELD_BUFFER_H

namespace ncui {

  /**
   * @brief A struct to store and manipulate a single line of text.
   */
  typedef struct line_buffer {

    int len;
    int key;
    int idx;
    char *str;

    /**
     * @brief Constructor.
     * Create a new line_buffer object of required size.
     * @param _len The size of the line_buffer.
     */
    line_buffer(int _len) : len(_len), idx(0), key(0) {
      str = new char[len];
      memset(str, 0, len);
    }

    /**
     * @brief Destructor.
     * Destroy a line_buffer object.
     */
    ~line_buffer() {
      delete[] str;
    }

    /**
     * @brief Move the cursor to required position.
     * @param n Zero indexed position.
     */
    void move(int n) {
      if ( (n > -1) &&
           (n < len) ) {
        idx = n;
      }
    }

    /**
     * @brief Move the cursor relative to its current position.
     * @param offset Offset, negative or positive.
     */
    void move_rel(int offset) {
      if ( ((idx + offset) > -1) &&
           ((idx + offset) < len) ) {
        idx += offset;
      }
    }

    /**
     * @brief Insert a character at current position if the buffer is not full.
     * @param c The character to insert.
     * @return True of false whether the character was inserted.
     */
    bool put(char c) {
      if (idx < len) {
        str[idx++] = c;
      }
      return (idx < len);
    }

    /**
     * @brief Emulate a backspace character.
     * Write a null character at the current position and decrement the cursor.
     */
    void bksp() {
      if (idx > 0) {
        str[--idx] = '\0';
      }
    }

  } line_buf_t;

  /**
   * @brief A struct to store and manipulate multiple line_buffer objects.
   */
  typedef struct field_buffer {

    int num_cols;
    int num_rows;
    int idx;
    line_buf_t **rows;

    /**
     * @brief Constructor.
     * Create a new field_buffer object containing required number of
     * line_buffer objects, each of required size.
     * @param _rows Number of line_buffer objects.
     * @param _cols Size of each line_buffer.
     */
    field_buffer(int _rows, int _cols) : num_rows(_rows), num_cols(_cols), idx(0) {
      rows = new line_buf_t*[num_rows];
      for (int i = 0; i < num_rows; i++) {
        rows[i] = new line_buf_t(num_cols);
      }
    }

    /**
     * @brief Destructor.
     * Destory a field_buffer object.
     */
    ~field_buffer() {
      for (int i = 0; i < num_rows; i++) {
        delete rows[i];
      }
      delete[] rows;
    }

    /**
     * @brief Move the cursor to required row (select the required line_buffer
     * object).
     * @param row Zero indexed row position.
     */
    void move_row(int row) {
      if ( (row > -1) &&
           (row < num_rows) ) {
        idx = row;
      }
    }

    /**
     * @brief Move the cursor to required row relative to its current position.
     * @param offset Row offset, negative or positive.
     */
    void move_row_rel(int offset) {
      if ( ((idx + offset) > -1) &&
           ((idx + offset) < num_rows) ) {
        idx += offset;
      }
    }

    /**
     * @brief Move the cursor to required position in the line_buffer object.
     * @param col Zero indexed column position.
     */
    void move_col(int col) {
      rows[idx]->move(col);
    }

    /**
     * @brief Move the cursor to required position in the line_buffer object
     * relative to its current position.
     * @param offset Column offset, negative or positive.
     */
    void move_col_rel(int offset) {
      rows[idx]->move_rel(offset);
    }

    /**
     * @brief Move the cursor to required row and column.
     * @param row Zero indexed row position.
     * @param col Zero indexed column position.
     */
    void move(int row, int col) {
      move_row(row);
      move_col(col);
    }

    /**
     * @brief Move the cursor to relative row and column position.
     * @param i Row offset, negative or positive.
     * @param j Column offset, negative or positive.
     */
    void move_rel(int i, int j) {
      move_row_rel(i);
      move_col_rel(j);
    }

    /**
     * @brief Insert a character at current position if the buffer is not full.
     * @param c The character to insert.
     * @return True of false whether the character was inserted.
     */
    bool put(char c) {
      if (idx < num_rows) {
        if (rows[idx]->put(c) == false) {
          newline();
        }
      }
      return (idx < num_rows);
    }

    /**
     * @brief Emulates a newline character.
     * Decrements the cursor by one row.
     */
    void newline() {
      if (++idx == num_rows) {
        --idx;
      }
    }

    /**
     * @brief Emulate a backspace character.
     * Write a null character at the current position and decrement the cursor.
     */
    void bksp() {
      if (idx > 0) {
        if (idx < num_rows)
          rows[idx]->bksp();
        else
          rows[--idx]->bksp();
      }
    }
  } field_buf_t;

}

#endif /* NCUI_FIELD_BUFFER_H */
