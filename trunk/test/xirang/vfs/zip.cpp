#include "../precompile.h"
#if 0
#include <xirang/io/memory.h>
#include <xirang/vfs/zip.h>
#include <xirang/vfs/inmemory.h>
#include <xirang/io/adaptor.h>

#include "./vfs.h"

BOOST_AUTO_TEST_SUITE(vfs_suite)
BOOST_AUTO_TEST_CASE(zipfs_case)
{
    using namespace xirang::fs;
    using namespace xirang;
    using xirang::io::open_flag;

    xirang::io::mem_read_write_archive ar;
    {
        InMemory cache;
        ZipFs vfs(ar, cache, "test zipfs");

        xirang::any ret = vfs.getopt(vo_readonly);
        BOOST_CHECK(!ret.empty() && !xirang::any_cast<bool>(ret));

        VfsTester tester;
        tester.test_mount(vfs);
        tester.test_on_empty(vfs);
        tester.test_modification(vfs);
    }
    {
        using namespace xirang::io;
        typedef multiplex_deletor<multiplex_reader<multiplex_random<multiplex_base<reader, xirang::io::random, xirang::ideletor> > > > multiplex_read_archive;
        multiplex_read_archive ar2(&ar);
        InMemory cache;
        ZipFs vfs(ar2, cache);

        xirang::any ret = vfs.getopt(vo_readonly);
        BOOST_CHECK(!ret.empty() && xirang::any_cast<bool>(ret));

        VfsTester tester;
        tester.test_readonly(vfs);
    }

}


BOOST_AUTO_TEST_SUITE_END()

#endif
