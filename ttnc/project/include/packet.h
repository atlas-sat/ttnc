#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdbool.h>
// #include <csp.h>
#include <uart.h>

#define MAX_TC_DATA_SIZE 20
#define MAX_TM_DATA_SIZE 20
#define MAX_PACKET_LENGTH 65542

// ack struct
// typedef struct {
//     uint8_t compl_exe: 1;
//     uint8_t progr_exe: 1;
//     uint8_t start_exe: 1;
//     uint8_t accep_pac: 1;
// } Ack; // 4 bits

//#pragma pack(push, 1)

// time struct
typedef struct {
    int c1: 8; // 256^3
    int c2: 8; // 256^2
    int c3: 8; // 256^1
    int c4: 8; // 256^0
    int f1: 8; // 256^-1
} Abs_time; // 40 bits

// Packet Header

typedef struct {
    uint8_t version_num: 3;
    uint8_t type: 1;
    uint8_t data_field_header_flags: 1;
    uint16_t APID: 11;
} Packet_ID; // 16 bits

typedef struct {
    uint8_t seq_flags: 2;
    uint16_t seq_count: 14;
} Packet_seq_control; // 16 bits

typedef struct {
    Packet_ID p_ID;
    Packet_seq_control p_seq_control;
    uint16_t p_length: 16;
} Packet_header; // 48 bits

// Packet Data Field

typedef struct {
    bool sec_header_flag: 1;
    uint8_t PUS_version: 3;
    uint8_t ack: 4;
    uint8_t service_type: 8;
    uint8_t service_subtype: 8;
} TC_data_field_header; // 24 bits

typedef struct {
    uint8_t spare_1: 1;
    uint8_t PUS_version: 3;
    uint8_t spare_2: 4; // how to name vars?
    uint8_t service_type: 8;
    uint8_t service_subtype: 8;
    Abs_time absolute_time; // 40 bits
} TM_data_field_header; // 64 bits

typedef struct {
    TC_data_field_header data_field_header;
    uint8_t app_data[20];
    uint16_t checksum: 16;
} TC_packet_data_field; // 40 + (MAX_TC_DATA_SIZE * 8) bits

typedef struct {
    TM_data_field_header data_field_header;
    uint8_t src_data[MAX_TM_DATA_SIZE];
    uint16_t checksum: 16;
} TM_packet_data_field; // 80 + (MAX_DATA)

// Packets

typedef struct {
    Packet_header header;
    TC_packet_data_field data;
} TC_packet;

typedef struct {
    Packet_header header;
    TM_packet_data_field data;
} TM_packet;

//#pragma pack(pop)

// init methods
void packet_ID_init(Packet_ID* p);
void packet_seq_control_init(Packet_seq_control* p);
void packet_header_init(Packet_header* p);
void TC_data_field_header_init(TC_data_field_header* p);
void TM_data_field_header_init(TM_data_field_header* p);
void TC_packet_data_field_init(TC_packet_data_field* p);
void TM_packet_data_field_init(TM_packet_data_field* p);
void TC_packet_init(TC_packet* p);
void TM_packet_init(TM_packet* p);

// setters
void set_application_data(TC_packet* p, void* content, int content_length);
void set_source_data(TM_packet* p, char* content, int content_length);

void set_TC_checksum(TC_packet* p);
void set_TM_checksum(TM_packet* p);

void set_type(Packet_header* h, int type);

void set_TM_packet_length(TM_packet* p);
void set_TC_packet_length(TC_packet* p);

void set_APID(Packet_header* p, int APID);

void set_ack(TC_packet* p, uint8_t ack);

void set_TC_service(TC_packet* p, int service, int subservice);
void set_TM_service(TM_packet* p, int service, int subservice);

void set_TM_time(TM_packet* p, Abs_time absolute_time);

#endif /* PACKET_H */