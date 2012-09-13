#include <aio/xirang/vfs.h>

class VfsTester
{
public:
    void test_mount(xirang::fs::IVfs& vfs);
    void test_on_empty(xirang::fs::IVfs& vfs);
    void test_modification(xirang::fs::IVfs& vfs);
    void test_readonly(xirang::fs::IVfs& vfs);
};
