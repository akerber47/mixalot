#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int open_and_map(std::string filename, size_t sz, void *& map, int& fd) {
  const char *path = filename.c_str();
  fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    return -1;
  }
  map = mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (map == MAP_FAILED) {
    close(fd);
    return -1;
  }
  return 0;
}

int unmap_and_close(void *map, size_t sz, int fd) {
  if (munmap(map, sz) == -1) {
    return -1;
  }
  return close(fd);
}
