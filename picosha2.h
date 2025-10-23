/*
picosha2: 2014-02-13

The MIT License (MIT)

Copyright (c) 2014 okdshin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef PICOSHA2_H
#define PICOSHA2_H
//picosha2:20140213
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

namespace picosha2
{
typedef unsigned long word_t;
typedef unsigned char byte_t;

static const size_t k_digest_size = 32;

namespace detail
{
inline byte_t mask_8bit(byte_t x)
{
	return x & 0xff;
}

inline word_t mask_32bit(word_t x)
{
	return x & 0xffffffff;
}

const word_t add_constant[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

const word_t initial_message_digest[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

inline word_t ch(word_t x, word_t y, word_t z)
{
	return (x & y) ^ ((~x) & z);
}

inline word_t maj(word_t x, word_t y, word_t z)
{
	return (x & y) ^ (x & z) ^ (y & z);
}

inline word_t rotr(word_t x, std::size_t n)
{
	return mask_32bit((x >> n) | (x << (32 - n)));
}

inline word_t bsig0(word_t x)
{
	return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline word_t bsig1(word_t x)
{
	return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline word_t shr(word_t x, std::size_t n)
{
	return x >> n;
}

inline word_t ssig0(word_t x)
{
	return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
}

inline word_t ssig1(word_t x)
{
	return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
}

template<typename RaIter>
void hash256_block(RaIter first, RaIter last, word_t* message_digest)
{
	word_t w[64];
	std::fill(w, w + 64, 0);
	for (std::size_t i = 0; i < 16; ++i) {
		w[i] = mask_32bit(static_cast<word_t>(first[i*4]) << 24 | static_cast<word_t>(first[i*4+1]) << 16 | static_cast<word_t>(first[i*4+2]) << 8 | static_cast<word_t>(first[i*4+3]));
	}
	for (std::size_t i = 16; i < 64; ++i) {
		w[i] = mask_32bit(ssig1(w[i - 2]) + w[i - 7] + ssig0(w[i - 15]) + w[i - 16]);
	}

	word_t a = message_digest[0];
	word_t b = message_digest[1];
	word_t c = message_digest[2];
	word_t d = message_digest[3];
	word_t e = message_digest[4];
	word_t f = message_digest[5];
	word_t g = message_digest[6];
	word_t h = message_digest[7];

	for (std::size_t i = 0; i < 64; ++i) {
		word_t temp1 = h + bsig1(e) + ch(e, f, g) + add_constant[i] + w[i];
		word_t temp2 = bsig0(a) + maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = mask_32bit(d + temp1);
		d = c;
		c = b;
		b = a;
		a = mask_32bit(temp1 + temp2);
	}
	message_digest[0] += a;
	message_digest[1] += b;
	message_digest[2] += c;
	message_digest[3] += d;
	message_digest[4] += e;
	message_digest[5] += f;
	message_digest[6] += g;
	message_digest[7] += h;
	for (std::size_t i = 0; i < 8; ++i) {
		message_digest[i] = mask_32bit(message_digest[i]);
	}
}

} // namespace detail

template<typename InIter>
void output_hex(InIter first, InIter last, std::ostream& os)
{
	os.setf(std::ios::hex, std::ios::basefield);
	while (first != last) {
		os.width(2);
		os.fill('0');
		os << static_cast<unsigned int>(*first);
		++first;
	}
	os.setf(std::ios::dec, std::ios::basefield);
}

template<typename InIter>
void bytes_to_hex_string(InIter first, InIter last, std::string& hex_str)
{
	std::ostringstream oss;
	output_hex(first, last, oss);
	hex_str.assign(oss.str());
}

template<typename InContainer>
void bytes_to_hex_string(const InContainer& bytes, std::string& hex_str)
{
	bytes_to_hex_string(bytes.begin(), bytes.end(), hex_str);
}

template<typename InIter>
std::string bytes_to_hex_string(InIter first, InIter last)
{
	std::string hex_str;
	bytes_to_hex_string(first, last, hex_str);
	return hex_str;
}

template<typename InContainer>
std::string bytes_to_hex_string(const InContainer& bytes)
{
	std::string hex_str;
	bytes_to_hex_string(bytes, hex_str);
	return hex_str;
}

class hash256_one_by_one
{
public:
	hash256_one_by_one()
	{
		init();
	}

	void init()
	{
		buffer_.clear();
		std::copy(detail::initial_message_digest, detail::initial_message_digest + 8, message_digest_);
	}

	template<typename RaIter>
	void process(RaIter first, RaIter last)
	{
		add_to_buffer(first, last);
		process_buffer();
	}

	void finish()
	{
		byte_t len[8];
		pad_buffer(len);
		process(len, len + 8);
	}

	template<typename OutIter>
	void get_hash_bytes(OutIter first, OutIter last)const
	{
		for(const word_t* iter = message_digest_; iter != message_digest_ + 8; ++iter) {
			for(std::size_t i = 0; i < 4 && first != last; ++i) {
				*(first++) = detail::mask_8bit(static_cast<byte_t>((*iter >> (24 - 8 * i))));
			}
		}
	}

private:
	void add_to_buffer(const byte_t* first, const byte_t* last)
	{
		std::copy(first, last, std::back_inserter(buffer_));
	}

	template<typename RaIter>
	void add_to_buffer(RaIter first, RaIter last)
	{
		while(first != last){
			buffer_.push_back(*first++);
		}
	}

	void process_buffer()
	{
		std::vector<byte_t>::iterator first = buffer_.begin();
		while(buffer_.end() - first >= 64){
			detail::hash256_block(first, first + 64, message_digest_);
			first += 64;
		}
		buffer_.erase(buffer_.begin(), first);
	}

	void pad_buffer(byte_t* len)
	{
		std::size_t msg_bits = buffer_.size() * 8;
		for(int i = 0; i < 8; ++i){
			len[i] = detail::mask_8bit(static_cast<byte_t>(msg_bits >> (56 - 8 * i)));
		}

		buffer_.push_back(0x80);
		while((buffer_.size() % 64) != 56){
			buffer_.push_back(0);
		}
	}

	word_t message_digest_[8];
	std::vector<byte_t> buffer_;
};

template<typename RaIter>
void calc_hash(RaIter first, RaIter last, byte_t* hash)
{
	hash256_one_by_one hasher;
	hasher.process(first, last);
	hasher.finish();
	hasher.get_hash_bytes(hash, hash + k_digest_size);
}

template<typename RaIter>
void hash256_hex_string(RaIter first, RaIter last, std::string& hex_str)
{
	byte_t hash[k_digest_size];
	calc_hash(first, last, hash);
	bytes_to_hex_string(hash, hash + k_digest_size, hex_str);
}

template<typename RaIter>
std::string hash256_hex_string(RaIter first, RaIter last)
{
	std::string hex_str;
	hash256_hex_string(first, last, hex_str);
	return hex_str;
}

inline void hash256_hex_string(const std::string& src, std::string& hex_str)
{
	hash256_hex_string(src.begin(), src.end(), hex_str);
}

inline std::string hash256_hex_string(const std::string& src)
{
	return hash256_hex_string(src.begin(), src.end());
}

template<typename InContainer>
void hash256_hex_string(const InContainer& src, std::string& hex_str)
{
	hash256_hex_string(src.begin(), src.end(), hex_str);
}

template<typename RaIter>
void hash256_bytes(RaIter first, RaIter last, std::vector<byte_t>& hash)
{
	hash.resize(k_digest_size);
	calc_hash(first, last, &hash[0]);
}

template<typename InContainer>
void hash256_bytes(const InContainer& src, std::vector<byte_t>& hash)
{
	hash256_bytes(src.begin(), src.end(), hash);
}

} // namespace picosha2

#endif //PICOSHA2_H
