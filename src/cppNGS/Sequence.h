#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "cppNGS_global.h"
#include <QByteArray>
#include <random>

///DNA sequence class
class CPPNGSSHARED_EXPORT Sequence
	: public QByteArray
{
public:

	//Constructors
	Sequence();
	Sequence(const char* rhs);
	Sequence(const Sequence& rhs);
	Sequence(const QByteArray& rhs);

	//Returns the left n bases of the sequence
	Sequence left(int n) const;

	///Changes the sequence to reverse order.
	void reverse();
	///Changes the sequence to the complement.
	void complement();
	///Changes the sequence to the reverse complement.
	void reverseComplement();

	///Returns the reverse complement of the sequence.
	Sequence toReverseComplement() const;

	///Returns the complementary base of the given base.
	static char complement(char base);

	///Adds random noise to the sequence. Returns how many errors have been added.
	int addNoise(double error_probabilty, std::mt19937& gen);
};

#endif // SEQUENCE_H
