#include "../precompile.h"
#include <aio/common/archive/mem_archive.h>
#include <aio/xirang/vfs/zip.h>
#include <aio/xirang/vfs/inmemory.h>
#include <aio/common/io/adaptor.h>

#include "./vfs.h"

BOOST_AUTO_TEST_SUITE(vfs_suite)
BOOST_AUTO_TEST_CASE(zipfs_case)
{
    using namespace xirang::fs;
    using namespace xirang;
    using aio::io::archive_mode;
    using aio::io::open_flag;

    aio::io::mem_read_write_archive ar;
    {
        InMemory cache;
        ZipFs vfs(ar, cache, "test zipfs");

        aio::any ret = vfs.getopt(vo_readonly);
        BOOST_CHECK(!ret.empty() && !aio::any_cast<bool>(ret));

        VfsTester tester;
        tester.test_mount(vfs);
        tester.test_on_empty(vfs);
        tester.test_modification(vfs);
    }
    {
        using namespace aio::io;
        typedef multiplex_deletor<multiplex_reader<multiplex_random<multiplex_base<reader, aio::io::random, aio::ideletor> > > > multiplex_read_archive;
        multiplex_read_archive ar2(&ar);
        InMemory cache;
        ZipFs vfs(ar2, cache);

        aio::any ret = vfs.getopt(vo_readonly);
        BOOST_CHECK(!ret.empty() && aio::any_cast<bool>(ret));

        VfsTester tester;
        tester.test_readonly(vfs);
    }

}


BOOST_AUTO_TEST_SUITE_END()
