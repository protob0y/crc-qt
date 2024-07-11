/*
 *CRC++
Copyright (c) 2022, Daniel Bahr
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of CRC++ nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CALC_CRC_H
#define CALC_CRC_H

#include <QString>
#include <QList>
#include <QDebug>

typedef enum{
	None =				0b00000000,
	StartFrameIndi =	0b00000001,
	SyncFrameIndi =		0b00000010,
	NullFrameIndi =		0b00000100,
	PayloadPreIndi =	0b00001000,
	ReservedBit =		0b00000000
}FR_CHECK_BOX;

class calc_crc
{
public:
	calc_crc() : CAN15table(calc_crc::CRC_15()), CAN17table(calc_crc::CRC_17_CAN()), CAN21table(calc_crc::CRC_21_CAN()),
					FR11table(calc_crc::CRC_11()), FRA24table(calc_crc::CRC_24_FLEXRAYA()), FRB24table(calc_crc::CRC_24_FLEXRAYB()),
					E32table(calc_crc::CRC_32())
		{
			//Lookup tables der einzeilnen Bussysteme erstellen
			// Initialisierung der Tabelle im Konstruktor
			CAN15table = calc_crc::Table<uint16_t, 15>(calc_crc::CRC_15());
			CAN17table = calc_crc::Table<uint32_t, 17> (calc_crc::CRC_17_CAN());
			CAN21table = calc_crc::Table<std::uint32_t, 21> (calc_crc::CRC_21_CAN());
			FR11table = calc_crc::Table<std::uint16_t, 11> (calc_crc::CRC_11());
			FRA24table = calc_crc::Table<std::uint32_t, 24> (calc_crc::CRC_24_FLEXRAYA());
			FRB24table = calc_crc::Table<std::uint32_t, 24> (calc_crc::CRC_24_FLEXRAYB());
			E32table = calc_crc::Table<std::uint32_t, 32> (calc_crc::CRC_32());

		} // Konstruktor

//returns: error/success code gets: data (user input) as a QList, pointer to QList with values to write
QString LinClassic_CRC(QList<QString> data, QList<QString> * results);
QString LinEnhanced_CRC(QList<QString> data, QList<QString> * results);

QString FrHeader_CRC(QList<QString> data, uint8_t * byteArray, QList<QString> * results, uint8_t check, uint8_t* dataLength);
QString Fr_CRC(QList<QString> data, uint8_t *byteArray, QList<QString> * results, uint8_t check, uint8_t* dataLength);
uint8_t getParseValue(QString query);




/***************************************  BEGIN: aus CRCpp  *************************************/
	template <typename CRCType, uint16_t CRCWidth>
	struct Table;

	/**
		@brief CRC parameters.
	*/
	template <typename CRCType, uint16_t CRCWidth>
	struct Parameters
		{
			CRCType polynomial;   ///< CRC polynomial
			CRCType initialValue; ///< Initial CRC value
			CRCType finalXOR;     ///< Value to XOR with the final CRC
			bool reflectInput;    ///< true to reflect all input bytes
			bool reflectOutput;   ///< true to reflect the output CRC (reflection occurs before the final XOR)

			Table<CRCType, CRCWidth> MakeTable() const;
		};

	/**
		@brief CRC lookup table. After construction, the CRC parameters are fixed.
		@note A CRC table can be used for multiple CRC calculations.
	*/
	template <typename CRCType, uint16_t CRCWidth>
	struct Table
		{
			// Constructors are intentionally NOT marked explicit.
			Table(const Parameters<CRCType, CRCWidth> & parameters);


			const Parameters<CRCType, CRCWidth> & GetParameters() const;

			const CRCType * GetTable() const;

			CRCType operator[](unsigned char index) const;

		private:
			void InitTable();

			Parameters<CRCType, CRCWidth> parameters; ///< CRC parameters used to construct the table
			CRCType table[1 << CHAR_BIT];             ///< CRC lookup table
		};

	template <typename CRCType, uint16_t CRCWidth>
	static CRCType Calculate(const void * data, size_t size, const Parameters<CRCType, CRCWidth> & parameters);


	template <typename CRCType, uint16_t CRCWidth>
	static CRCType Calculate(const void * data, size_t size, const Table<CRCType, CRCWidth> & lookupTable);


	template <typename CRCType, uint16_t CRCWidth>
	static CRCType CalculateBits(const void * data, size_t size, const Parameters<CRCType, CRCWidth> & parameters);


	template <typename CRCType, uint16_t CRCWidth>
	static CRCType CalculateBits(const void * data, size_t size, const Table<CRCType, CRCWidth> & lookupTable);

	static const Parameters< uint8_t,  8> & CRC_8();			//ALLGEMEINER CRC8

	static const Parameters<uint16_t, 11> & CRC_11();			//FLEXRAY-CRC11

	static const Parameters<uint16_t, 15> & CRC_15();			// irgendein CAN
	static const Parameters<uint32_t, 17> & CRC_17_CAN();
	static const Parameters<uint32_t, 21> & CRC_21_CAN();

	static const Parameters<uint32_t, 24> & CRC_24_FLEXRAYA();
	static const Parameters<uint32_t, 24> & CRC_24_FLEXRAYB();

	static const Parameters<uint32_t, 32> & CRC_32();			//Ethernet-CRC
/***************************************  ENDE: aus CRCpp  *************************************/


private:
    void parse(QString query);

    //used by parser
    uint8_t hexCharToBin(QChar q_hex); // gets char (hex), returns bit pattern
    uint8_t binCharToBin(QChar q_bin); // gets char (bin), returns bit

    //used by synthesizer (makes bit pattern into hex or bin string, for user to read)
    QString toReadableStringBin(uint16_t input, int numberOfBits);
    QString toReadableStringHex(uint16_t input, int numberOfBits);
	QString toReadableStringBin32(uint32_t input, int numberOfBits);
	QString toReadableStringHex32(uint32_t input, int numberOfBits);
    QChar binToHexChar(uint8_t halfByte); // gets pit pattern, returns char (hex)


    //result of parse
    QList<uint8_t> parse_result;
    int parsed_bits;

	//Lookup Tabellen
	Table<uint16_t, 15> CAN15table;
	Table<uint32_t, 17> CAN17table;
	Table<uint32_t, 21> CAN21table;
	Table<uint16_t, 11> FR11table;
	Table<uint32_t, 24> FRA24table;
	Table<uint32_t, 24> FRB24table;
	Table<uint32_t, 32> E32table;

/***************************************  BEGIN: aus CRCpp  *************************************/
	template <typename IntegerType>
	static IntegerType Reflect(IntegerType value, uint16_t numBits);

	template <typename CRCType, uint16_t CRCWidth>
	static CRCType Finalize(CRCType remainder, CRCType finalXOR, bool reflectOutput);

	template <typename CRCType, uint16_t CRCWidth>
	static CRCType UndoFinalize(CRCType remainder, CRCType finalXOR, bool reflectOutput);

	template <typename CRCType, uint16_t CRCWidth>
	static CRCType CalculateRemainder(const void * data, size_t size, const Parameters<CRCType, CRCWidth> & parameters, CRCType remainder);

	template <typename CRCType, uint16_t CRCWidth>
	static CRCType CalculateRemainder(const void * data, size_t size, const Table<CRCType, CRCWidth> & lookupTable, CRCType remainder);

	template <typename CRCType, uint16_t CRCWidth>
	static CRCType CalculateRemainderBits(unsigned char byte, size_t numBits, const Parameters<CRCType, CRCWidth> & parameters, CRCType remainder);

};



/**
	@brief Returns a CRC lookup table construct using these CRC parameters.
	@note This function primarily exists to allow use of the auto keyword instead of instantiating
		a table directly, since template parameters are not inferred in constructors.
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC lookup table
*/
template <typename CRCType, uint16_t CRCWidth>
inline calc_crc::Table<CRCType, CRCWidth> calc_crc::Parameters<CRCType, CRCWidth>::MakeTable() const
{
	// This should take advantage of RVO and optimize out the copy.
	return calc_crc::Table<CRCType, CRCWidth>(*this);
}

/**
	@brief Constructs a CRC table from a set of CRC parameters
	@param[in] params CRC parameters
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
*/
template <typename CRCType, uint16_t CRCWidth>
inline calc_crc::Table<CRCType, CRCWidth>::Table(const Parameters<CRCType, CRCWidth> & params) :
	parameters(params)
{
	InitTable();
}


/**
	@brief Gets the CRC parameters used to construct the CRC table
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC parameters
*/
template <typename CRCType, uint16_t CRCWidth>
inline const calc_crc::Parameters<CRCType, CRCWidth> & calc_crc::Table<CRCType, CRCWidth>::GetParameters() const
{
	return parameters;
}

/**
	@brief Gets the CRC table
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC table
*/
template <typename CRCType, uint16_t CRCWidth>
inline const CRCType * calc_crc::Table<CRCType, CRCWidth>::GetTable() const
{
	return table;
}

/**
	@brief Gets an entry in the CRC table
	@param[in] index Index into the CRC table
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC table entry
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::Table<CRCType, CRCWidth>::operator[](unsigned char index) const
{
	return table[index];
}

/**
	@brief Initializes a CRC table.
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
*/
template <typename CRCType, uint16_t CRCWidth>
inline void calc_crc::Table<CRCType, CRCWidth>::InitTable()
{
	// For masking off the bits for the CRC (in the event that the number of bits in CRCType is larger than CRCWidth)
	static const CRCType BIT_MASK((CRCType(1) << (CRCWidth - CRCType(1))) |
										   ((CRCType(1) << (CRCWidth - CRCType(1))) - CRCType(1)));

	// The conditional expression is used to avoid a -Wshift-count-overflow warning.
	static const CRCType SHIFT((CHAR_BIT >= CRCWidth) ? static_cast<CRCType>(CHAR_BIT - CRCWidth) : 0);

	CRCType crc;
	unsigned char byte = 0;

	// Loop over each dividend (each possible number storable in an unsigned char)
	do
	{
		crc = calc_crc::CalculateRemainder<CRCType, CRCWidth>(&byte, sizeof(byte), parameters, CRCType(0));

		// This mask might not be necessary; all unit tests pass with this line commented out,
		// but that might just be a coincidence based on the CRC parameters used for testing.
		// In any case, this is harmless to leave in and only adds a single machine instruction per loop iteration.
		crc &= BIT_MASK;

		if (!parameters.reflectInput && CRCWidth < CHAR_BIT)
		{
			// Undo the special operation at the end of the CalculateRemainder()
			// function for non-reflected CRCs < CHAR_BIT.
			crc = static_cast<CRCType>(crc << SHIFT);
		}

		table[byte] = crc;
	}
	while (++byte);
}

/**
	@brief Computes a CRC.
	@param[in] data Data over which CRC will be computed
	@param[in] size Size of the data, in bytes
	@param[in] parameters CRC parameters
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::Calculate(const void * data, size_t size, const Parameters<CRCType, CRCWidth> & parameters)
{
	CRCType remainder = CalculateRemainder(data, size, parameters, parameters.initialValue);

	// No need to mask the remainder here; the mask will be applied in the Finalize() function.

	return Finalize<CRCType, CRCWidth>(remainder, parameters.finalXOR, parameters.reflectInput != parameters.reflectOutput);
}



/**
	@brief Computes a CRC via a lookup table.
	@param[in] data Data over which CRC will be computed
	@param[in] size Size of the data, in bytes
	@param[in] lookupTable CRC lookup table
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::Calculate(const void * data, size_t size, const Table<CRCType, CRCWidth> & lookupTable)
{
	const Parameters<CRCType, CRCWidth> & parameters = lookupTable.GetParameters();

	CRCType remainder = CalculateRemainder(data, size, lookupTable, parameters.initialValue);

	// No need to mask the remainder here; the mask will be applied in the Finalize() function.

	return Finalize<CRCType, CRCWidth>(remainder, parameters.finalXOR, parameters.reflectInput != parameters.reflectOutput);
}



/**
	@brief Computes a CRC.
	@param[in] data Data over which CRC will be computed
	@param[in] size Size of the data, in bits
	@param[in] parameters CRC parameters
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::CalculateBits(const void * data, size_t size, const Parameters<CRCType, CRCWidth> & parameters)
{
	CRCType remainder = parameters.initialValue;

	// Calculate the remainder on a whole number of bytes first, then call
	// a special-case function for the remaining bits.
	size_t wholeNumberOfBytes = size / CHAR_BIT;
	if (wholeNumberOfBytes > 0)
	{
		remainder = CalculateRemainder(data, wholeNumberOfBytes, parameters, remainder);
	}

	size_t remainingNumberOfBits = size % CHAR_BIT;
	if (remainingNumberOfBits != 0)
	{
		unsigned char lastByte = *(reinterpret_cast<const unsigned char *>(data) + wholeNumberOfBytes);
		remainder = CalculateRemainderBits(lastByte, remainingNumberOfBits, parameters, remainder);
	}

	// No need to mask the remainder here; the mask will be applied in the Finalize() function.

	return Finalize<CRCType, CRCWidth>(remainder, parameters.finalXOR, parameters.reflectInput != parameters.reflectOutput);
}



/**
	@brief Computes a CRC via a lookup table.
	@param[in] data Data over which CRC will be computed
	@param[in] size Size of the data, in bits
	@param[in] lookupTable CRC lookup table
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::CalculateBits(const void * data, size_t size, const Table<CRCType, CRCWidth> & lookupTable)
{
	const Parameters<CRCType, CRCWidth> & parameters = lookupTable.GetParameters();

	CRCType remainder = parameters.initialValue;

	// Calculate the remainder on a whole number of bytes first, then call
	// a special-case function for the remaining bits.
	size_t wholeNumberOfBytes = size / CHAR_BIT;
	if (wholeNumberOfBytes > 0)
	{
		remainder = CalculateRemainder(data, wholeNumberOfBytes, lookupTable, remainder);
	}

	size_t remainingNumberOfBits = size % CHAR_BIT;
	if (remainingNumberOfBits != 0)
	{
		unsigned char lastByte = *(reinterpret_cast<const unsigned char *>(data) + wholeNumberOfBytes);
		remainder = CalculateRemainderBits(lastByte, remainingNumberOfBits, parameters, remainder);
	}

	// No need to mask the remainder here; the mask will be applied in the Finalize() function.

	return Finalize<CRCType, CRCWidth>(remainder, parameters.finalXOR, parameters.reflectInput != parameters.reflectOutput);
}



/**
	@brief Reflects (i.e. reverses the bits within) an integer value.
	@param[in] value Value to reflect
	@param[in] numBits Number of bits in the integer which will be reflected
	@tparam IntegerType Integer type of the value being reflected
	@return Reflected value
*/
template <typename IntegerType>
inline IntegerType calc_crc::Reflect(IntegerType value, uint16_t numBits)
{
	IntegerType reversedValue(0);

	for (uint16_t i = 0; i < numBits; ++i)
	{
		reversedValue = static_cast<IntegerType>((reversedValue << 1) | (value & 1));
		value = static_cast<IntegerType>(value >> 1);
	}

	return reversedValue;
}


/**
	@brief Computes the final reflection and XOR of a CRC remainder.
	@param[in] remainder CRC remainder to reflect and XOR
	@param[in] finalXOR Final value to XOR with the remainder
	@param[in] reflectOutput true to reflect each byte of the remainder before the XOR
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return Final CRC
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::Finalize(CRCType remainder, CRCType finalXOR, bool reflectOutput)
{
	// For masking off the bits for the CRC (in the event that the number of bits in CRCType is larger than CRCWidth)
	static const CRCType BIT_MASK = (CRCType(1) << (CRCWidth - CRCType(1))) |
											 ((CRCType(1) << (CRCWidth - CRCType(1))) - CRCType(1));

	if (reflectOutput)
	{
		remainder = Reflect(remainder, CRCWidth);
	}

	return (remainder ^ finalXOR) & BIT_MASK;
}


/**
	@brief Undoes the process of computing the final reflection and XOR of a CRC remainder.
	@note This function allows for computation of multi-part CRCs
	@note Calling UndoFinalize() followed by Finalize() (or vice versa) will always return the original remainder value:

		CRCType x = ...;
		CRCType y = Finalize(x, finalXOR, reflectOutput);
		CRCType z = UndoFinalize(y, finalXOR, reflectOutput);
		assert(x == z);

	@param[in] crc Reflected and XORed CRC
	@param[in] finalXOR Final value XORed with the remainder
	@param[in] reflectOutput true if the remainder is to be reflected
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return Un-finalized CRC remainder
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::UndoFinalize(CRCType crc, CRCType finalXOR, bool reflectOutput)
{
	// For masking off the bits for the CRC (in the event that the number of bits in CRCType is larger than CRCWidth)
	static const CRCType BIT_MASK = (CRCType(1) << (CRCWidth - CRCType(1))) |
											 ((CRCType(1) << (CRCWidth - CRCType(1))) - CRCType(1));

	crc = (crc & BIT_MASK) ^ finalXOR;

	if (reflectOutput)
	{
		crc = Reflect(crc, CRCWidth);
	}

	return crc;
}



/**
	@brief Computes a CRC remainder.
	@param[in] data Data over which the remainder will be computed
	@param[in] size Size of the data, in bytes
	@param[in] parameters CRC parameters
	@param[in] remainder Running CRC remainder. Can be an initial value or the result of a previous CRC remainder calculation.
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC remainder
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::CalculateRemainder(const void * data, size_t size, const Parameters<CRCType, CRCWidth> & parameters, CRCType remainder)
{

	// Catching this compile-time error is very important. Sadly, the compiler error will be very cryptic, but it's
	// better than nothing.
	enum { static_assert_failed_CRCType_is_too_small_to_contain_a_CRC_of_width_CRCWidth = 1 / (::std::numeric_limits<CRCType>::digits >= CRCWidth ? 1 : 0) };


	const unsigned char * current = reinterpret_cast<const unsigned char *>(data);

	// Slightly different implementations based on the parameters. The current implementations try to eliminate as much
	// computation from the inner loop (looping over each bit) as possible.
	if (parameters.reflectInput)
	{
		CRCType polynomial = calc_crc::Reflect(parameters.polynomial, CRCWidth);
		while (size--)
		{
			remainder = static_cast<CRCType>(remainder ^ *current++);

			// An optimizing compiler might choose to unroll this loop.
			for (size_t i = 0; i < CHAR_BIT; ++i)
			{
				remainder = static_cast<CRCType>((remainder & 1) ? ((remainder >> 1) ^ polynomial) : (remainder >> 1));
			}
		}
	}
	else if (CRCWidth >= CHAR_BIT)
	{
		static const CRCType CRC_WIDTH_MINUS_ONE(CRCWidth - CRCType(1));
		static const CRCType CRC_HIGHEST_BIT_MASK(CRCType(1) << CRC_WIDTH_MINUS_ONE);
		// The conditional expression is used to avoid a -Wshift-count-overflow warning.
		static const CRCType SHIFT((CRCWidth >= CHAR_BIT) ? static_cast<CRCType>(CRCWidth - CHAR_BIT) : 0);

		while (size--)
		{
			remainder = static_cast<CRCType>(remainder ^ (static_cast<CRCType>(*current++) << SHIFT));

			// An optimizing compiler might choose to unroll this loop.
			for (size_t i = 0; i < CHAR_BIT; ++i)
			{
				remainder = static_cast<CRCType>((remainder & CRC_HIGHEST_BIT_MASK) ? ((remainder << 1) ^ parameters.polynomial) : (remainder << 1));
			}
		}
	}
	else
	{
		static const CRCType CHAR_BIT_MINUS_ONE(CHAR_BIT - 1);
		static const CRCType CHAR_BIT_HIGHEST_BIT_MASK(CRCType(1) << CHAR_BIT_MINUS_ONE);

		// The conditional expression is used to avoid a -Wshift-count-overflow warning.
		static const CRCType SHIFT((CHAR_BIT >= CRCWidth) ? static_cast<CRCType>(CHAR_BIT - CRCWidth) : 0);

		CRCType polynomial = static_cast<CRCType>(parameters.polynomial << SHIFT);
		remainder = static_cast<CRCType>(remainder << SHIFT);

		while (size--)
		{
			remainder = static_cast<CRCType>(remainder ^ *current++);

			// An optimizing compiler might choose to unroll this loop.
			for (size_t i = 0; i < CHAR_BIT; ++i)
			{
				remainder = static_cast<CRCType>((remainder & CHAR_BIT_HIGHEST_BIT_MASK) ? ((remainder << 1) ^ polynomial) : (remainder << 1));
			}
		}

		remainder = static_cast<CRCType>(remainder >> SHIFT);
	}

	return remainder;
}





/**
	@brief Computes a CRC remainder using lookup table.
	@param[in] data Data over which the remainder will be computed
	@param[in] size Size of the data, in bytes
	@param[in] lookupTable CRC lookup table
	@param[in] remainder Running CRC remainder. Can be an initial value or the result of a previous CRC remainder calculation.
	@tparam CRCType Integer type for storing the CRC result
	@tparam CRCWidth Number of bits in the CRC
	@return CRC remainder
*/
template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::CalculateRemainder(const void * data, size_t size, const Table<CRCType, CRCWidth> & lookupTable, CRCType remainder)
{
	const unsigned char * current = reinterpret_cast<const unsigned char *>(data);

	if (lookupTable.GetParameters().reflectInput)
	{
		while (size--)
		{

			remainder = static_cast<CRCType>((remainder >> CHAR_BIT) ^ lookupTable[static_cast<unsigned char>(remainder ^ *current++)]);

		}
	}
	else if (CRCWidth >= CHAR_BIT)
	{
		// The conditional expression is used to avoid a -Wshift-count-overflow warning.
		static const CRCType SHIFT((CRCWidth >= CHAR_BIT) ? static_cast<CRCType>(CRCWidth - CHAR_BIT) : 0);

		while (size--)
		{
			remainder = static_cast<CRCType>((remainder << CHAR_BIT) ^ lookupTable[static_cast<unsigned char>((remainder >> SHIFT) ^ *current++)]);
		}
	}
	else
	{
		// The conditional expression is used to avoid a -Wshift-count-overflow warning.
		static const CRCType SHIFT((CHAR_BIT >= CRCWidth) ? static_cast<CRCType>(CHAR_BIT - CRCWidth) : 0);

		remainder = static_cast<CRCType>(remainder << SHIFT);

		while (size--)
		{
			// Note: no need to mask here since remainder is guaranteed to fit in a single byte.
			remainder = lookupTable[static_cast<unsigned char>(remainder ^ *current++)];
		}

		remainder = static_cast<CRCType>(remainder >> SHIFT);
	}

	return remainder;
}


template <typename CRCType, uint16_t CRCWidth>
inline CRCType calc_crc::CalculateRemainderBits(unsigned char byte, size_t numBits, const Parameters<CRCType, CRCWidth> & parameters, CRCType remainder)
{
	// Slightly different implementations based on the parameters. The current implementations try to eliminate as much
	// computation from the inner loop (looping over each bit) as possible.
	if (parameters.reflectInput)
	{
		CRCType polynomial = calc_crc::Reflect(parameters.polynomial, CRCWidth);
		remainder = static_cast<CRCType>(remainder ^ byte);

		// An optimizing compiler might choose to unroll this loop.
		for (size_t i = 0; i < numBits; ++i)
		{
			remainder = static_cast<CRCType>((remainder & 1) ? ((remainder >> 1) ^ polynomial) : (remainder >> 1));
		}
	}
	else if (CRCWidth >= CHAR_BIT)
	{
		static const CRCType CRC_WIDTH_MINUS_ONE(CRCWidth - CRCType(1));
		static const CRCType CRC_HIGHEST_BIT_MASK(CRCType(1) << CRC_WIDTH_MINUS_ONE);

		// The conditional expression is used to avoid a -Wshift-count-overflow warning.
		static const CRCType SHIFT((CRCWidth >= CHAR_BIT) ? static_cast<CRCType>(CRCWidth - CHAR_BIT) : 0);

		remainder = static_cast<CRCType>(remainder ^ (static_cast<CRCType>(byte) << SHIFT));

		// An optimizing compiler might choose to unroll this loop.
		for (size_t i = 0; i < numBits; ++i)
		{
			remainder = static_cast<CRCType>((remainder & CRC_HIGHEST_BIT_MASK) ? ((remainder << 1) ^ parameters.polynomial) : (remainder << 1));
		}
	}
	else
	{
		static const CRCType CHAR_BIT_MINUS_ONE(CHAR_BIT - 1);
		static const CRCType CHAR_BIT_HIGHEST_BIT_MASK(CRCType(1) << CHAR_BIT_MINUS_ONE);

		// The conditional expression is used to avoid a -Wshift-count-overflow warning.
		static const CRCType SHIFT((CHAR_BIT >= CRCWidth) ? static_cast<CRCType>(CHAR_BIT - CRCWidth) : 0);

		CRCType polynomial = static_cast<CRCType>(parameters.polynomial << SHIFT);
		remainder = static_cast<CRCType>((remainder << SHIFT) ^ byte);

		// An optimizing compiler might choose to unroll this loop.
		for (size_t i = 0; i < numBits; ++i)
		{
			remainder = static_cast<CRCType>((remainder & CHAR_BIT_HIGHEST_BIT_MASK) ? ((remainder << 1) ^ polynomial) : (remainder << 1));
		}

		remainder = static_cast<CRCType>(remainder >> SHIFT);
	}

	return remainder;
}


/***********************************************************************************************/
/***********************************************************************************************/
/********************************* START-BUSSYSTEME ********************************************/
/***********************************************************************************************/
/***********************************************************************************************/

/**
	@brief Returns a set of parameters for CRC-8 SMBus.
	@note The parameters are static and are delayed-constructed to reduce memory footprint.
	@note CRC-8 SMBus has the following parameters and check value:
		- polynomial     = 0x07
		- initial value  = 0x00
		- final XOR      = 0x00
		- reflect input  = false
		- reflect output = false
		- check value    = 0xF4
	@return CRC-8 SMBus parameters
*/
inline const calc_crc::Parameters<uint8_t, 8> & calc_crc::CRC_8()
{
	static const Parameters<uint8_t, 8> parameters = { 0x07, 0x00, 0x00, false, false };
	return parameters;
}


/**
	@brief Returns a set of parameters for CRC-11 FlexRay.
	@note The parameters are static and are delayed-constructed to reduce memory footprint.
	@note CRC-11 FlexRay has the following parameters and check value:
		- polynomial     = 0x385
		- initial value  = 0x01A
		- final XOR      = 0x000
		- reflect input  = false
		- reflect output = false
		- check value    = 0x5A3
	@return CRC-11 FlexRay parameters
*/
inline const calc_crc::Parameters<uint16_t, 11> & calc_crc::CRC_11()
{
	static const Parameters<uint16_t, 11> parameters = { 0x385, 0x01A, 0x000, false, false };
	return parameters;
}


/**
	@brief Returns a set of parameters for CRC-15 CAN.
	@note The parameters are static and are delayed-constructed to reduce memory footprint.
	@note CRC-15 CAN has the following parameters and check value:
		- polynomial     = 0x4599
		- initial value  = 0x0000
		- final XOR      = 0x0000
		- reflect input  = false
		- reflect output = false
		- check value    = 0x059E
	@return CRC-15 CAN parameters
*/
inline const calc_crc::Parameters<uint16_t, 15> & calc_crc::CRC_15()
{
	static const Parameters<uint16_t, 15> parameters = { 0x4599, 0x0000, 0x0000, false, false };
	return parameters;
}


/**
	@brief Returns a set of parameters for CRC-17 CAN.
	@note The parameters are static and are delayed-constructed to reduce memory footprint.
	@note CRC-17 CAN has the following parameters and check value:
		- polynomial     = 0x1685B
		- initial value  = 0x00000
		- final XOR      = 0x00000
		- reflect input  = false
		- reflect output = false
		- check value    = 0x04F03
	@return CRC-17 CAN parameters
*/
inline const calc_crc::Parameters<uint32_t, 17> & calc_crc::CRC_17_CAN()
{
	static const Parameters<uint32_t, 17> parameters = { 0x1685B, 0x00000, 0x00000, false, false };
	return parameters;
}

/**
	@brief Returns a set of parameters for CRC-21 CAN.
	@note The parameters are static and are delayed-constructed to reduce memory footprint.
	@note CRC-21 CAN has the following parameters and check value:
		- polynomial     = 0x102899
		- initial value  = 0x000000
		- final XOR      = 0x000000
		- reflect input  = false
		- reflect output = false
		- check value    = 0x0ED841
	@return CRC-21 CAN parameters
*/
inline const calc_crc::Parameters<uint32_t, 21> & calc_crc::CRC_21_CAN()
{
	static const Parameters<uint32_t, 21> parameters = { 0x102899, 0x000000, 0x000000, false, false };
	return parameters;
}


/**
	@brief Returns a set of parameters for CRC-24 FlexRay-A.
	@note The parameters are static and are delayed-constructed to reduce memory footprint.
	@note CRC-24 FlexRay-A has the following parameters and check value:
		- polynomial     = 0x5D6DCB
		- initial value  = 0xFEDCBA
		- final XOR      = 0x000000
		- reflect input  = false
		- reflect output = false
		- check value    = 0x7979BD
	@return CRC-24 FlexRay-A parameters
*/
inline const calc_crc::Parameters<uint32_t, 24> & calc_crc::CRC_24_FLEXRAYA()
{
	static const Parameters<uint32_t, 24> parameters = { 0x5D6DCB, 0xFEDCBA, 0x000000, false, false };
	return parameters;
}


/**
	@brief Returns a set of parameters for CRC-24 FlexRay-B.
	@note The parameters are static and are delayed-constructed to reduce memory footprint.
	@note CRC-24 FlexRay-B has the following parameters and check value:
		- polynomial     = 0x5D6DCB
		- initial value  = 0xABCDEF
		- final XOR      = 0x000000
		- reflect input  = false
		- reflect output = false
		- check value    = 0x1F23B8
	@return CRC-24 FlexRay-B parameters
*/
inline const calc_crc::Parameters<uint32_t, 24> & calc_crc::CRC_24_FLEXRAYB()
{
	static const Parameters<uint32_t, 24> parameters = { 0x5D6DCB, 0xABCDEF, 0x000000, false, false };
	return parameters;
}


/**
	@brief Returns a set of parameters for CRC-32 (aka CRC-32 ADCCP, CRC-32 PKZip).
	@note The parameters are static and are delayed-constructed to reduce memory footprint.
	@note CRC-32 has the following parameters and check value:
		- polynomial     = 0x04C11DB7
		- initial value  = 0xFFFFFFFF
		- final XOR      = 0xFFFFFFFF
		- reflect input  = true
		- reflect output = true
		- check value    = 0xCBF43926
	@return CRC-32 parameters
*/
inline const calc_crc::Parameters<uint32_t, 32> & calc_crc::CRC_32()
{
	static const Parameters<uint32_t, 32> parameters = { 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true };
	return parameters;
}
/***********************************************************************************************/
/***********************************************************************************************/
/**************************** ENDE-BUSSYSTEME **************************************************/
/***********************************************************************************************/
/***********************************************************************************************/


/***************************************  ENDE: aus CRCpp  *************************************/


#endif // CALC_CRC_H
