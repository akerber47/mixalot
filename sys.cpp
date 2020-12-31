#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "sys.h"

void open_and_map(
    std::string filename,
    size_t sz,
    void *& map,
    int& fd) {
  const char *path = filename.c_str();
  fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    throw Sys_error(errno);
  }
  // Need to make sure any created core file is the correct size.
  // Otherwise, touching the mmap will raise SIGBUS
  if (ftruncate(fd, sz) == -1) {
    throw Sys_error(errno);
  }
  map = mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    close(fd);
    throw Sys_error(errno);
  }
}

void unmap_and_close(void *map, size_t sz, int fd) {
  msync(map, sz, MS_SYNC);
  munmap(map, sz);
  close(fd);
}

int open_and_resize(std::string filename, size_t sz) {
  const char *path = filename.c_str();
  int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    throw Sys_error(errno);
  }
  if (ftruncate(fd, sz) == -1) {
    throw Sys_error(errno);
  }
  return fd;
}

int open_append(std::string filename) {
  const char *path = filename.c_str();
  int fd = open(path, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    throw Sys_error(errno);
  }
  return fd;
}

int seek_read(int fd, void *buf, int off, size_t sz) {
  if (off >= 0) {
    if(lseek(fd, (off_t) off, SEEK_SET) == (off_t) -1)
      throw Sys_error(errno);
  }
  ssize_t ret = read(fd, buf, sz);
  if (ret == -1)
    throw Sys_error(errno);
  return ret;
}

int seek_write(int fd, void *buf, int off, size_t sz) {
  if (off >= 0) {
    if(lseek(fd, (off_t) off, SEEK_SET) == (off_t) -1)
      throw Sys_error(errno);
  }
  ssize_t ret = write(fd, buf, sz);
  if (ret == -1)
    throw Sys_error(errno);
  return ret;
}
