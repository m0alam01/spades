/*
 * read.hpp
 *
 *  Created on: 29.03.2011
 *      Author: vyahhi
 */

#ifndef READ_HPP_
#define READ_HPP_

#include "quality.hpp"
#include "sequence.hpp"
#include "nucl.hpp"
#include <string>
#include <iostream>
#include "simple_tools.hpp"
#include "sequence_tools.hpp"
using namespace std;

class Read {
public:
	static const int PHRED_OFFSET = 33;
	static const int BAD_QUALITY_THRESHOLD = 2;

	bool isValid() const {
		return valid;
	}

	Sequence getSequence() const {
		assert(valid);
		return Sequence(seq_);
	}
	Sequence getSubSequence(size_t start, size_t length) const {
		assert(length > 0 && start >= 0 && start + length <= seq_.size());
		return Sequence(seq_.substr(start, length));
	}

	Quality getQuality() const {
		assert(valid);
		return Quality(qual_);
	}

	const string& getSequenceString() const {
		return seq_;
	}

	const string& getQualityString() const {
		return qual_;
	}

	string getPhredQualityString(int offset = PHRED_OFFSET) const {
		string res = qual_;
		for (size_t i = 0; i < res.size(); ++i) {
			res[i] += offset;
		}
		return res;
	}

	const string& getName() const {
		return name_;
	}

	size_t size() const {
		return seq_.size();
	}

	char operator[](size_t i) const {
		assert(is_nucl(seq_[i]));
		return dignucl(seq_[i]);
	}

	/**
	 * It's actually not trim Ns, but trim everything before first 'N'
	 * P.S. wtf? (Kolya)
	 */
	void trimNs() __attribute__ ((deprecated)) {
		size_t index = seq_.find('N');
		if (index != string::npos) {
			seq_.erase(seq_.begin() + index, seq_.end());
			qual_.erase(qual_.begin() + index, qual_.end());
		}
		valid = updateValid();
	}

	/**
	 * trim bad quality nucleotides from start and end of the read
	 * @return size of the read left
	 */
	size_t trimBadQuality() __attribute__ ((deprecated)) {
		size_t start = 0;
		for (; start < seq_.size(); ++start) {
			if (qual_[start] > BAD_QUALITY_THRESHOLD)
				break;
		}
		if (start != seq_.size()) {
			seq_.erase(seq_.begin(), seq_.begin() + start);
			qual_.erase(qual_.begin(), qual_.begin() + start);
			size_t end = seq_.size();
			for (; end > 0; --end) {
				if (qual_[end] > BAD_QUALITY_THRESHOLD)
					break;
			}
			seq_.erase(seq_.begin() + end + 1, seq_.end());			
			qual_.erase(qual_.begin() + end + 1, qual_.end());
			valid = updateValid();
			return seq_.size();
		} else {
			seq_ = "";
			qual_ = "";
			valid = updateValid();
			return 0;
		}
	}
	/**
	 * @param k k as in k-mer
	 * @param start start point
	 * @return the first starting point of a valid k-mer >=start; return -1 if no such place exists
	 */
	int firstValidKmer(size_t start, size_t k) const __attribute__ ((deprecated)) {
		size_t curHypothesis = start;
		size_t i = start;
		for (; i < seq_.size(); ++i) {
			if (i > k + curHypothesis)
				return curHypothesis;
			if (!is_nucl(seq_[i])) {
				curHypothesis = i + 1;
			}
		}
		if (i >= k + curHypothesis) {
			return curHypothesis;
		}
		return -1;
	}

	Read() :
		valid(false) {
		;
	}

	Read(const string &name, const string &seq, const string &qual) :
		name_(name), seq_(seq), qual_(qual) { // for test only!
		valid = updateValid();
	}
private:
	string name_;
	string seq_;
	string qual_;
	bool valid;
	friend class ireadstream;
	void setName(const char* s) {
		name_ = s;
	}
	void setQuality(const char* s, int offset = PHRED_OFFSET) {
		qual_ = s;
		for (size_t i = 0; i < qual_.size(); ++i) {
			qual_[i] -= offset;
		}
	}
	void setSequence(const char* s) {
		seq_ = s;
		valid = updateValid();
	}
	const bool updateValid() const {
		if (seq_.size() == 0) {
			return false;
		}
		for (size_t i = 0; i < seq_.size(); ++i) {
			if (!is_nucl(seq_[i])) {
				return false;
			}
		}
		return true;
	}

public:
	Read operator!() const {
		string newName;
		if (name_ == "" || name_[0] != '!') {
			newName = '!' + name_;
		} else {
			newName = name_.substr(1, name_.length());
		}
		return Read(newName, ReverseComplement(seq_), Reverse(qual_));
	}
};

// todo: put this to *.cpp
//ostream& operator<<(ostream& os, const Read& read) {
//	return os << read.getSequenceString();
//}

#endif /* READ_HPP_ */
