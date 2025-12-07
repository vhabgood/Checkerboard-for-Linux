#ifndef PACKED_ARRAY_H
#define PACKED_ARRAY_H

#include <cstdint>
#include <vector>

namespace egdb_interface {

class Packed_array {
public:
	// Constructor: nbits specifies the number of bits each element will occupy
	Packed_array(unsigned int nbits = 1) : nbits_(nbits), num_elements_(0) {
		num_qwords_ = (nbits_ * num_elements_ + 63) / 64;
		buffer_.resize(num_qwords_);
	}

	// Destructor (default is fine as vector handles memory)
	~Packed_array() = default;

	// Copy constructor
	Packed_array(const Packed_array& other) = default;

	// Copy assignment operator
	Packed_array& operator=(const Packed_array& other) = default;

	// Move constructor
	Packed_array(Packed_array&& other) noexcept = default;

	// Move assignment operator
	Packed_array& operator=(Packed_array&& other) noexcept = default;

	// Clear the array (reset number of elements to 0)
	void clear() {
		num_elements_ = 0;
		buffer_.clear();
		num_qwords_ = 0;
	}

	// Reserve memory for a certain number of elements
	void reserve(size_t capacity) {
		size_t required_qwords = (nbits_ * capacity + 63) / 64;
		buffer_.reserve(required_qwords);
	}

	// Shrink capacity to fit current size
	void shrink_to_fit() {
		buffer_.shrink_to_fit();
	}

	// Resize the array to a specific number of elements, initializing new elements to 0
	void resize(size_t new_size) {
		num_elements_ = new_size;
		num_qwords_ = (nbits_ * num_elements_ + 63) / 64;
		buffer_.resize(num_qwords_);
	}

	// Get the number of elements in the array
	size_t size() const {
		return num_elements_;
	}

	// Get the number of bits per element
	unsigned int nbits() const {
		return nbits_;
	}

	// Access element at given index (bounds checked)
	uint32_t at(size_t index) const {
		if (index >= num_elements_) {
			// Handle out-of-bounds access, e.g., throw an exception
			// For simplicity, returning 0 or asserting here
			return 0; // Or throw std::out_of_range("Packed_array::at out of bounds");
		}
		return get_value(index);
	}

	// Access element at given index (no bounds checking)
	uint32_t operator[](size_t index) const {
		return get_value(index);
	}

	// Set element at given index (bounds checked)
	void set_at(size_t index, uint32_t value) {
		if (index >= num_elements_) {
			// Handle out-of-bounds access
			return; // Or throw std::out_of_range("Packed_array::set_at out of bounds");
		}
		set_value(index, value);
	}

	// Add an element to the end of the array
	void push_back(uint32_t value) {
		size_t old_num_qwords = num_qwords_;
		num_elements_++;
		num_qwords_ = (nbits_ * num_elements_ + 63) / 64;
		if (num_qwords_ > old_num_qwords) {
			buffer_.resize(num_qwords_); // Expand buffer if needed
		}
		set_value(num_elements_ - 1, value);
	}

	// Access raw 64-bit word at qword_index
	uint64_t raw_at(size_t qword_index) const {
		if (qword_index >= num_qwords_) {
			return 0;
		}
		return buffer_[qword_index];
	}

	// Set raw 64-bit word at qword_index
	void raw_set(size_t qword_index, uint64_t value) {
		if (qword_index < num_qwords_) {
			buffer_[qword_index] = value;
		}
	}

	// Get pointer to raw buffer
	uint64_t* raw_buf() {
		return buffer_.data();
	}

private:
	unsigned int nbits_;
	size_t num_elements_;
	size_t num_qwords_;
	std::vector<uint64_t> buffer_;

	union bytes64 {
		uint64_t all_bits;
		uint8_t eight_bits[8];
	};

	// Helper to get value at an index
	uint32_t get_value(size_t index) const {
		size_t bit_offset = index * nbits_;
		size_t qword_index = bit_offset / 64;
		unsigned int inner_bit_offset = bit_offset % 64;

		uint64_t current_qword = buffer_[qword_index];
		uint64_t value = current_qword >> inner_bit_offset;

		if (inner_bit_offset + nbits_ > 64 && (qword_index + 1) < num_qwords_) {
			// Value spans two 64-bit words
			uint64_t next_qword = buffer_[qword_index + 1];
			value |= (next_qword << (64 - inner_bit_offset));
		}

		return static_cast<uint32_t>(value & ((1ULL << nbits_) - 1));
	}

	// Helper to set value at an index
	void set_value(size_t index, uint32_t value) {
		size_t bit_offset = index * nbits_;
		size_t qword_index = bit_offset / 64;
		unsigned int inner_bit_offset = bit_offset % 64;

		uint64_t mask = (1ULL << nbits_) - 1;

		uint64_t current_qword = buffer_[qword_index];
		current_qword &= ~(mask << inner_bit_offset); // Clear old value bits
		current_qword |= (static_cast<uint64_t>(value) & mask) << inner_bit_offset; // Set new value bits
		buffer_[qword_index] = current_qword;

		if (inner_bit_offset + nbits_ > 64 && (qword_index + 1) < num_qwords_) {
			// Value spans two 64-bit words, need to update the next word
			uint64_t next_qword = buffer_[qword_index + 1];
			next_qword &= ~(mask >> (64 - inner_bit_offset)); // Clear old value bits in next word
			next_qword |= (static_cast<uint64_t>(value) & mask) >> (64 - inner_bit_offset); // Set new value bits
			buffer_[qword_index + 1] = next_qword;
		}
	}
};

} // namespace egdb_interface

#endif // PACKED_ARRAY_H
