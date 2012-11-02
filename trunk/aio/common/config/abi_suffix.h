#ifndef AIO_ABI_PREFIX_H
# error Header common/config/abi_prefix.h must only be used after common/config/abi_prefix.h
#else
#undef AIO_ABI_PREFIX_H
#endif

#ifdef AIO_HAS_ABI_HEADERS
#  include AIO_ABI_SUFFIX
#endif


