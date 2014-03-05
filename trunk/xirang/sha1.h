// boost/uuid/sha1.hpp header file  ----------------------------------------------//

// Copyright 2007 Andy Tompkins.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Revision History
//  29 May 2007 - Initial Revision
//  25 Feb 2008 - moved to namespace boost::uuids::detail
//  10 Jan 2012 - can now handle the full size of messages (2^64 - 1 bits)

// This is a byte oriented implementation

#ifndef XIRANG_BOOST_UUID_SHA1_H
#define XIRANG_BOOST_UUID_SHA1_H

#include <xirang/context_except.h>
#include <xirang/operators.h>
#include <xirang/io.h>

#include <ostream>

namespace xirang {

class sha1;
AIO_EXCEPTION_TYPE(sha1_runtime_exception);

struct sha1_digest{

	sha1_digest();
	explicit sha1_digest(const_range_string d);
	string to_string() const;

	bool operator==(const sha1_digest& rhs) const;
	bool operator<(const sha1_digest& rhs) const;
	uint32_t v[5];
};
inline bool is_empty(const sha1_digest& dig){
		for (auto i : dig.v) if (i != 0) return false;
		return true;
}

extern bool is_valid_sha1_str(const_range_string d);

struct sha1_digest_compare_ : totally_ordered<sha1_digest>{};

struct hash_sha1{
	size_t operator()(const sha1_digest& d) const{
		return sizeof(size_t) == 8 ? (std::size_t(d.v[0]) << 32) + d.v[1] : d.v[0];
	}
};

AIO_EXCEPTION_TYPE(bad_sha1_string_exception);

inline std::ostream& operator<<(std::ostream& outs, const sha1_digest& dig){
	return outs << dig.to_string();
}

class sha1
{
public:
    sha1();
	// io::writer
	range<const byte*> write(const range<const byte*>& r);
	bool writable() const;
	void sync();

    void reset();

    void process_byte(byte b);

    sha1_digest get_digest() const;
	struct sha1_data{
		sha1_digest h_;
		uint8_t block_[64];
		std::size_t block_byte_index_;
		std::size_t bit_count_low;
		std::size_t bit_count_high;
	};

private:
	sha1_data data;

};


template<typename Ar>
sha1_digest calculate_sha1(Ar& ar, long_size_t max_size  = ~0){
	sha1 sha;
	io::copy_data(ar, sha, max_size);
	return sha.get_digest();
}


} // namespace xirang

#endif

