#include <string>

/*
 * Open the given filename as a memory mapped file. If the file
 * doesn't already exist, create it.
 * Throw Sys_error on failure (containing errno).
 * Output stored in map (pointer to memory map) and fd (file descriptor).
 *
 * Used for the memory mapped MIX core.
 */
void open_and_map(std::string filename, size_t sz, void *& map, int& fd);

/*
 * Inverse of the above operation, for cleanup.
 */
void unmap_and_close(void *map, size_t sz, int fd);

/*
 * Open the given filename and set it to the given size.
 * Throw Sys_error on failure (containing errno).
 * Return the new file descriptor.
 */
int open_and_resize(std::string filename, size_t sz);

/*
 * Open the given filename with mode append.
 * Throw Sys_error on failure (containing errno).
 * Return the new file descriptor.
 */
int open_append(std::string filename);

/*
 * Seek to the given position (if not -1) and read/write
 * Throw Sys_error on failure (containing errno).
 * Return number of bytes read/written.
 */
int seek_read(int fd, void *buf, int off, size_t sz);
int seek_write(int fd, void *buf, int off, size_t sz);

/*
 * Seek to the given position and write
 * Throw Sys_error on failure (containing errno).
 * Return number of bytes written.
 */

class Sys_error {
public:
  Sys_error(int err) : err(err) {}
  int err;
};
