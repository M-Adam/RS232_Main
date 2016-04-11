#include <iostream>
#include <string>
#include <fstream>
#include <Windows.h>

using namespace std;

bool portOpen(LPCTSTR);
bool setDCB(unsigned long, int, unsigned long, int, int);    // Ustawienie DCB
bool setCommTimeouts(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long); //Ustawienie CommTimeouts
bool portSend(int); // Wysylanie danych
bool portRecieve(int&); // Odbieranie danych
bool portClose(HANDLE); // COMMTIMEOUTS - ustawienia

void showDCB();   // Wyswietlenie DCB
void showHandleError(HANDLE);   // Obsluga bledu przy otwieraniu portu
void setCommProp();			//COMMPROP
void setCommConfig();	//COMMCONFIG 


LPCTSTR commName = L"COM3"; // L"COM3";
HANDLE commHandle;
DCB dcb;
COMMPROP commprop;
COMMCONFIG commconfig;
COMMTIMEOUTS commtimeouts;
char chOut;
int buffer;


int main() {

// Otwacie portu
	if(portOpen(commName))
		cout << "Udalo sie otworzyc port" << endl ;
	else{
		cout << "Nie udalo sie otworzyc portu" << endl;
		return 1;
	}

// DCB
	if (setDCB(CBR_9600, 8, true, NOPARITY, ONESTOPBIT))
		cout << "Ustawiono DCB" << endl;
	else {
		cout << "Nie udalo sie ustawic DCB" << endl;
		CloseHandle(commHandle);
		return 2;
	}

// COMMTIMEOUTS
	if (setCommTimeouts(0, 0, 0, 0, 0))
		cout << "Ustawiono COMMTIMEOUTS" << endl;
	else {
		cout << "Nie udalo sie ustawic COMMTIMEOUTS" << endl;
		CloseHandle(commHandle);
		return 3;
	}

// Wysy³anie zawartosci pliku
	const char* filePath = "rysunek.txt";
	ifstream ifs(filePath);
	char znaczek;
	int count = 0;
	
	while (ifs.get(znaczek)) {
		if (portSend(znaczek)) 
			count++;
		else
			cout << "Nie udalo sie wyslac znaku" << endl;
	}

	cout << "Wyslano " << count << " znakow.\nOdebrano: " << endl;

	while (count > 0) {
		portRecieve(buffer);
		Sleep(3);
		count--;
	}
	cout << endl;

	ifs.close();
	cout << "Koniec odbierania zawartosci pliku" << endl;


// Wyslanie danych
	count = 0;
	cout << "Nadawanie, wpisz znaki i nacisnij ENTER: ";
	while ((chOut = getchar()) != '\n')
	{
		if (portSend(chOut)) 
			count++;
		else 
			cout << "Nie udalo sie wyslac znaku" << endl;
	}
	
	cout << "Wyslano "<< count <<" znakow.\nOdebrano: " << endl;
	while (count > 0) {
		portRecieve(buffer);
		count--;
	}
	cout << endl;
	
	
// Zamkniecie portu
	if (portClose(commHandle))
		cout << "Zamknieto port" << endl;
	else {
		cout << "Nie udalo sie zamknac portu" << endl;
		return 6;
	}


	cin.ignore();
	return 0;
}

// OTWARCIE PORTU
bool portOpen(LPCTSTR commName) {
	commHandle = CreateFile(commName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);

	if (commHandle == INVALID_HANDLE_VALUE) {
		cout << "Nie udalo sie otworzyc portu." << endl;
		showHandleError(commHandle);
		return false;
	}
	
	return true;
}

// USTAWIENIE DCB
bool setDCB(unsigned long BaudRate, int ByteSize, unsigned long fParity, int Parity, int StopBits) {
	if (GetCommState(commHandle, &dcb))
		cout << "Pobrano ustawienia portu do DCB" << endl;
	else
		cout << "Nie udalo sie pobrac ustawien portu do DCB" << endl;
	cout << endl;

	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = BaudRate;
	dcb.ByteSize = ByteSize;
	dcb.fParity = fParity;
	dcb.Parity = Parity;
	dcb.StopBits = StopBits;

	dcb.fBinary = true;
	dcb.fDsrSensitivity = false;
	dcb.fOutX = false;
	dcb.fInX = false;
	dcb.fNull = false;
	dcb.fAbortOnError = true;
	dcb.fOutxCtsFlow = false;
	dcb.fOutxDsrFlow = false;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = false;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fOutxCtsFlow = false;
	dcb.fOutxCtsFlow = false;

	if (!SetCommState(commHandle, &dcb))
		return false;
	return true;
}

// USTAWIENIE COMMTIMEOUTS
bool setCommTimeouts(unsigned long ReadIntervalTimeout, unsigned long ReadTotalTimeoutMultiplier, unsigned long ReadTotalTimeoutConstant, unsigned long WriteTotalTimeoutMultiplier, unsigned long WriteTotalTimeoutConstant) {
	if (!GetCommTimeouts(commHandle, &commtimeouts))
		return false;

	commtimeouts.ReadIntervalTimeout = ReadIntervalTimeout;
	commtimeouts.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier;
	commtimeouts.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant;
	commtimeouts.WriteTotalTimeoutMultiplier = WriteTotalTimeoutMultiplier;
	commtimeouts.WriteTotalTimeoutConstant = WriteTotalTimeoutConstant;

	if (!SetCommTimeouts(commHandle, &commtimeouts)) 
		return false;

	return true;
}

// WYSYLANIE DANYCH
bool portSend(int buffer) {
	unsigned long numberOfBytesWritten = 0;
	unsigned long numberOfBytesToWrite = 1;
	if (WriteFile(commHandle, &buffer, numberOfBytesToWrite, &numberOfBytesWritten, NULL))
		return true;
	else
		return false;
}

// ODBIERANIE DANYCH
bool portRecieve(int &numberOfBytesRead) {
	int bytesRead;
	numberOfBytesRead = 0;
	unsigned long numberOfBytesToRead = 0;
	if(ReadFile(commHandle, &bytesRead, 1, &numberOfBytesToRead, 0))
		if (numberOfBytesToRead == 1) {
			numberOfBytesRead = bytesRead;
			cout << (char)numberOfBytesRead;
			return true;
		}
	return false;
}

// ZAMKNIECIE PORTU
bool portClose(HANDLE commhandle) {
	if (CloseHandle(commHandle))
		return true;
	else
		return false;
}

// OBLSUGA BLEDU PRZY OTWIERANIU PORTU
void showHandleError(HANDLE commHandle) {
	cout << "Kod bledu:" << endl;
	switch ((int)commHandle) {
	case -1:
		cout << "Bledna identyfiakcja portu." << endl;
		break;
	case -12:
		cout << "Blednie okreslona szybkosc transmisji." << endl;
		break;
	case -11:
		cout << "Blednie okreslona liczba bitow danych." << endl;
		break;
	case -5:
		cout << "Blednie okreslone parametry domyslne urzadzenia." << endl;
		break;
	case -10:
		cout << "Odbiornik zablokowany." << endl;
		break;
	case -4:
		cout << "Niewlasciwe rozmiary buforow." << endl;
		break;
	case -3:
		cout << "Urzadzenie nie jest otwarte do transmisji." << endl;
		break;
	case -2:
		cout << "Urzadzenie pozostaje otwarte." << endl;
		break;
	default:
		cout << "Nieokreslony kod bledu." << endl;
		break;
	}
}

// WYSWIETLENIE DCB
void showDCB() {
	cout << "Predkosc portu: " << dcb.BaudRate << endl;

	cout << "Rozmiar bitow danych: " << (int)dcb.ByteSize << endl;

	cout << "Parzystosc: ";
	switch (dcb.Parity)
	{
	case NOPARITY:
		cout << "Brak";
		break;
	case ODDPARITY:
		cout << "Nie parzysta";
		break;
	case EVENPARITY:
		cout << "Parzysta";
		break;
	case MARKPARITY:
		cout << "Znacznik: 1";
		break;
	}
	cout << endl;

	cout << "Bity stopu: ";
	switch (dcb.StopBits)
	{
	case ONESTOPBIT:
		cout << "1";
		break;
	case TWOSTOPBITS:
		cout << "2";
		break;
	case ONE5STOPBITS:
		cout << "1.5";
		break;
	}
	cout << endl;

	cout << "DTR Controll:";
	switch (dcb.fDtrControl)
	{
	case DTR_CONTROL_DISABLE:
		cout << "Nieaktywna";
		break;
	case DTR_CONTROL_ENABLE:
		cout << "Aktywna";
		break;
	case DTR_CONTROL_HANDSHAKE:
		cout << "Handshaking";
		break;
	}
	cout << endl;
}

// COMMPROP
void setCommProp(){
	commprop.wPacketLength = sizeof(commprop);
	commprop.dwProvSpec1 = COMMPROP_INITIALIZED;
	memset(&commprop, 0, sizeof(commprop));
	if (GetCommProperties(commHandle, &commprop))
		cout << endl << "Pobrano wlasciwosci portu do COMMPROP" << endl;
	else
		cout << endl << "Nie udalo sie pobrac wlasciwosci portu do COMMPROP" << endl;
}

// COMMCONFIG
void setCommConfig() {
	commconfig.dwSize = sizeof(commconfig);
	if (GetCommConfig(commHandle, &commconfig, &commconfig.dwSize)) {
		cout << endl << "Pobrano konfiguracje lacza do COMMCONFIG" << endl;
		CommConfigDialog(commName, NULL, &commconfig);
	}
	else
		cout << endl << "Nie udalo sie pobrac konfiguracji lacza" << endl;
}


