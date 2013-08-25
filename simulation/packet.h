/**
 * License: MIT
 * Copyright (c) 2013 Gerard Krol
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef WANDERLUST_PACKET_H
#define WANDERLUST_PACKET_H

// Note: all data is stored in network byte order (big endian)
class Pubkey {
public:
    uint8_t data[32];
    bool operator<(const Pubkey &other) const {
        return memcmp(data, other.data, sizeof(data)) < 0;
    }
    bool operator!=(const Pubkey &other) const {
        return memcmp(data, other.data, sizeof(data))!=0;
    }
    bool operator==(const Pubkey &other) const {
        return memcmp(data, other.data, sizeof(data))==0;
    }
    uint16_t getShortId() const {
        return *(uint16_t*)data;
    }
    void setShortId(uint16_t id) {
       *(uint16_t*)data = id;
    }
};

class Location {
public:
    uint8_t data[16];
    bool operator!=(const Location &other) const {
        return (memcmp((void*)data, (void*)other.data, sizeof(data))!=0);
    }
};

typedef struct {
    uint8_t data[64];
} signature_t;

typedef struct {
    uint8_t version;
    uint8_t header_length;
    uint8_t message_type;
    uint8_t hop_limit;
    uint8_t flags;
    uint8_t flow_id1;
    uint8_t flow_id2;
    uint8_t flow_id3;
    Pubkey src_pubkey;
    Pubkey dst_pubkey;
    Location src_location;
    Location dst_location;
    signature_t signature;
} wanderlust_header_t;

enum {
    WANDERLUST_TYPE_DATA=0,
    WANDERLUST_TYPE_SWAP_REQUEST,
    WANDERLUST_TYPE_SWAP_RESPONSE,
    WANDERLUST_TYPE_SWAP_CONFIRMATION,
    WANDERLUST_TYPE_SWAP_REFUSAL,
    WANDERLUST_TYPE_LOCATION_QUERY,
    WANDERLUST_TYPE_LOCATION_ANSWER,
    WANDERLUST_TYPE_HELLO,
    WANDERLUST_TYPE_PING,
    WANDERLUST_TYPE_PONG
};

#endif
