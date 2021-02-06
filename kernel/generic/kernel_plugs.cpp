#include <filesystem/userspace_fs.h>
#include <physical.h>
#include <syscall.h>
#include <system_plug.h>
namespace plug
{

    uintptr_t allocate_page(size_t count)
    {

        return reinterpret_cast<uintptr_t>(pmm_alloc(count));
    }
    bool free_page(uintptr_t addr, size_t count)
    {

        pmm_free(reinterpret_cast<void *>(addr), count);
        return true;
    }
    void debug_out(const char *str, size_t length)
    {
        debug_device *dev = find_device<debug_device>();
        dev->echo_out(str, length);
    }

    int open(const char *path_name, int flags, int mode)
    {
        return fs_open(path_name, flags, mode);
    }
    int close(int fd)
    {
        return fs_close(fd);
    }
    size_t lseek(int fd, size_t offset, int whence)
    {
        return fs_lseek(fd, offset, whence);
    }
    size_t read(int fd, void *buffer, size_t count)
    {
        return fs_read(fd, buffer, count);
    }
} // namespace plug
