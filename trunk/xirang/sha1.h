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
#include <ostream>

namespace xirang {

namespace {
	inline uint32_t left_rotate(uint32_t x, std::size_t n)
	{
		return (x<<n) ^ (x>> (32-n));
	}
}
class sha1;
AIO_EXCEPTION_TYPE(sha1_runtime_exception);

struct sha1_digest : totally_ordered<sha1_digest>{

	sha1_digest(){
		for (auto& i : v)
			i = 0;
	}
	explicit sha1_digest(const_range_string d);

	string to_string() const;

	bool operator==(const sha1_digest& rhs) const{
		for (int i = 0; i != 5; ++i){
			if (v[i] != rhs.v[i])	return false;
		}
		
		return true;
	}
	 bool operator<(const sha1_digest& rhs) const{
		 return v[0] < rhs.v[0]
			 || (v[0] == rhs.v[0] && v[1] < rhs.v[1])
			 || (v[1] == rhs.v[1] && v[2] < rhs.v[2])
			 || (v[2] == rhs.v[2] && v[3] < rhs.v[3])
			 || (v[3] == rhs.v[3] && v[4] < rhs.v[4]);
	 }

	uint32_t v[5];
};

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

    void reset();

    void process_byte(byte b);
	void process(range<const byte*> rng){
		process_block(rng.begin(), rng.end());
	}
    void process_block(byte const* bytes_begin, byte const* bytes_end);
    void process_bytes(byte const* buffer, std::size_t byte_count);

    const sha1_digest& get_digest();
	bool is_finished() const;

private:
    void process_block();
    void process_byte_impl(uint8_t byte);

private:
    sha1_digest h_;

    uint8_t block_[64];

    std::size_t block_byte_index_;
    std::size_t bit_count_low;
    std::size_t bit_count_high;
	bool finished;
};

inline sha1::sha1()
{
    reset();
}

inline void sha1::reset()
{
    h_.v[0] = 0x67452301;
    h_.v[1] = 0xEFCDAB89;
    h_.v[2] = 0x98BADCFE;
    h_.v[3] = 0x10325476;
    h_.v[4] = 0xC3D2E1F0;

    block_byte_index_ = 0;
    bit_count_low = 0;
    bit_count_high = 0;
	finished = false;
}

inline void sha1::process_byte(byte b)
{
	AIO_PRE_CONDITION(!finished);
    process_byte_impl(uint8_t(b));

    // size_t max value = 0xFFFFFFFF
    //if (bit_count_low + 8 >= 0x100000000) { // would overflow
    //if (bit_count_low >= 0x100000000-8) {
    if (bit_count_low < 0xFFFFFFF8) {
        bit_count_low += 8;
    } else {
        bit_count_low = 0;

        if (bit_count_high <= 0xFFFFFFFE) {
            ++bit_count_high;
        } else {
            AIO_THROW(sha1_runtime_exception)("sha1 too many bytes");
        }
    }
}

inline void sha1::process_byte_impl(uint8_t b)
{
	AIO_PRE_CONDITION(!finished);

    block_[block_byte_index_++] = uint8_t(b);

    if (block_byte_index_ == 64) {
        block_byte_index_ = 0;
        process_block();
    }
}

inline void sha1::process_block(byte const* begin, byte const* end)
{
	AIO_PRE_CONDITION(!finished);
    for(; begin != end; ++begin) {
        process_byte(*begin);
    }
}

inline void sha1::process_bytes(byte const* b, std::size_t byte_count)
{
	AIO_PRE_CONDITION(!finished);
    process_block(b, b+byte_count);
}

inline void sha1::process_block()
{
	AIO_PRE_CONDITION(!finished);
    uint32_t w[80];
    for (std::size_t i=0; i<16; ++i) {
        w[i]  = (block_[i*4 + 0] << 24);
        w[i] |= (block_[i*4 + 1] << 16);
        w[i] |= (block_[i*4 + 2] << 8);
        w[i] |= (block_[i*4 + 3]);
    }
    for (std::size_t i=16; i<80; ++i) {
        w[i] = left_rotate((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]), 1);
    }

    uint32_t a = h_.v[0];
    uint32_t b = h_.v[1];
    uint32_t c = h_.v[2];
    uint32_t d = h_.v[3];
    uint32_t e = h_.v[4];

    for (std::size_t i=0; i<80; ++i) {
        uint32_t f;
        uint32_t k;

        if (i<20) {
            f = (b & c) | (~b & d);
            k = 0x5A827999;
        } else if (i<40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i<60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        uint32_t temp = left_rotate(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = left_rotate(b, 30);
        b = a;
        a = temp;
    }

    h_.v[0] += a;
    h_.v[1] += b;
    h_.v[2] += c;
    h_.v[3] += d;
    h_.v[4] += e;
}

inline bool sha1::is_finished() const{
	return finished;
}
inline const sha1_digest& sha1::get_digest()
{
	if (finished)
		return h_;
    // append the bit '1' to the message
    process_byte_impl(0x80);

    // append k bits '0', where k is the minimum number >= 0
    // such that the resulting message length is congruent to 56 (mod 64)
    // check if there is enough space for padding and bit_count
    if (block_byte_index_ > 56) {
        // finish this block
        while (block_byte_index_ != 0) {
            process_byte_impl(0);
        }

        // one more block
        while (block_byte_index_ < 56) {
            process_byte_impl(0);
        }
    } else {
        while (block_byte_index_ < 56) {
            process_byte_impl(0);
        }
    }

    // append length of message (before pre-processing) 
    // as a 64-bit big-endian integer
    process_byte_impl( static_cast<uint8_t>((bit_count_high>>24) & 0xFF) );
    process_byte_impl( static_cast<uint8_t>((bit_count_high>>16) & 0xFF) );
    process_byte_impl( static_cast<uint8_t>((bit_count_high>>8 ) & 0xFF) );
    process_byte_impl( static_cast<uint8_t>((bit_count_high)     & 0xFF) );
    process_byte_impl( static_cast<uint8_t>((bit_count_low>>24) & 0xFF) );
    process_byte_impl( static_cast<uint8_t>((bit_count_low>>16) & 0xFF) );
    process_byte_impl( static_cast<uint8_t>((bit_count_low>>8 ) & 0xFF) );
    process_byte_impl( static_cast<uint8_t>((bit_count_low)     & 0xFF) );

    // get final digest
	finished = true;
	return h_;
}

} // namespace boost::uuids::detail

#endif
