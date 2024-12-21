#include "project/include/packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <uart.h>

// Generic zero-initialization function for structs
void init_struct_to_zero(void* p, size_t size) {
    memset(p, 0, size); // Set all bytes of the struct to 0
}

void packet_ID_init(Packet_ID* p) {
    init_struct_to_zero(p, sizeof(*p));
    p->version_num = 0;
    p->data_field_header_flags = 1;
}

void packet_seq_control_init(Packet_seq_control* p) {
    init_struct_to_zero(p, sizeof(*p));
    p->seq_flags = 3; // 0b11
}

void packet_header_init(Packet_header* p) {
    init_struct_to_zero(p, sizeof(*p));
    packet_ID_init(&(p->p_ID));
    packet_seq_control_init(&(p->p_seq_control));
}

void TC_data_field_header_init(TC_data_field_header* p) {
    init_struct_to_zero(p, sizeof(*p));
    p->sec_header_flag = 0;
    p->PUS_version = 1;
}

void TM_data_field_header_init(TM_data_field_header* p) {
    init_struct_to_zero(p, sizeof(*p));
    p->spare_1 = 0;
    p->PUS_version = 1;
    p->spare_2 = 0;
}

void TC_packet_data_field_init(TC_packet_data_field* p) {
    init_struct_to_zero(p, sizeof(*p));
    TC_data_field_header_init(&(p->data_field_header));
}

void TM_packet_data_field_init(TM_packet_data_field* p) {
    init_struct_to_zero(p, sizeof(*p));
    TM_data_field_header_init(&(p->data_field_header));
}

void TC_packet_init(TC_packet* p) {
    init_struct_to_zero(p, sizeof(*p));
    packet_header_init(&(p->header));
    TC_packet_data_field_init(&(p->data));
    p->header.p_ID.type = 1;
}

void TM_packet_init(TM_packet* p) {
    init_struct_to_zero(p, sizeof(*p));
    packet_header_init(&(p->header));
    TM_packet_data_field_init(&(p->data));
    p->header.p_ID.type = 0;
}


// setters

void set_application_data(TC_packet* p, void* content, int content_length) {
    memcpy(&(p->data.app_data), content, content_length);
}

void set_source_data(TM_packet* p, char* content, int content_length) {
    memcpy(p->data.src_data, content, content_length);
}

void set_TC_checksum(TC_packet* p) {
    unsigned char *p_bytes = (unsigned char *)p;
    unsigned char checksum = 0x00;
    for (size_t i = 0; i < sizeof(TC_packet) - 2; i++) {// minus 2 to remove xor-ing empty checksum bytes
        checksum ^= p_bytes[i];
    }
    //printf("%d\r\n", checksum);
    p->data.checksum = checksum;
}

void set_TM_checksum(TM_packet* p) {
    unsigned char *p_bytes = (unsigned char *)p;
    unsigned char checksum = 0x00;
    for (size_t i = 0; i < sizeof(TC_packet) - 2; i++) {// minus 2 to remove xor-ing empty checksum bytes
        checksum ^= p_bytes[i];
    }
    //printf("%d\r\n", checksum);
    p->data.checksum = checksum;
}

void set_type(Packet_header* h, int type) {
    //h->p_ID.type = type;
}

// void set_TC_packet_length(Packet_header* h, int length) {
//     // should make a packet 
//     if (length <= MAX_PACKET_LENGTH) {
//         h->p_length = length;
//     }
//     else {
//         printf("packet too long");
//     }
// }

void set_TM_packet_length(TM_packet* p) {
    int length = 80 + MAX_TM_DATA_SIZE; // size always the same since array has static size
    p->header.p_length = length;
}

void set_TC_packet_length(TC_packet* p) {
    int length = 40 + MAX_TC_DATA_SIZE;
    p->header.p_length = length;
}

void set_APID(Packet_header* p, int APID) {
    p->p_ID.APID = APID;
}

void set_ack(TC_packet* p, uint8_t ack){
    p->data.data_field_header.ack = ack;
}

void set_TC_service(TC_packet* p, int service, int subservice) {
    p->data.data_field_header.service_type = service;
    p->data.data_field_header.service_subtype = subservice;
}

void set_TM_service(TM_packet* p, int service, int subservice) {
    p->data.data_field_header.service_type = service;
    p->data.data_field_header.service_subtype = subservice;
}

void set_TM_time(TM_packet* p, Abs_time absolute_time) {
    // stuff here to get time on function call, arg: absolute_time is temporary
    p->data.data_field_header.absolute_time = absolute_time;
}