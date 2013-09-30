#include <xirang/vfs.h>

class VfsTester
{
public:
    void test_mount(xirang::vfs::IVfs& vfs);
    void test_on_empty(xirang::vfs::IVfs& vfs);
    void test_modification(xirang::vfs::IVfs& vfs);
    void test_readonly(xirang::vfs::IVfs& vfs);
};
