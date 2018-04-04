#include "BitSet.h"

#include <cstdio>

#define BITS_PER_WORD 32
#define BITS64_PER_WORD 64

size_t MinLen(size_t len) {
	return len / BITS_PER_WORD + ((len % BITS_PER_WORD) ? 1 : 0);
}

BitSet::BitSet(size_t len, bool isSet): data(MinLen(len), isSet ? 0xFFFFFFFF : 0) {}


BitSet::~BitSet() {}

bool BitSet::operator[](size_t index) const {
	size_t i = index / BITS_PER_WORD;
	size_t j = index % BITS_PER_WORD;
	return (data[i] >> j) & 1;
}

void BitSet::Set(size_t index) {
	size_t i = index / BITS_PER_WORD;
	size_t j = index % BITS_PER_WORD;
	data[i] |= (1 << j);
}

void BitSet::Clear(size_t index) {
	size_t i = index / BITS_PER_WORD;
	size_t j = index % BITS_PER_WORD;
	data[i] &= ~(1 << j);
}

BitSet BitSet::operator&(const BitSet& other) const {
	BitSet result(data.size() * BITS_PER_WORD);
	for (size_t i = 0; i < data.size(); i++) {
		result.data[i] = data[i] & other.data[i];
	}
	return result;
}
BitSet BitSet::operator|(const BitSet& other) const {
	BitSet result(data.size() * BITS_PER_WORD);
	for (size_t i = 0; i < data.size(); i++) {
		result.data[i] = data[i] | other.data[i];
	}
	return result;
}

BitSet& BitSet::operator&=(const BitSet& other) {
	for (size_t i = 0; i < data.size(); i++) {
		data[i] &= other.data[i];
	}
	return *this;
}
BitSet& BitSet::operator|=(const BitSet& other) {
	for (size_t i = 0; i < data.size(); i++) {
		data[i] |= other.data[i];
	}
	return *this;
}

inline size_t FindJ(uint32_t word) {
	uint32_t bit = word & -word;
	size_t j = 0;
	while (bit) {
		bit >>= 1;
		j++;
	}
	return j - 1;
}

size_t BitSet::FindFirst() const {
	for (size_t i = 0; i < data.size(); i++) {
		if (data[i]) {
			size_t j = FindJ(data[i]);
			return i * BITS_PER_WORD + j;
		}
	}
	return data.size() * BITS_PER_WORD; // Past end
}

size_t BitSet::FindNext(size_t index) const {
	size_t i = index / BITS_PER_WORD;
	size_t j0 = index % BITS_PER_WORD;
	size_t j1 = FindJ(data[i] & (0xFFFFFFFE << j0));
	if (j1 < BITS_PER_WORD) return i * BITS_PER_WORD + j1;
	for (i++; i < data.size(); i++) {
		if (data[i]) {
			size_t j = FindJ(data[i]);
			return i * BITS_PER_WORD + j;
		}
	}
	return data.size() * BITS_PER_WORD; // Past end
}

void BitSet::Print() const {
	printf("[%lu]: ", data.size());
	for (auto w : data) {
		printf("%x ", w);
	}
	printf("\n");
}


// ********
// BitSet64
// ********

BitSet64::BitSet64(bool isSet) : data(isSet ? 0xFFFFFFFF : 0) {}


BitSet64::~BitSet64() {}

bool BitSet64::operator[](size_t index) const {
	return (data >> index) & 1;
}

void BitSet64::Set(size_t index) {
	data |= (1ull << index);
}

void BitSet64::Clear(size_t index) {
	data &= ~(1ull << index);
}

BitSet64 BitSet64::operator&(const BitSet64& other) const {
	BitSet64 result;
	result.data = data & other.data;
	return result;
}
BitSet64 BitSet64::operator|(const BitSet64& other) const {
	BitSet64 result;
	result.data = data | other.data;
	return result;
}

BitSet64& BitSet64::operator&=(const BitSet64& other) {
	data &= other.data;
	return *this;
}
BitSet64& BitSet64::operator|=(const BitSet64& other) {
	data |= other.data;
	return *this;
}

inline size_t FindJ64(uint64_t data) {
	uint64_t bit = data & -data;
	size_t j = 0;
	while (bit) {
		bit >>= 1;
		j++;
	}
	return j - 1;
}

size_t BitSet64::FindFirst() const {
	return FindJ64(data);
}

size_t BitSet64::FindNext(size_t index) const {
	return FindJ(data & (0xFFFFFFFE << index));
}

void BitSet64::Print() const {
	printf("%lx ", data);
	printf("\n");
}

