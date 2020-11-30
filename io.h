#include <string>

/*
 * Open the given filename as a memory mapped file. If the file
 * doesn't already exist, create it.
 * Return 0 on success, and -1 on failure.
 * Errors stored in errno.
 * Output stored in map (pointer to memory map) and fd (file descriptor).
 *
 * Used for the memory mapped MIX core.
 */
int open_and_map(std::string filename, size_t sz, void *& map, int& fd);

/*
 * Inverse of the above operation, for cleanup.
 */
int unmap_and_close(void *map, size_t sz, int fd);
