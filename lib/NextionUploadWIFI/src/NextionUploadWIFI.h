/**
 * @file NexUpload.h
 *
 * The definition of class NexUpload. 
 *
 * @author Chen Zengpeng (email:<zengpeng.chen@itead.cc>), Bogdan Symchych (email:<bogdan.symchych@gmail.com>)
 * @date 2019/10/1
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#ifndef __NEXTIONUPLOADWIFI_H__
#define __NEXTIONUPLOADWIFI_H__
#include <Arduino.h>

/**
 * @addtogroup CoreAPI 
 * @{ 
 */

/**
 *
 * Provides the API for nextion to download the ftf file.
 */
class NextionUploadWIFI
{
public: /* methods */

    /**
     * Constructor. 
     * 
     * @param upload_baudrate - set upload baudrate.
     * @param rx_pin - set serial rx pin.
     * @param tx_pin - set serial tx pin.
     */
    // Baudrate, RX_PIN, TX_PIN
    NextionUploadWIFI(uint32_t upload_baudrate, gpio_num_t rx_pin, gpio_num_t tx_pin);
    /**
     * destructor. 
     * 
     */
    ~NextionUploadWIFI(){}
    
    /*
     * start download.
     *
     * @return none.
     */
    String check(uint32_t size);

    /**
     * start dowload tft file to nextion. 
     * 
     * @return none. 
     */
    bool uploadTftFile(uint8_t *file_buf, size_t buf_size);
    /**
     * Send reset command to Nextion over serial
     *
     * @return none.
     */
	void softReset(void);

    /**
     * Send reset, end serial, reset _sent_packets & update status message
     *
     * @return none.
     */
	void end(void);
    
private: /* methods */

    /*
     * get communicate baudrate. 
     * 
     * @return communicate baudrate.
     *
     */
    uint32_t _getBaudrate(void);
    
    /*
     * check tft file.
     *
     * @return true if success, false for failure. 
     */
    bool _checkFile(void);

    /*
     * search communicate baudrate.
     *
     * @param baudrate - communicate baudrate.
     *   
     * @return true if success, false for failure. 
     */
    bool _searchBaudrate(uint32_t baudrate);

    /*
     * set download baudrate.
     *
     * @param baudrate - set download baudrate.
     *   
     * @return true if success, false for failure. 
     */
    bool _setDownloadBaudrate(uint32_t baudrate);
    
    /*
     * Send command to Nextion.
     *
     * @param cmd - the string of command.
     *
     * @return none.
     */
    void sendCommand(const char* cmd);
    //void sendCommand(const char* cmd, bool tail = true, bool null_head = false);
    /*
     * Receive string data. 
     * 
     * @param buffer - save string data.  
     * @param timeout - set timeout time. 
     * @param recv_flag - if recv_flag is true,will braak when receive 0x05.
     *
     * @return the length of string buffer.
     *
     */   
    uint16_t recvRetString(String &response, uint32_t timeout = 100,bool recv_flag = false);


private: /* data */ 
    uint32_t _baudrate; /*nextion serail baudrate*/
    uint32_t _undownloadByte; /*undownload byte of tft file*/
    uint32_t _upload_baudrate; /*upload baudrate*/
    uint32_t _uploaded_bytes = 0; /*counter of uploaded bytes*/
    gpio_num_t _next_rx_pin;
    gpio_num_t _next_tx_pin;
};
/**
 * @}
 */

#endif /* #ifndef __NEXDOWNLOAD_H__ */
