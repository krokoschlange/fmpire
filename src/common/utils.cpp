#include "utils.h"

#include <charconv>

namespace fmpire
{

static const char* base32_code = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
								 "012345";

static int base32_decode[256] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  26, 27, 28, 29, 30, 31, 0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0};

std::string encode_base32(const uint8_t* const data, const size_t size)
{
	std::string encoded(size * 2, '*');
	for (size_t byte = 0; byte < size; byte++)
	{
		encoded[byte * 2] = base32_code[data[byte] & 0xf];
		encoded[byte * 2 + 1] = base32_code[data[byte] >> 4];
	}
	return encoded;
}

size_t decode_base32(const std::string& encoded,
					 const size_t offset,
					 uint8_t* decoded,
					 const size_t decoded_size)
{
	size_t byte = 0;
	for (; byte < decoded_size && byte * 2 + 1 + offset < encoded.size();
		 byte++)
	{
		decoded[byte] =
			base32_decode[(uint8_t) encoded[byte * 2 + offset]]
			| (base32_decode[(uint8_t) encoded[byte * 2 + 1 + offset]] << 4);
	}
	return byte * 2;
}

} // namespace fmpire
