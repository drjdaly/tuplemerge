#pragma once

#include <cstdint>
#include <vector>

using std::size_t;

class BitSet {
public:
	BitSet(size_t len, bool isSet = false);
	~BitSet();

	bool operator[](size_t index) const;

	void Set(size_t index);
	void Clear(size_t index);

	BitSet operator&(const BitSet& other) const;
	BitSet operator|(const BitSet& other) const;

	BitSet& operator&=(const BitSet& other);
	BitSet& operator|=(const BitSet& other);

	size_t FindFirst() const;
	size_t FindNext(size_t index) const;

	void Print() const;
private:
	std::vector<uint32_t> data;
};

class BitSet64 {
public:
	BitSet64(bool isSet = false);
	~BitSet64();

	bool operator[](size_t index) const;

	void Set(size_t index);
	void Clear(size_t index);

	BitSet64 operator&(const BitSet64& other) const;
	BitSet64 operator|(const BitSet64& other) const;

	BitSet64& operator&=(const BitSet64& other);
	BitSet64& operator|=(const BitSet64& other);

	size_t FindFirst() const;
	size_t FindNext(size_t index) const;

	void Print() const;
private:
	uint64_t data;
};

