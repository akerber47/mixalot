#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "io.h"

void open_and_map(std::string filename, size_t sz, void *& map, int& fd) {
  const char *path = filename.c_str();
  fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    throw Sys_error(errno);
  }
  map = mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (map == MAP_FAILED) {
    close(fd);
    throw Sys_error(errno);
  }
}

void unmap_and_close(void *map, size_t sz, int fd) {
  munmap(map, sz);
  close(fd);
}
