#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <memory>
#include "Maxim14574.h"

const unsigned int focusReg0 = 0x00;
const unsigned int focusReg1 = 0x01;
const unsigned int controlReg = 0x02;
const unsigned int modeReg = 0x03;
const unsigned int swVersionReg = 0x05;
const unsigned int falutReg = 0x0A;

const unsigned int regInteractCount = 0x01;
const unsigned int stx = 0x02;
const unsigned int writeCommand = 0x37;
const unsigned int readCommand = 0x38;

const unsigned int clearMask = 0x00;
const unsigned int controlMask = 0x01;
const unsigned int standbyMask = 0x1;
const unsigned int analogMask = 0x2;
const unsigned int overloadMask = 0x1;
const unsigned int notRespondingMask = 0x2;
const unsigned int thermalShutdownMask = 0x4;

const unsigned int lsbMask = 0xFF;
const unsigned int msbMask = 0xFF00;

const unsigned int ack = 0x06;
const unsigned int nack = 0x15;

const unsigned int readBufSize = 64;
const unsigned int byteShift = 8;

const unsigned int ackRespLength = 4;

MAX14574Driver::MAX14574Driver(char* path)
    : port(new SerialPort(path)) 
{
    port->configureSerialPort();
}

void MAX14574Driver::setValue(unsigned short value) {

    writeRegister16(focusReg0, value);
}

void MAX14574Driver::setStandbyMode() {
    
    if (isAnalogMode()) {
        writeRegister8(modeReg, clearMask);
    }

    writeRegister8(modeReg, standbyMask);
}

void MAX14574Driver::setAnalogMode() {

    if (isStandbyMode()) {
        writeRegister8(modeReg, clearMask);
    }

    writeRegister8(modeReg, analogMask);
}

bool MAX14574Driver::isStandbyMode() {
    
    return ( (readRegister(modeReg) & standbyMask) != 0 );
}

bool MAX14574Driver::isAnalogMode() {
    
    return ( (readRegister(modeReg) & analogMask) != 0 );
}

void MAX14574Driver::clearModes() {

    writeRegister8(modeReg, clearMask);
}

void MAX14574Driver::saveState() {
    
    writeRegister8(controlReg, controlMask);
}

int MAX14574Driver::softwareVersion() {

    return (int) readRegister(swVersionReg);
}

bool MAX14574Driver::isOverloaded() {
    
    return ( (readRegister(falutReg) & overloadMask) != 0 );
}

bool MAX14574Driver::isNotResponding() {
    
    return ( (readRegister(falutReg) & notRespondingMask) != 0 );
}

bool MAX14574Driver::isThermalShutdown() {
    
    return ( (readRegister(falutReg) & thermalShutdownMask) != 0 );
}

void MAX14574Driver::writeRegister8(unsigned char addr, unsigned char data) {
    writeFrame(addr, (unsigned short)data, 1);
}

void MAX14574Driver::writeRegister16(unsigned char addr, unsigned short data) {
    writeFrame(addr, data, 2);
}

void MAX14574Driver::writeFrame(unsigned char addr, unsigned short data, unsigned int dataLength) {
    
    std::vector <unsigned char> message;
    unsigned writeFrameLength = 6;
    unsigned checksum = 0;

    message.push_back(stx);
    message.push_back(writeCommand);
    message.push_back(addr);
    message.push_back(dataLength);
    
    checksum = message.at(0) + message.at(1) + message.at(2) + message.at(3);
    
    if (dataLength == 1) {
        message.push_back((unsigned char)data);
        checksum += message.at(4);
    }
    else {
        message.push_back((unsigned char)data);
        message.push_back((unsigned char)(data >> byteShift));
        checksum += message.at(4);
        checksum += message.at(5);
        writeFrameLength++;
    }   

    message.push_back(checksum);

    port->writeMessage(message);
    std::vector<unsigned char> readBuf = readFrame();

    if (readBuf.at(2) != ack || readBuf.size() != ackRespLength)
        throw std::runtime_error("Failed to read acknowledgement from the driverboard.");
    if (readBuf.at(2) == nack)
        throw std::runtime_error("The driverboard sent a nack.");
}

std::vector<unsigned char> MAX14574Driver::readFrame() {
    
    std::vector<unsigned char> readBuf;
    readBuf.resize(readBufSize);

    int retVal = port->readMessage(&readBuf);
    
    readBuf.resize(retVal);
    checkChecksum(readBuf);
    return readBuf;
}

unsigned char MAX14574Driver::readRegister(unsigned char addr) {

    std::vector <unsigned char> message;

    message.push_back(stx);   // constant
    message.push_back(readCommand);
    message.push_back(addr);
    message.push_back(regInteractCount);  // only read one register

    unsigned int checksum = message.at(0) + message.at(1) + message.at(2) + message.at(3);
    message.push_back(checksum); // checksum
    
    port->writeMessage(message);

    std::vector <unsigned char> readBuf = readFrame();

    return readBuf.at(2);
}

void MAX14574Driver::checkChecksum(std::vector <unsigned char> buffer) {
    
    unsigned checksum = 0;

    for (unsigned char character : buffer) {
        checksum += character;
    }
    checksum -= buffer.at(buffer.size() - 1);

    if(checksum != buffer.at(buffer.size() - 1))
        throw(std::runtime_error("Checksum of received message does not match."));
}

static void printMessage(std::vector<unsigned char> message) {
    for (unsigned char character : message)
        printf("%#10x, ", character);
    
    std::cout << std::endl;
}
