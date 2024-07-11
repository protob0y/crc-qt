 #include "calc_crc.h"
#include <QByteArray>


/**
 * @brief calc_crc::FrHeader_CRC
 * @param data			Liste der Daten, aus denen der CRC-Wert berechnet werden soll
 * @param byteArray		Zeiger auf byteArray, in dem die Daten für die HeaderCRC-Berechnung gespeichert werden
 * @param check			Variable mit dem Status der Checkboxen
 * @param dataLength	Variable zum Speichern der eingegebenen Länge der Nutzdaten
 * @return error-String
 */
QString calc_crc::FrHeader_CRC(QList<QString> data, uint8_t* byteArray, QList<QString> * results, uint8_t check, uint8_t* dataLength){
	QString query;
	std::uint16_t FrameId;
	std::uint16_t crc;
	std::uint8_t bits;

	if (check & SyncFrameIndi){
		byteArray[0] = 0x80;		//SyncFrameIndicator
	}
	if (check & StartFrameIndi){
		byteArray[0] |=0x40;
	}

	//11Bit der Frame ID an die richtige Stelle im byteArray[0] und byteArray[1] schieben
	query = data[0];
	parse(query);
	FrameId = (parse_result[0] << 8) | parse_result[1];
	FrameId = FrameId >> 4;			// rechtsbündig -> aus 0xFFF0 wird 0x0FFF
	if (FrameId > 0x7FF){
		return "failure FrameID";
	}

	bits = (FrameId >> 5) & 0x3F; // Verschiebung um 5 Bits nach rechts, um die ersten 6 Bits zu isolieren
	byteArray[0] |= bits;
	bits = FrameId & 0x1F;
	byteArray[1] |= bits <<3;


	//7-Bit der Payload-Länge an die richtige Stelle im byteArray[1] und byteArray[2] schieben
	query = data[1];	//payload-Länge als String
	parse(query);		// String in uint8_t Value umwandeln
	*dataLength = parse_result[0];	// Value zwischenspeichern
	if(*dataLength > 0x7F){
		return "failure dataLength";
	}

	bits = (*dataLength >> 4) & 0x07;		//3 Bits in bits speichern
	byteArray[1] |= bits;				// die letzen 3 bits in byteArray[1] setzen
	bits = *dataLength  & 0x0F;				//4 Bits in bits speichern
	byteArray[2] |= bits << 4;			// die ersten 4 bits in byteArray[2] setzen

	qDebug() << "byteArray: "<<byteArray[0]<<" "<<byteArray[1]<<" "<<byteArray[2];

	crc= CalculateBits(byteArray, 20, FR11table);	// HEADER-CRC berechnen


	//CRC-Value an die Daten anhängen
	bits = (crc >> 7) & 0x0F; // Verschiebung um 7 Bits nach rechts, um die ersten 4 Bits zu isolieren
	byteArray[2] |= bits;
	bits = crc & 0x7F;
	byteArray[3] |= bits << 1;

	QString hexString = QString("0x") + QString::number(crc, 16).toUpper();
	QString binString = QString("0b") + QString::number(crc, 2);


	QString number = number.number(crc);

	results->append("Header CRC:");
	results->append(hexString);
//	results->append("Header CRC (dec):");
//	results->append(number);
	results->append("Header CRC:");
	results->append(binString);

	qDebug() << "Der berechnete Header-CRC: "<< crc;
	qDebug() <<  binString;
	qDebug() <<  hexString;

	return "success";
}

/**
 * @brief calc_crc::Fr_CRC
 * @param data			Liste der Daten, aus denen der CRC-Wert berechnet werden soll
 * @param byteArray		Zeiger auf byteArray, in dem die Daten der HeaderCRC-Berechnung gespeichert sind
 * @param check			Variable mit dem Status der Checkboxen
 * @return	error-String
 */
QString calc_crc::Fr_CRC(QList<QString> data, uint8_t *byteArray, QList<QString> * results,  uint8_t check, uint8_t* dataLength){
	QString query;
	std::uint32_t crc;
	uint8_t cycleCount = 0;
	uint8_t dataArray[5] = {0x00, 0x00, 0x00, 0x00, 0x00};


	//Aus byteArray des Headers, check-Value und  data[0] (=CycleCount) den Header korrekt zusammensetzen.
	dataArray[0] |= check << 3;	//
	dataArray[0] |= byteArray[0] >> 3 ;
	for ( int i = 1; i <= 3; i++){	//Gernzen noch setzen
		dataArray[i] |= byteArray[i-1] << 5;
		dataArray[i] |= byteArray[i] >> 3;
	}
	dataArray[4] |= byteArray[3] << 5; // letzten 2 bit vom Header-CRC setzen + letztes 0 Bit
	// In dataArray sind nun die StatusBits, FrameID und HeaderCRC enthalten

	query = data[0];	//CycleCount -> 6bit groß
	parse(query);
	cycleCount = parse_result[0];
	if(cycleCount > 0x3F){
		return "failure cycleCount";
	}
	dataArray[4] |= cycleCount;
	// HEADER nun komplett richtig gefüllt



// Erstellt ein QByteArray-Objekt, das direkt auf die ersten 5 Bytes von dataArray zeigt, ohne die Daten zu kopieren
	QByteArray qByteArray = QByteArray::fromRawData(reinterpret_cast<const char*>(dataArray), 5);

//Nutzdaten verarbeiten
	query = data[1];		// Payload-Data als String
	parse(query);			// String in Value umwandeln



	if(  parse_result.size() != (*dataLength)*2 ){	// Prüfen, ob Datenmenge mit Payloadlänge übereinstimmt
		return "failure dataLength";
	}

	if(  *dataLength != 0  ){	// wenn Daten vorhanden, dann anhängen
		qByteArray.resize(parse_result.size() + 5);	//Größe von qByteArray anpassen, entsprechend der größe der Nutzdaten

		for (int i = 0; i < parse_result.size(); i++) {
				qByteArray[i + 5] = parse_result[i];		//Nutzdaten in qByteArray kopieren
			}
	}


//FlexRay-Kannal-A
	crc = Calculate(qByteArray, qByteArray.size(), FRA24table);
	QString hexString = QString("0x") + QString::number(crc, 16).toUpper();
	QString binString = QString("0b") + QString::number(crc, 2);
	QString number = number.number(crc);

	qDebug() << "FlexRay-Channel A";
	qDebug() << "CRC-Trailer-A in Dezimal: "<< crc;
	qDebug() <<  hexString;
	qDebug() <<  binString;

	results->append("CRC-Trailer-A:");
	results->append(hexString);
	results->append("CRC-Trailer-A:");
	results->append(binString);
//	results->append("CRC-Trailer-A (dec):");
//	results->append(number);

//FlexRay-Kannal-B
	crc = Calculate(qByteArray, qByteArray.size(), FRB24table);
	hexString = QString("0x") + QString::number(crc, 16).toUpper();
	binString = QString("0b") + QString::number(crc, 2);
	number = number.number(crc);

//	qDebug() << "FlexRay-Channel B";
//	qDebug() << "CRC-Trailer-B in Dezimal: "<< crc;
//	qDebug() << "CRC-Trailer-B in Hex: "<< hexString;
//	qDebug() << "CRC-Trailer-B in Bin: "<< binString;

	results->append("CRC-Trailer-B:");
	results->append(hexString);
	results->append("CRC-Trailer-B:");
	results->append(binString);
//	results->append("CRC-Trailer-B (dec):");
//	results->append(number);

	return "success";
}


/**
 * @brief calc_crc::getParseValue
 * @param query		umzuwandelnder String
 * @return
 */
uint8_t calc_crc::getParseValue(QString query){
	parse(query);
	return parse_result[0];
}


QString calc_crc::LinClassic_CRC(QList<QString> data, QList<QString> * results){
    QString query = data[0]; // erste query extrahieren (wir bekommen überhaupt nur einen eintrag im gelieferten datenpaket)

    parse(query);

    //Checksumme berechnen...
    qDebug() << "Ich bekam vom Parser eine Liste mit " << parse_result.length() << " Einträgen, " << parsed_bits << " Bits wurden geparsed.";
    //Jeder eintrag ist 8bit breit

    //für lin wird inv(byte1+byte2+byte3+...) gemacht:
    uint16_t sum = 0;
    for(int i = 0; i < parse_result.length(); i++){
        sum += parse_result[i];
    }
    if(sum >= 256)
        sum -= 255;
    sum = ~sum;
    sum = sum & 0x00FF; // nur die rechten 8 bit behalten

    QString binary_checksum_string = toReadableStringBin(sum, 8);
    QString hex_checksum_string = toReadableStringHex(sum, 8);

    results->append("Checksum (b):");
    results->append(binary_checksum_string);

    results->append("Checksum (h):");
    results->append(hex_checksum_string);

    return "";
}

QString calc_crc::LinEnhanced_CRC(QList<QString> data, QList<QString> * results){
    qDebug() << "Der Lin Enh CRC calcer";
    QString idQuery = data[0];
    QString pyldQuery = data[1];

    uint8_t pid = 0;

    //Calculate Parity bits
    parse(idQuery);
    uint8_t id = parse_result[0] & 0b00111111;
    if(!__builtin_parity(id & 0b00111010)){
        pid |= 0b10000000;
    }
    if(__builtin_parity(id & 0b00010111)){
        pid |= 0b01000000;
    }

    //string for output of parity bits
    QString parityBitsString = "";
    parityBitsString += QString::number((pid & 0b10000000) >> 7);
    parityBitsString += ", ";
    parityBitsString += QString::number((pid & 0b01000000) >> 6);
    results->append("Paritätsbits P1 und P0");
    results->append(parityBitsString);

    //whole identifier byte is now id (actually called "PID")
    id |= pid & 0b11000000;

    //write out into result window
    QString idStringBin = toReadableStringBin(id, 8);
    QString idStringHex = toReadableStringHex(id, 8);
    results->append("Protected Identifier (b)");
    results->append(idStringBin);
    results->append("Protected Identifier (h)");
    results->append(idStringHex);

    //ACTUAL CHECKSUM GENERATION
    //parse the payload
    parse(pyldQuery);

    //calculate the whole checksum
	uint16_t sum = 0;
    sum += id;
    //sum now contains the PID
    //now we'll go for each data byte
    bool carry = false;
    for(int i = 0; i < parse_result.length(); i++){
        qDebug() << "beginning addition. sum is " << sum << ", carry is " << carry;
        //for each byte:
        //qDebug() << "parser gab mir als nächstes " << parse_result[i];
        if(!carry){
            sum += parse_result[i];
        }
        else{
            sum += 0b00000001;
            sum += parse_result[i];
            carry = false;
        }

        if(sum >= 256){
            sum -= 256;
            carry = true;
        }

        qDebug() << "finished, sum is " << sum << ", carry is " << carry;
    }
    if(carry){
        sum += 0b00000001;
    }
    sum = sum & 0x00FF; // nur die rechten 8 bit behalten
    sum = ~sum;


    QString binary_checksum_string = toReadableStringBin(sum, 8);
    QString hex_checksum_string = toReadableStringHex(sum, 8);

    results->append("Checksum (b)");
    results->append(binary_checksum_string);

    results->append("Checksum (h)");
    results->append(hex_checksum_string);
    return "No Error";
}

//gets: query
//result goes to a private QList (parse_result)
void calc_crc::parse(QString query){
    qDebug() << "This is the parser. I got: " << query;

    parse_result.clear(); // empty the result buffer
    parsed_bits = 0; // reset the bit counter
    if(query.length() < 3){ // see if query is not too short
        return;
    }

    uint8_t workingByte = 0b00000000; // this is one byte of the result. we will send this byte
    // (and more bytes if necessary)

    //first, lets find if the data is bin or hex
    QString remainder = query.mid(2);
    if(query.startsWith("0b", Qt::CaseInsensitive)){
        for(int pos = 0; pos < remainder.length(); pos++){
            uint8_t current_bit = binCharToBin(remainder[pos]); // one position gets us one bit
            workingByte |= (current_bit << (7-(pos % 8)));
            qDebug() << "wb is " << workingByte;
            parsed_bits++;
            //if now we have read a full byte
            if((pos + 1) % 8 == 0){
                parse_result.append(workingByte);
                qDebug() << "parser appended full byte: " << workingByte;
                workingByte = 0b00000000;
            }
            //if we already reached the end
            else if((pos + 1) == remainder.length()){
                // if we didnt read a full byte, we have to make digits left bounded
                // (ex: "11111100" with only 6 read bytes now becomes "00111111")
                workingByte = workingByte >> (8 - remainder.length());
                parse_result.append(workingByte);
            }
        }
    }
    else if(query.startsWith("0x", Qt::CaseInsensitive)){
        for(int pos = 0; pos < remainder.length(); pos++){
            uint8_t current_halfbyte = hexCharToBin(remainder[pos]); // one position gets us half a byte (4bit)
            //returns 10 for "A" and 11 for "B" -- tested
            parsed_bits+=4;

            if(pos % 2 == 0){ // if we are at an even position (0,2,4,6,...)
                workingByte = current_halfbyte << 4; // make the half byte left bound
            }
            else{ // or if we are at an odd position (1,3,5,...)
                workingByte |= current_halfbyte;
            }

            if((pos + 1) % 2 == 0){ // if we just finished processing at pos 1, 3, 5, ...
                parse_result.append(workingByte); // add this byte to the result list, so we can start with the next two letters
                workingByte = 0b00000000;
                //qDebug() << "Pushed byte, because two chars read.";
            }
            else if(pos + 1 == remainder.length()){ // if we already reached the end of the string
                parse_result.append(workingByte);
                //qDebug() << "Pushed byte, because end reached";
            }
        }
    }
}


//wandelt hex buchstaben "9" oder "F" in bitmuster um.
//gets: QChar
//returns: bit pattern
uint8_t calc_crc::hexCharToBin(QChar q_hex){
    q_hex  = q_hex.toLower(); // Bsp "B" wird zu "b"
    char hex = q_hex.toLatin1(); // zu ascii
    switch(hex){
    case '0':
        return 0b00000000;
    case '1':
        return 0b00000001;
    case '2':
        return 0b00000010;
    case '3':
        return 0b00000011;
    case '4':
        return 0b00000100;
    case '5':
        return 0b00000101;
    case '6':
        return 0b00000110;
    case '7':
        return 0b00000111;
    case '8':
        return 0b00001000;
    case '9':
        return 0b00001001;
    case 'a':
        return 0b00001010;
    case 'b':
        return 0b00001011;
    case 'c':
        return 0b00001100;
    case 'd':
        return 0b00001101;
    case 'e':
        return 0b00001110;
    case 'f':
        return 0b00001111;
    default:
        return 0b00000000;
    }
}

//transforms QChars like "1" or "0" to one byte
uint8_t calc_crc::binCharToBin(QChar q_bin){
    char bin = q_bin.toLatin1();
    switch(bin){
    case '0':
        return 0b00000000;
    case '1':
        return 0b00000001;
    default:
        return 0b00000000;
    }
}

//converts a bit pattern (max 16bit) into a human readable string in binary form (ex: "0b10101010") with a given number of bits to include
QString calc_crc::toReadableStringBin(uint16_t input, int numberOfBits){
    QString result = "0b";
    for(int i = (numberOfBits - 1); i >= 0; i--){
        result += QString::number((input >> i) & 0b00000001);
    }
    return result;
}

//converts a bit pattern (more than 16bit) into a human readable string in binary form (ex: "0b10101010") with a given number of bits to include
QString calc_crc::toReadableStringBin32(uint32_t input, int numberOfBits){
	QString result = "0b";
	for(int i = (numberOfBits - 1); i >= 0; i--){
		result += QString::number((input >> i) & 0b00000001);
	}
	return result;
}

//converts a bit pattern (max16 bits) into a human readable string in hex form (ex: "0x3C") with a given number of bits to include
QString calc_crc::toReadableStringHex(uint16_t input, int numberOfBits){
    QString result = "0x";
    for(int i = (numberOfBits - 4); i >= 0; i-=4){
        result += binToHexChar((input >> i) & 0x000F);
    }
    return result;
}

//converts a bit pattern (max32 bits) into a human readable string in hex form (ex: "0x3C") with a given number of bits to include
QString calc_crc::toReadableStringHex32(uint32_t input, int numberOfBits){
	QString result = "0x";
	for(int i = (numberOfBits - 4); i >= 0; i-=4){
		result += binToHexChar((input >> i) & 0x000F);
	}
	return result;
}

//konvertiert ein halbes byte (als int, also die letzten 4 bit) zu einem buchstaben
QChar calc_crc::binToHexChar(uint8_t halfByte){
    switch(halfByte){
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    case 10:
        return 'A';
    case 11:
        return 'B';
    case 12:
        return 'C';
    case 13:
        return 'D';
    case 14:
        return 'E';
    default:
        return 'F';
    }
}
