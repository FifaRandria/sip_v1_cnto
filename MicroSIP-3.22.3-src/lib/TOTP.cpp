#include "TOTP.h"

#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace TOTP {

namespace {

typedef std::vector<uint8_t> Bytes;

static uint32_t rotl32(uint32_t x, int n)
{
	return (x << n) | (x >> (32 - n));
}

struct Sha1Ctx {
	uint32_t h[5];
	uint64_t byteCount;
	uint8_t buffer[64];
	size_t bufferLen;
};

static void sha1Init(Sha1Ctx& ctx)
{
	ctx.h[0] = 0x67452301;
	ctx.h[1] = 0xEFCDAB89;
	ctx.h[2] = 0x98BADCFE;
	ctx.h[3] = 0x10325476;
	ctx.h[4] = 0xC3D2E1F0;
	ctx.byteCount = 0;
	ctx.bufferLen = 0;
}

static void sha1Transform(uint32_t state[5], const uint8_t data[64])
{
	uint32_t w[80];
	for (int i = 0; i < 16; ++i) {
		w[i] = ((uint32_t)data[i * 4] << 24) |
			((uint32_t)data[i * 4 + 1] << 16) |
			((uint32_t)data[i * 4 + 2] << 8) |
			((uint32_t)data[i * 4 + 3]);
	}
	for (int i = 16; i < 80; ++i) {
		w[i] = rotl32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	}
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
	for (int i = 0; i < 80; ++i) {
		uint32_t f, k;
		if (i < 20) { f = (b & c) | (~b & d); k = 0x5A827999; }
		else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
		else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
		else { f = b ^ c ^ d; k = 0xCA62C1D6; }
		uint32_t temp = rotl32(a, 5) + f + e + k + w[i];
		e = d; d = c; c = rotl32(b, 30); b = a; a = temp;
	}
	state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}

static void sha1Update(Sha1Ctx& ctx, const uint8_t* data, size_t len)
{
	ctx.byteCount += len;
	while (len > 0) {
		size_t n = std::min(len, sizeof(ctx.buffer) - ctx.bufferLen);
		memcpy(ctx.buffer + ctx.bufferLen, data, n);
		ctx.bufferLen += n;
		data += n;
		len -= n;
		if (ctx.bufferLen == 64) {
			sha1Transform(ctx.h, ctx.buffer);
			ctx.bufferLen = 0;
		}
	}
}

static void sha1Final(Sha1Ctx& ctx, uint8_t out[20])
{
	uint64_t bitCount = ctx.byteCount * 8;
	const uint8_t pad80 = 0x80;
	sha1Update(ctx, &pad80, 1);
	const uint8_t zero = 0x00;
	while (ctx.bufferLen != 56) {
		sha1Update(ctx, &zero, 1);
	}
	uint8_t lenbuf[8];
	for (int i = 0; i < 8; ++i) {
		lenbuf[i] = (uint8_t)(bitCount >> (56 - i * 8));
	}
	sha1Update(ctx, lenbuf, 8);
	for (int i = 0; i < 5; ++i) {
		out[i * 4] = (uint8_t)(ctx.h[i] >> 24);
		out[i * 4 + 1] = (uint8_t)(ctx.h[i] >> 16);
		out[i * 4 + 2] = (uint8_t)(ctx.h[i] >> 8);
		out[i * 4 + 3] = (uint8_t)(ctx.h[i]);
	}
}

static void sha1Hash(const uint8_t* data, size_t len, uint8_t out[20])
{
	Sha1Ctx ctx;
	sha1Init(ctx);
	sha1Update(ctx, data, len);
	sha1Final(ctx, out);
}

static Bytes hmacSha1(const Bytes& key, const Bytes& data)
{
	const size_t blockSize = 64;
	uint8_t keyBlock[blockSize] = { 0 };
	if (key.size() > blockSize) {
		sha1Hash(key.data(), key.size(), keyBlock);
	}
	else if (!key.empty()) {
		memcpy(keyBlock, key.data(), key.size());
	}
	uint8_t ipad[blockSize], opad[blockSize];
	for (size_t i = 0; i < blockSize; ++i) {
		ipad[i] = keyBlock[i] ^ 0x36;
		opad[i] = keyBlock[i] ^ 0x5C;
	}
	uint8_t inner[20];
	{
		Sha1Ctx ctx;
		sha1Init(ctx);
		sha1Update(ctx, ipad, blockSize);
		sha1Update(ctx, data.data(), data.size());
		sha1Final(ctx, inner);
	}
	uint8_t outer[20];
	{
		Sha1Ctx ctx;
		sha1Init(ctx);
		sha1Update(ctx, opad, blockSize);
		sha1Update(ctx, inner, sizeof(inner));
		sha1Final(ctx, outer);
	}
	return Bytes(outer, outer + sizeof(outer));
}

Bytes decodeBase32(const std::string& input)
{
	const char base32Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
	Bytes buffer;
	int bitsLeft = 0;
	uint32_t currentByte = 0;
	for (size_t i = 0; i < input.size(); ++i) {
		char c = input[i];
		if (c == ' ' || c == '-' || c == '=') continue;
		if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
		const char* p = strchr(base32Chars, c);
		if (!p) {
			return Bytes();
		}
		uint32_t val = (uint32_t)(p - base32Chars);
		currentByte = (currentByte << 5) | val;
		bitsLeft += 5;
		if (bitsLeft >= 8) {
			buffer.push_back((uint8_t)((currentByte >> (bitsLeft - 8)) & 0xFF));
			bitsLeft -= 8;
		}
	}
	return buffer;
}

std::string totpFromCounter(const Bytes& key, uint64_t counter, int digits)
{
	Bytes msg(8);
	for (int i = 7; i >= 0; --i) {
		msg[i] = (uint8_t)(counter & 0xFF);
		counter >>= 8;
	}
	Bytes hash = hmacSha1(key, msg);
	int offset = hash[hash.size() - 1] & 0x0F;
	uint32_t binary = ((hash[offset] & 0x7F) << 24) |
		((hash[offset + 1] & 0xFF) << 16) |
		((hash[offset + 2] & 0xFF) << 8) |
		(hash[offset + 3] & 0xFF);
	uint32_t otp = binary % (uint32_t)std::pow(10.0, digits);
	std::ostringstream ss;
	ss << std::setw(digits) << std::setfill('0') << otp;
	return ss.str();
}

}

std::string generate(const std::string& secretBase32, uint64_t timeStep, int digits)
{
	Bytes key = decodeBase32(secretBase32);
	uint64_t counter = (uint64_t)std::time(NULL) / timeStep;
	return totpFromCounter(key, counter, digits);
}

bool verify(const std::string& secretBase32, const std::string& code, uint64_t timeStep, int digits, int window)
{
	Bytes key = decodeBase32(secretBase32);
	if (key.empty()) {
		return false;
	}
	std::string clean;
	for (size_t i = 0; i < code.size(); ++i) {
		char c = code[i];
		if (c == ' ' || c == '-' || c == '\t' || c == '\r' || c == '\n') continue;
		clean += c;
	}
	if (clean.size() != (size_t)digits) {
		return false;
	}
	uint64_t counter = (uint64_t)std::time(NULL) / timeStep;
	for (int64_t c = (int64_t)counter - window; c <= (int64_t)counter + window; ++c) {
		if (c < 0) continue;
		if (totpFromCounter(key, (uint64_t)c, digits) == clean) {
			return true;
		}
	}
	return false;
}

}
