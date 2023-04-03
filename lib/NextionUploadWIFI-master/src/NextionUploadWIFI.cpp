/**
 * @file NextionUploadWIFI.cpp
 *
 * The implementation of download tft file for nextion. 
 *
 * @author  Chen Zengpeng (email:<zengpeng.chen@itead.cc>), Bogdan Symchych (email:<bogdan.symchych@gmail.com>)
 * @date    2019/10/1
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include "NextionUploadWIFI.h"


#define debugSerial Serial
#define nexSerial Serial2
#define nexSerialBegin(a, b, c) nexSerial.begin(a, SERIAL_8N1, b, c)

NextionUploadWIFI::NextionUploadWIFI(uint32_t upload_baudrate, gpio_num_t rx_pin, gpio_num_t tx_pin) {
    _upload_baudrate = upload_baudrate;
    _next_rx_pin = rx_pin;
    _next_tx_pin = tx_pin;
}

String NextionUploadWIFI::check(uint32_t size) {
    _undownloadByte = size;
    if(_getBaudrate() == 0) {
        debugSerial.println("get baudrate error");
        return "Get baudrate error";
    }
    if(!_setDownloadBaudrate(_upload_baudrate)) {
        debugSerial.println("modify baudrate error");
        return "Modify baudrate error";
    }
    return "0";
}

uint32_t NextionUploadWIFI::_getBaudrate(void) {
    uint32_t baudrate_array[7] = {115200,9600,19200,57600,38400,4800,2400};
    for(uint8_t i = 0; i < 7; i++) {
        if(_searchBaudrate(baudrate_array[i])) {
            _baudrate = baudrate_array[i];
            break;
        }
    }
    debugSerial.println("Baudrate is: " + String(_baudrate));
    return _baudrate;
}

bool NextionUploadWIFI::_searchBaudrate(uint32_t baudrate){
    String response = String("");
    nexSerialBegin(baudrate, _next_rx_pin, _next_tx_pin);
    this->sendCommand("");
    this->sendCommand("connect");
    this->recvRetString(response);
    if(response.indexOf(F("comok")) != -1) {
        return true;
    }
    nexSerial.end();
    return false;
}

void NextionUploadWIFI::sendCommand(const char* cmd) {
    nexSerial.print(cmd);
	nexSerial.write(0xFF);
	nexSerial.write(0xFF);
	nexSerial.write(0xFF);
}

uint16_t NextionUploadWIFI::recvRetString(String &response, uint32_t timeout, bool recv_flag) {
    uint16_t ret = 0;
    uint8_t c = 0;
    int32_t start;
    bool exit_flag = false;
    start = millis();
    while (millis() - start <= timeout) {
        while (nexSerial.available()) {
            c = nexSerial.read(); 
            if(c == 0) {
                continue;
            }
            response += (char)c;
            if(recv_flag) {
                if(response.indexOf(0x05) != -1) { 
                    exit_flag = true;
                } 
            }
        }
        if(exit_flag) {
            break;
        }
    }
    ret = response.length();
    return ret;
}

bool NextionUploadWIFI::_setDownloadBaudrate(uint32_t baudrate){
    String response = String(""); 
    String cmd = String("");
    String filesize_str = String(_undownloadByte,10);
    String baudrate_str = String(baudrate);
    cmd = "whmi-wri " + filesize_str + "," + baudrate_str + ",0";
    this->sendCommand("");
	this->sendCommand(cmd.c_str());
	nexSerial.flush();
	nexSerialBegin(baudrate, _next_rx_pin, _next_tx_pin);
    debugSerial.println("changing upload baudrate..." + String(baudrate));
	this->recvRetString(response, 500,true);
    if(response.indexOf(0x05) != -1) { 
        debugSerial.println(F("preparation for firmware update done"));
        return 1;
    }
    else { 
        debugSerial.println(F("preparation for firmware update failed"));
		return 0;
	}
}

String NextionUploadWIFI::uploadTftFile(uint8_t *file_buf, size_t buf_size) {
    String string = String("");
    uint8_t timeout = 0;
    for(uint16_t i = 0; i < buf_size; i++) {
        if (_uploaded_bytes == 4096) {
        this->recvRetString(string,500,true);  
            if(string.indexOf(0x05) != -1) { 
                _uploaded_bytes = 0;
                string = "";
            } 
            else {
                if(timeout >= 8) {
                    debugSerial.println("Connection lost");
                    return "Connection lost";
                }
                timeout++;
            }
            i--;
        }
        else {
		    nexSerial.write(file_buf[i]);
            _uploaded_bytes++;
	    }
    }
    return "0";
}

void NextionUploadWIFI::softReset(void){
    // soft reset nextion device
	this->sendCommand("rest");
}

void NextionUploadWIFI::end(){
    // wait for the nextion to finish internal processes 1600ms
    uint32_t currentMillis = millis();
    uint32_t waiting_time = 1600;
    while(millis() - currentMillis < waiting_time) {
        // waiting
    }
	// soft reset the nextion
	this->softReset();
    // end Serial connection
    nexSerial.end();
	// reset sent packets counter
	_uploaded_bytes = 0;
    debugSerial.println("Upload Ok");

}


