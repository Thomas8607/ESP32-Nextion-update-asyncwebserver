/**
 * @file NexUploadWIFI.cpp
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

#include "NexUploadWIFI.h"

#define nexSerial Serial2
#define debugSerial Serial

NexUploadWIFI::NexUploadWIFI(uint32_t download_baudrate) {
    _download_baudrate = download_baudrate;
}

String NexUploadWIFI::check(uint32_t size) {
    _undownloadByte = size;
    if(_getBaudrate() == 0) {
        debugSerial.println("get baudrate error");
        return "Get baudrate error";
    }
    if(!_setDownloadBaudrate(_download_baudrate)) {
        debugSerial.println("modify baudrate error");
        return "Modify baudrate error";
    }
    return "0";
}

uint16_t NexUploadWIFI::_getBaudrate(void) {
    uint32_t baudrate_array[5] = {115200,57600,38400,19200,9600};
    for(uint8_t i = 0; i < 5; i++) {
        if(_searchBaudrate(baudrate_array[i])) {
            _baudrate = baudrate_array[i];
            break;
        }
    }
    debugSerial.println("Baudrate is: " + String(_baudrate));
    return _baudrate;
}

bool NexUploadWIFI::_searchBaudrate(uint32_t baudrate) {
    String response = String("");  
    nexSerial.begin(baudrate);
    this->sendCommand("");
    this->sendCommand("connect");
    this->recvRetString(response);  
    if(response.indexOf("comok") == -1 && response[0] != 0x1A) {
        return false;
    } 
    return true;
}

void NexUploadWIFI::sendCommand(const char* cmd) {
    while (nexSerial.available()) {
        nexSerial.read();
    }
    nexSerial.print(cmd);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
}

uint16_t NexUploadWIFI::recvRetString(String &response, uint32_t timeout, bool recv_flag) {
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

bool NexUploadWIFI::_setDownloadBaudrate(uint32_t baudrate) {
    String response = String(""); 
    String cmd = String("");
    String filesize_str = String(_undownloadByte,10);
    String baudrate_str = String(baudrate,10);
    cmd = "whmi-wri " + filesize_str + "," + baudrate_str + ",0";
    this->sendCommand("");
    this->sendCommand(cmd.c_str());
    nexSerial.flush();
    nexSerial.begin(baudrate);
    this->recvRetString(response,500);  
    if(response.indexOf(0x05) != -1) { 
        return true;
    } 
    return false;
}

String NexUploadWIFI::uploadTftFile(uint8_t *data, size_t len) {
    String string = String("");
    uint8_t timeout = 0;
    for(uint16_t i = 0; i < len; i++) {
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
		    nexSerial.write(data[i]);
            _uploaded_bytes++;
	    }
    }
    return "0";
}


