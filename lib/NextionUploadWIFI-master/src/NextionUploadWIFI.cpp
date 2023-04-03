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
/*
uint16_t NextionUploadWIFI::_getBaudrate(void){
    _baudrate = 0;
    uint32_t baudrate_array[2] = {115200,9600};
    uint32_t previousMillis = 0;
    const uint32_t interval = 500; // wait for 500ms
    uint8_t i = 0;
    while (i < 2 && _baudrate == 0) {
        uint32_t currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            if(_searchBaudrate(baudrate_array[i])) {
                _baudrate = baudrate_array[i];
            }
            i++;
        }
    }
    debugSerial.println("Baudrate is: " + String(_baudrate));
    return _baudrate;
}
*/
uint16_t NextionUploadWIFI::_getBaudrate(void){
    _baudrate = 0;
    uint32_t baudrate_array[7] = {115200,19200,9600,57600,38400,4800,2400};
    for(uint8_t i = 0; i < 7; i++) {
        if(_searchBaudrate(baudrate_array[i])) {
            _baudrate = baudrate_array[i];
            break;
        }
		//delay(1500); // wait for 1500 ms
    }
    debugSerial.println("Baudrate is: " + String(_baudrate));
    return _baudrate;
}

bool NextionUploadWIFI::_searchBaudrate(uint32_t baudrate){
    String response = String("");
    Serial2.begin(9600, SERIAL_8N1, GPIO_NUM_41, GPIO_NUM_42);
    nexSerialBegin(baudrate, _next_rx_pin, _next_tx_pin);
    const char _nextion_FF_FF[3] = {0xFF, 0xFF, 0x00};
    this->sendCommand("DRAKJHSUYDGBNCJHGJKSHBDN");
    this->sendCommand("",true,true);                    // 0x00 0xFF 0xFF 0xFF
    this->recvRetString(response);
    this->sendCommand("connect");                       // first connect attempt
    this->recvRetString(response);
    response = String("");
    uint32_t currentMillis = millis();
    uint32_t waiting_time = 110;
    while(millis() - currentMillis < waiting_time) {
        // waiting
    }
    this->sendCommand(_nextion_FF_FF, false);
    this->sendCommand("connect");                       // second attempt
    this->recvRetString(response);
    if(response.indexOf(F("comok")) == -1 && response[0] != 0x1A) {
        return false;
    }
    return true;
}

void NextionUploadWIFI::sendCommand(const char* cmd, bool tail, bool null_head) {
	if(null_head) {
		nexSerial.write(0x00);
	}
    while(nexSerial.available()) {
        nexSerial.read();
    }
    nexSerial.print(cmd);
    if(tail) {
		nexSerial.write(0xFF);
		nexSerial.write(0xFF);
		nexSerial.write(0xFF);
	}
}

uint16_t NextionUploadWIFI::recvRetString(String &response, uint32_t timeout, bool recv_flag) {
    uint16_t ret = 0;
    uint8_t c = 0;
    uint8_t nr_of_FF_bytes = 0;
    long start;
    bool exit_flag = false;
    bool ff_flag = false;
    start = millis();
    while (millis() - start <= timeout) {
        while (nexSerial.available()) {
            c = nexSerial.read(); 
            if(c == 0) {
                continue;
            }
            if (c == 0xFF)
				nr_of_FF_bytes++;
			else {
				nr_of_FF_bytes=0;
				ff_flag = false;
			}
			if(nr_of_FF_bytes >= 3) {
				ff_flag = true;
            }
            response += (char)c;
            if(recv_flag) {
                if(response.indexOf(0x05) != -1) { 
                    exit_flag = true;
                } 
            }
        }
        if(exit_flag || ff_flag) {
            break;
        }
    }
	if(ff_flag) {
		response = response.substring(0, response.length() -3);
    }
    ret = response.length();
    return ret;
}

bool NextionUploadWIFI::_setDownloadBaudrate(uint32_t baudrate){
    String response = String(""); 
    String cmd = String("");
	cmd = F("00");
	this->sendCommand(cmd.c_str());
	this->recvRetString(response, 800,true); // normal response time is 400ms
    String filesize_str = String(_undownloadByte,10);
    String baudrate_str = String(baudrate);
    cmd = "whmi-wri " + filesize_str + "," + baudrate_str + ",0";
	this->sendCommand(cmd.c_str());
	// Without flush, the whmi command will NOT transmitted by the ESP in the current baudrate
	// because switching to another baudrate (nexSerialBegin command) has an higher prio.
	// The ESP will first jump to the new 'upload_baudrate' and than process the serial 'transmit buffer'
	// The flush command forced the ESP to wait until the 'transmit buffer' is empty
	nexSerial.flush();
	nexSerialBegin(baudrate, _next_rx_pin, _next_tx_pin);
    debugSerial.println("changing upload baudrate..." + String(baudrate));
	this->recvRetString(response, 800,true); // normal response time is 400ms
	// The Nextion display will, if it's ready to accept data, send a 0x05 byte.
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


