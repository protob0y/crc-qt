#include "crc_main.h"
#include "ui_crc_main.h"
#include <vector>

#include "calc_crc.h"

crc_main::crc_main(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::crc_main)
{
    ui->setupUi(this);

    //CRC-Berechnungsklasse instanziieren
    Calculator = new calc_crc();

    //set up regular expressions for pre check
    binaryRegEx.setPattern("^[01]+$"); // only allow 0, 1, b, B, for binary
    hexRegEx.setPattern("^[0-9a-fA-F]+$"); // only allow 0-9, a-f, A-F, x, X for hex
}

crc_main::~crc_main()
{
    delete ui;
    delete Calculator;
}

// updaten der Radio Buttons Ankickbarkeit im LIN tab
void crc_main::lin_updateClickableRadioBtn(){
    if (ui->radioLin2_X->isChecked()){ // wenn user auf "LIN 2.X" klickt
        ui->radioLinEnh->setCheckable(1);
        }
    else{
        //LIN 1.3 ist selektiert
        ui->radioLinEnh->setChecked(0);
        ui->radioLinClassic->setChecked(1);
        ui->radioLinEnh->setCheckable(0);
        lin_update_LineEditID(); // damit wir die ID Zeile und ihr Label deaktivieren
    }
}

// updaten der Eingabezeile bei LIN, sie ist aktiv wenn Enhanced checksum selektiert
void crc_main::lin_update_LineEditID(){
    if(ui->radioLinEnh->isChecked()){
        ui->lineEditLinID->setEnabled(true);
        ui->LS_LinID->setEnabled(true);
    }
    else{
        ui->lineEditLinID->setText("");
        ui->lineEditLinID->setEnabled(false);
        ui->LS_LinID->setEnabled(false);
    }
}

// Erster Plausibilitätscheck eines Eingabefeldes. Aufruf immer bei Textänderung
void crc_main::preliminary_parse(){
    int detected_bits = 0;
    QString message; // wird neben dem eingabefeld angezeigt

    QObject *senderObj = sender(); // Wer ist der Sender/Aufrufer
    if(!senderObj){ // if there is no sender (function was called randomly, shouldnt happen)
        return;
    }
    //who was the caller?
    QLineEdit *senderLineEdit = qobject_cast<QLineEdit*>(senderObj); // Try to cast from QLineEdit
    QPlainTextEdit *senderTextEdit = qobject_cast<QPlainTextEdit*>(senderObj); // Try to cast from QPlainTextEdit
    //if at least one of the two is not null, we can proceed
    if(!senderLineEdit && !senderTextEdit){
        //caller was neither a lineEdit nor a TextEdit
        return;
    }

    //begin check
    bool ok = false;
    QString text = getWidgetText(senderLineEdit, senderTextEdit);
//    qDebug() << "text is " << text;
    QString callerObjectName = getWidgetName(senderLineEdit, senderTextEdit);
    if(text.isEmpty()){
        setWidgetStylesheet(senderLineEdit, senderTextEdit, ""); // remove BG color
        message  = "Eingabe NOK";// set label to "Eingabe NOK"
        UpdateStatusLabel(callerObjectName, 0, message);
        inputOkList.removeAll(callerObjectName);
        return;
    }

    text = stringRemoveDelimiters(text);

    //Textfelder-Eingaben haben Regeln - Bsp: "Nur 4 Bits", "nur volle bytes", "auch vielfache davon".
    //setzen wir hier fest.
    int accept_bits = 1;
    bool accept_multiple = true;
    getInputRules(callerObjectName, &accept_bits, &accept_multiple);
    //qDebug() << "Input rules bekommen: accept bits = " << accept_bits << " mult: " << accept_multiple;

    if(text.length() >= 3){
        QString remainder = text.mid(2);
        if(text.startsWith("0b", Qt::CaseInsensitive)){
            if(binaryRegEx.match(remainder).hasMatch()){
                // if multiples of accept_bits is True and there is a multiple of accepted bits, or if its False and lenght = bits
                if((!accept_multiple && remainder.length() == accept_bits) || (accept_multiple && remainder.length() % accept_bits == 0)){
                    ok = true;
                    detected_bits = remainder.length();
                }
                else{
                    message = "Datenmenge stimmt nicht";
                }
            }
            else{
                message = "Nicht-erlaubte Zeichen.";
            }

            //add delimiters, binary style
            setWidgetText(senderLineEdit, senderTextEdit, stringAddDelimiters(text, 'b'));
        }
        else if(text.startsWith("0x", Qt::CaseInsensitive)){
            if(accept_bits % 4 != 0){
                accept_bits = (accept_bits / 4 + 1) * 4; // if 6bit specified, (6 % 4 = 2) then accept_bits will be (6/4+1)*4=(1+1)*4=8bit, so they can be entered in hex
            }
            if(hexRegEx.match(remainder).hasMatch()){
                int remainderBits = remainder.length() * 4;
                if((!accept_multiple && remainderBits == accept_bits) || (accept_multiple && remainderBits % accept_bits == 0)){
                    ok = true;
                    detected_bits = remainder.length() * 4;
                }
                else{
                    message = "Datenmenge stimmt nicht";
                }
            }
            else{
                message = "Nicht-erlaubte Zeichen.";
            }

            // add delimiters for hexadecimal style
            setWidgetText(senderLineEdit, senderTextEdit, stringAddDelimiters(text, 'h'));
        }
        else{
            message = "Muss mit 0b oder 0x beginnen.";
        }
    }
    else{
        message = "Nicht ausreichend Daten.";
    }

    if(ok){
        //color green
        setWidgetStylesheet(senderLineEdit, senderTextEdit, "background-color: rgba(0,255,0,100);");
        if(!inputOkList.contains(callerObjectName)){
            inputOkList.append(callerObjectName);
        }
        UpdateStatusLabel(callerObjectName, detected_bits);
        qDebug() << "Livaparse erkannte gute Eingabe mit " << detected_bits << " bits";
    }
    else{
        //color red
        setWidgetStylesheet(senderLineEdit, senderTextEdit, "background-color: rgba(255,0,0,100);");
        inputOkList.removeAll(callerObjectName);
        UpdateStatusLabel(callerObjectName, 0, message);
    }
}

QString crc_main::getWidgetText(QLineEdit *senderLineEdit, QPlainTextEdit *senderTextEdit){
    if(senderLineEdit){
        return senderLineEdit->text();
    }
    else if(senderTextEdit){
        return senderTextEdit->toPlainText();
    }
    else{
        return "";
    }
}

QString crc_main::getWidgetName(QLineEdit *senderLineEdit, QPlainTextEdit *senderTextEdit){
    if(senderLineEdit){
        return senderLineEdit->objectName();
    }
    else if(senderTextEdit){
        return senderTextEdit->objectName();
    }
    else{
        return "";
    }
}

void crc_main::setWidgetText(QLineEdit *senderLineEdit, QPlainTextEdit *senderTextEdit, QString text){
    if(senderLineEdit){
        senderLineEdit->setText(text);
    }
    else if(senderTextEdit){

        bool oldState = senderTextEdit->blockSignals(true);
        senderTextEdit->setPlainText(text);

        QTextCursor cursor = senderTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        senderTextEdit->setTextCursor(cursor);

        senderTextEdit->blockSignals(oldState);
    }
}

void crc_main::setWidgetStylesheet(QLineEdit *senderLineEdit, QPlainTextEdit *senderTextEdit, QString style){
    if(senderLineEdit){
        senderLineEdit->setStyleSheet(style);
    }
    else if(senderTextEdit){
        senderTextEdit->setStyleSheet(style);
    }
}

QString crc_main::stringRemoveDelimiters(QString original){
    return original.remove(" ");
}

QString crc_main::stringAddDelimiters(QString original, char type){
    if(original.length() <= 2){
        return original;
    }

    original.remove(0, 2); // strip the first two characters

    int numberOfDelimiters;
    int pos;

    switch(type){
    case 'b':
        // delimiter every 8 chars
        numberOfDelimiters = original.length() / 8;
        if(original.length() % 8 == 0){
            numberOfDelimiters--; // add one delimiter less if the cursor is at the end of a block
        }
        pos = 8;
        for(int i = 0; i < numberOfDelimiters; i++){
            original.insert(pos, ' ');
            pos += 9;
        }
        original.insert(0, "0b");
        break;
    case 'h':
        //delimiter every 2 chars
        numberOfDelimiters = original.length() / 2;
        if(original.length() % 2 == 0){
            numberOfDelimiters--;
        }
        pos = 2;
        for(int i = 0; i < numberOfDelimiters; i++){
            original.insert(pos, ' ');
            pos += 3;
        }
        original.insert(0, "0x");
    break;
    }

    return original;
}

//berechnung starten button geklickt
void crc_main::clicked_buttonCalcCRC(){
    if(ui->tabWidget->currentIndex() == TAB_INDEX_LIN){
        if(ui->radioLinClassic->isChecked()){
            invokeLinCRC(0);
        }
        else if(ui->radioLinEnh->isChecked()){
            invokeLinCRC(1);
        }
	}
	else if(ui->tabWidget->currentIndex()== TAB_INDEX_FLXRY){
			invokeFrCRC();

	}
    else if(ui->tabWidget->currentIndex() == TAB_INDEX_CAN){
        invokeCanCRC(0,0);
    }
    else if(ui->tabWidget->currentIndex() == TAB_INDEX_CANFD){
        invokeCanCRC(1,0);
    }
    else if(ui->tabWidget->currentIndex() == TAB_INDEX_CANXL){
        invokeCanCRC(0,1);
    }
}

void crc_main::invokeCanCRC(bool fd, bool xl){
    qDebug() << "invokeCanCRC aufgerufen";
    //Standard CAN:
    if(fd == 0 && xl == 0){
        qDebug() << "Standard CAN CRC";

        //--------------------//
        //CAN CRC Berechnung
        //CRC stimmt nicht überein

        std::vector<uint8_t> canData = {0x0, 0x6, 0xF, 0x0, 0x2, 0x1, 0x0};

        uint16_t crc = 0;

        for (auto byte : canData) {
            for (int i = 0; i < 8; ++i) {
                bool bit = (byte >> (7 - i)) & 1;
                bool c15 = (crc >> 14) & 1;
                crc <<= 1;
                if (c15 ^ bit) {
                    crc ^= 0x4599; // Generatorpolynom: x^15 + x^14 + x^10 + x^8 + x^7 + x^4 + x^3 + 1 (0x4599)
                }
            }
        }

        uint16_t result = crc & 0x7FFF; // CRC-15 ist 15 Bit lang, daher nur die unteren 15 Bit verwenden
        qDebug() << result;
        //-----------------------//
        //Ende CAN CRC Berechnung//
        //-----------------------//
    }
    //CAN FD:
    else if(fd == 1 && xl == 0){
        qDebug() << "CAN FD CRC";

    }
    //CAN XL:
    else if(fd == 0 && xl == 1){
        qDebug() << "CAN XL CRC";

    }
    qDebug() << "invokeCanCRC voll durchlaufen";
    return;
}

void crc_main::invokeFrCRC(){
	QList<QString> results;
	QList<QString> dataToParse;
	QList<QString> dataToParseHeader;
	uint8_t byteArray[] = {0x00, 0x00, 0x00, 0x00, 0x00};	//Array für den Header
	QString headerCRC, FehlerCode;
	uint8_t check = None;			//Variable für den Status der CheckBoxen
	uint8_t dataLength = 0;
	bool alleFelderGruen = false;
	bool dataLenghtIs0 = false;
	bool teilweiseGruen = false;



	if(	   inputOkList.contains(ui->lineEditFrFrameID->objectName())			//Checken, ob alle Eingabefelder "grün" sind
		&& inputOkList.contains(ui->lineEditFrPayloadlaenge->objectName())
		&& inputOkList.contains(ui->lineEditFrCycleCount->objectName())
		&& inputOkList.contains(ui->plainTextEditFR->objectName())
		   ) {
		alleFelderGruen = true;
	} else if(	   inputOkList.contains(ui->lineEditFrFrameID->objectName())
				&& inputOkList.contains(ui->lineEditFrPayloadlaenge->objectName())
				&& inputOkList.contains(ui->lineEditFrCycleCount->objectName()) ){
				teilweiseGruen = true;
	}

	if( 0 == Calculator->getParseValue(ui->lineEditFrPayloadlaenge->text()) ){
		dataLenghtIs0 = true;
	}


	if( alleFelderGruen || ( teilweiseGruen && dataLenghtIs0) ){

			qDebug() << "Alle nötigen Daten zur Berechnung wurden korrekt eingegeben!";
			//Status der Checkboxen festhalten
			if(ui->checkBoxFrSyncFrameIndi->isChecked() ){
				check |= SyncFrameIndi;
				qDebug() << "Sync-FrameIndicator is set!";
			}
			if(ui->checkBoxFrStartupFrameIndi->isChecked() ){
				check |= StartFrameIndi;
				qDebug() << "Startup-FrameIndicator is set!";
			}
			if(ui->checkBoxFrNullFrameIndi->isChecked() ){
				check |= NullFrameIndi;
			}
			if(ui->checkBoxFrPayloadPreambleIndi->isChecked()){
				check |= PayloadPreIndi;
			}

			check |= ReservedBit;	// immer low

		//Header CRC
			dataToParseHeader.append(stringRemoveDelimiters(ui->lineEditFrFrameID->text()));
			dataToParseHeader.append(stringRemoveDelimiters(ui->lineEditFrPayloadlaenge->text()));
			// Header CRC berechnen
			FehlerCode = Calculator->FrHeader_CRC(dataToParseHeader, byteArray, &results, check, &dataLength );
			if(FehlerCode == "failure dataLength"){
				showErrorBox("Fehler: Die eingegebene Payloadlänge ist Größer als 0x7F!");
				return ;
			} else if(FehlerCode == "failure FrameID"){
				showErrorBox("Fehler: Die eingegebene Frame ID ist Größer als 0x7FF!");
				return;
			}


		//Trailer CRC
			dataToParse.append(stringRemoveDelimiters(ui->lineEditFrCycleCount->text()));
			dataToParse.append(stringRemoveDelimiters(ui->plainTextEditFR->toPlainText()));
			// Trailer CRC berechnen
			FehlerCode = Calculator->Fr_CRC(dataToParse, byteArray, &results, check, &dataLength);

			if(FehlerCode == "failure cycleCount"){
				showErrorBox("Fehler: Der eingegebene Cycle Count ist Größer als 0x3F!");
				return;
			} else if(FehlerCode == "failure dataLength"){
				showErrorBox("Fehler: Payloadlänge stimmt nicht mit der Anzahl an eingegebenen Daten überein!");
			}


	//} else if(teilweiseGruen ){

	} else {
		showErrorBox("Fehler: Mindestens ein Eingabefeld ist fehlerhaft oder leer");
		return;
	}


	myResultWindow = new ResultWindow();
	myResultWindow->setAttribute(Qt::WA_DeleteOnClose);
	myResultWindow->push_result(&results);
	myResultWindow->show();

}





//diese funktion macht nochmal einen preliminären check, ob alle userdaten da sind. danach ruft sie den parser auf.
void crc_main::invokeLinCRC(bool enhanced){
    if(ui->lineEditLinData->text() == "0x45 61 73 74 65 72 21 0A"){
        showErrorBox("Happy Easter! Greetings from the main dev. May you live in peace and happiness. 07 Jun 2024, protoman.");
    }
    QList<QString> results;
    QList<QString> dataToParse; // diese liste = "paket", wo wir verschiedene daten reinpacken und dem parser geben.
    QString err;
    if(enhanced){
        if(inputOkList.contains(ui->lineEditLinData->objectName()) && inputOkList.contains(ui->lineEditLinID->objectName())){
            qDebug() << "LIN ENH Check durchführen";
            // hier packen wir den text aus dem data- und id-textfeld (beide) in das übergabepaket.
            dataToParse.append(stringRemoveDelimiters(ui->lineEditLinID->text()));
            dataToParse.append(stringRemoveDelimiters(ui->lineEditLinData->text()));
            err = Calculator->LinEnhanced_CRC(dataToParse, &results);
        }
        else{
            showErrorBox("Fehler: Mindestens ein Eingabefeld ist fehlerhaft oder leer");
        }
    }
    else{ // classic checksum
        if(inputOkList.contains(ui->lineEditLinData->objectName())){
            qDebug() << "Starte mit classic lin checksumme";
            dataToParse.append(stringRemoveDelimiters(ui->lineEditLinData->text()));
            err = Calculator->LinClassic_CRC(dataToParse, &results);
        }
        else{
            showErrorBox("Fehler: Das Datenfeld ist fehlerhaft oder leer");
        }
    }

    qDebug() << "Die ergebnisse bestehen aus " << results.length() << " Daten.";
    /*for(int i = 0; i < results.length(); i++){
        qDebug() << results[i];
    }*/

    //neues Fenster öffnen
    myResultWindow = new ResultWindow();
    myResultWindow->setAttribute(Qt::WA_DeleteOnClose);
    myResultWindow->push_result(&results);
    myResultWindow->show();
}

void crc_main::showErrorBox(QString text){
    QMessageBox errorBox;
    errorBox.setIcon(QMessageBox::Critical);
    errorBox.setWindowTitle("May the force fix this error.");
    errorBox.setText(text);
    errorBox.addButton(QMessageBox::Ok);
    errorBox.exec();
}

//Diese Funktion updated labels neben den Textfeldern, die anzeigen, ob die Eingabe OK ist und wie viele bits erkannt wurden: UpdateStatusLabel(sender, 16);
//Sie kann auch Fehler anzeigen: UpdateStatusLabel(sender, 0, "Bitte nicht-erlaubte Zeichen entfernen");
// Sie wird gecallt von der preliminary_parse, die bei jeder veränderung jeder LineEdit aufgerufen wird.
void crc_main::UpdateStatusLabel(QString sender, int detected_bits, QString message){
    QString text;
    if(message.length() == 0){
        text = "(OK) " + QString::number(detected_bits) + " Bits.";
    }
    else{
        text = message;
    }

    //LIN
    if(sender == "lineEditLinData"){
        ui->LS_LinData->setText(text);
    }
    else if(sender == "lineEditLinID"){
        ui->LS_LinID->setText(text);
    }
    //Flexray
	else if (sender == "lineEditFrFrameID"){
		ui->labelFrIDEingabeNOK->setText(text);
	}
	else if (sender == "lineEditFrCycleCount"){
		ui->labelFrCycleCountEingabeNOK->setText(text);
	}
	else if (sender == "lineEditFrPayloadlaenge"){
		ui->labelFrPayloadlaengeEingabeNOK->setText(text);
	}
	else if (sender ==  "plainTextEditFR"){
		ui->labelFrDataEingabeNOK->setText(text);
	}
    //CAN Standard
    else if (sender ==  "lineEditCAN_Identifier"){
        ui->labelCAN_Identifier_EingabeNOK->setText(text);
    }
    else if (sender ==  "lineEditCAN_DLC"){
        ui->labelCAN_DLC_EingabeNOK->setText(text);
    }
    else if (sender ==  "lineEditCAN_Payload"){
        ui->labelCAN_Payload_EingabeNOK->setText(text);
    }
    //CAN FD
    //CAN XL
}

//Autofill
//Diese Funktion füllt Felder automatisch aus
void crc_main::autofill_fields(){
    QObject *senderObj = sender(); // Wer ist der Sender/Aufrufer
    if(!senderObj){
        return;
    }
    char filltype;
    int numberOfBits = ui->SpinBoxAutofillBytes->value() * 8; // how many bits we have to fill

    //who is the inquiry from?
    if(senderObj->objectName() == "ButtonFillOnes"){
        filltype = '1';
        qDebug() << "Fülle alles mit 1";
    }
    else if(senderObj->objectName() == "ButtonFillZeros"){
        filltype = '0';
        qDebug() << "Fülle alles mit 0";
    }
    else{
        filltype = 'r';
        qDebug() << "Fülle alles mit Zufall";
    }

    //now the actual filling
    if(ui->tabWidget->currentIndex() == TAB_INDEX_LIN){
        if(numberOfBits > 64){ // LIN data only accepts up to 8 bytes, but the user could set more than that
            numberOfBits = 64;
        }
        ui->lineEditLinData->setText(generateFillString(filltype, numberOfBits));
        if(ui->lineEditLinID->isEnabled()){
            //this one is special. since LIN ENH ID only supports 6 bits and only up to 0x3B, we have to modify the functions result
            QString fillStr = generateFillString(filltype, 8);
            switch(filltype){
            case '1':
				fillStr = "0x3B";
                break;
            case 'r':
                fillStr.replace(2,1,'0');
            }
            ui->lineEditLinID->setText(fillStr);
        }
    }


	if(ui->tabWidget->currentIndex() == TAB_INDEX_FLXRY ){
		QString fillStr;
		uint16_t randomValue;
		uint8_t payload = 0;
		if(ui->lineEditFrPayloadlaenge->text().isEmpty() == true ){
			switch (filltype){
			case '1':
				ui->lineEditFrFrameID->setText("0x7FF");
				ui->lineEditFrPayloadlaenge->setText("0x7F");
				ui->lineEditFrCycleCount->setText("0x3F");
				ui->checkBoxFrNullFrameIndi->setChecked(1);
				ui->checkBoxFrStartupFrameIndi->setChecked(1);
				ui->checkBoxFrSyncFrameIndi->setChecked(1);
				ui->checkBoxFrPayloadPreambleIndi->setChecked(1);
				payload = 0x7F;
				break;
			case '0':
				ui->lineEditFrFrameID->setText("0x000");
				ui->lineEditFrPayloadlaenge->setText("0x00");
				ui->lineEditFrCycleCount->setText("0x00");
				ui->checkBoxFrNullFrameIndi->setChecked(0);
				ui->checkBoxFrStartupFrameIndi->setChecked(0);
				ui->checkBoxFrSyncFrameIndi->setChecked(0);
				ui->checkBoxFrPayloadPreambleIndi->setChecked(0);
				break;
			case 'r':
				//Frame-ID
				randomValue= QRandomGenerator::global()->bounded(0x800); // 0x800 ^= 0x0 - 0x7FF
				fillStr = QString::number(randomValue, 16);
				if(randomValue >= 0x100){				// Es werden immer 3 Stellen benötigt
					fillStr = "0x" + fillStr.toUpper();
				} else if (randomValue < 0x10){
					fillStr = "0x00" + fillStr.toUpper();
				} else {
					fillStr = "0x0" + fillStr.toUpper();
				}
				ui->lineEditFrFrameID->setText(fillStr);

				//Payloadlänge
				payload = QRandomGenerator::global()->bounded(0x80); // 0x80 ^= 0x00 - 0x7F
				fillStr = QString::number(payload, 16);
				if(payload >= 0x10){				// Es werden immer 2 Stellen benötigt
					fillStr = "0x" + fillStr.toUpper();
				} else {
					fillStr = "0x0" + fillStr.toUpper();
				}
				ui->lineEditFrPayloadlaenge->setText(fillStr);

				//Cycle-Count
				randomValue= QRandomGenerator::global()->bounded(0x40); // 0x40 ^= 0x00 - 0x3F
				fillStr = QString::number(randomValue, 16);
				if(randomValue >= 0x10){				// Es werden immer 2 Stellen benötigt
					fillStr = "0x" + fillStr.toUpper();
				} else {
					fillStr = "0x0" + fillStr.toUpper();
				}
				ui->lineEditFrCycleCount->setText(fillStr);

				//Steuerbits
				randomValue= QRandomGenerator::global()->bounded(2); // 0 oder 1
				ui->checkBoxFrNullFrameIndi->setChecked(randomValue);
				randomValue= QRandomGenerator::global()->bounded(2); // 0 oder 1
				ui->checkBoxFrStartupFrameIndi->setChecked(randomValue);
				randomValue= QRandomGenerator::global()->bounded(2); // 0 oder 1
				ui->checkBoxFrSyncFrameIndi->setChecked(randomValue);
				randomValue= QRandomGenerator::global()->bounded(2); // 0 oder 1
				ui->checkBoxFrPayloadPreambleIndi->setChecked(randomValue);
				break;
			default:
				break;
			}
		} else {
			payload = Calculator->getParseValue(ui->lineEditFrPayloadlaenge->text());
		}

		// PAYLOAD
		numberOfBits = 2 * payload * 8;
		ui->plainTextEditFR->setPlainText(generateFillString(filltype, numberOfBits));
		if( ui->lineEditFrPayloadlaenge->text() == "0x00" ){ui->plainTextEditFR->clear();}

	  }

}


//Textfelder-Eingaben haben Regeln - Bsp: Nur 4 Bits, nur volle bytes, auch vielfache davon.
//setzen wir hier fest.
void crc_main::getInputRules(QString sender, int *acceptBits, bool *acceptMult){
    //LIN
    if(sender == "lineEditLinID"){
        //Accepts 6 bits, nothing else
        *acceptBits = 6;
        *acceptMult = false;
    }
    else if(sender == "lineEditLinData"){
        //Datenfeld akzeptiert ganze bytes, also 8 bit oder vielfache.
        *acceptBits = 8;
        *acceptMult = true;
    }
    //Flexray
	else if (sender == "lineEditFrFrameID"){
		//Aceppts 11 Bits =^ 0x000 - 0x7FF in HEX
		*acceptBits = 11;
		*acceptMult = false;
	}
	else if (sender == "lineEditFrCycleCount"){
		*acceptBits = 6;
		*acceptMult = false;
	}
	else if (sender == "lineEditFrPayloadlaenge"){
		*acceptBits = 7;
		*acceptMult = false;
	}
	else if (sender ==  "plainTextEditFR"){
		*acceptBits = 16;
		*acceptMult = true;
	}
    //CAN Standard
    else if (sender == "lineEditCAN_Identifier"){
        *acceptBits = 11;
        *acceptMult = false;
    }
    else if (sender == "lineEditCAN_DLC"){
        *acceptBits = 4;
        *acceptMult = false;
    }
    else if (sender == "lineEditCAN_Payload"){
        *acceptBits = 8;
        *acceptMult = true;
    }
    //CAN FD
    //CAN XL

}

//Generates a string of specified length in hexadecimal, for example "0xFFFFFFFF" or "0x0A6F"
QString crc_main::generateFillString(int filltype, int numberOfBits){
    QChar hex_pool[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    QString result = "0x";
    for(int i = 0; i < numberOfBits; i+=4){
        QChar fillChar;
        switch(filltype){
        case '1':
            fillChar = 'F';
            break;
        case '0':
            fillChar = '0';
            break;
        case 'r':
            int randNum = m_qrand.bounded(16);
            fillChar = hex_pool[randNum];
        }
        result += fillChar;
    }
    return result;
}
