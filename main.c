#include <stdio.h>
#include <stdint.h>

const uint8_t sync_byte = 0x47;

struct ts_packet_adaptation_field {
    uint8_t length;

    uint8_t discontinuity_indicator;
    uint8_t random_access_indicator;
    uint8_t elementary_stream_priority_indicator;
    uint8_t pcr_flag;
    uint8_t opcr_flag;
    uint8_t splicing_point_flag;
    uint8_t transport_private_data_flag;
    uint8_t adaptation_field_extension_flag;
};

struct ts_packet {
    uint8_t transport_error_indicator;
    uint8_t payload_unit_start_indicator;
    uint8_t transport_priority;
    uint16_t pid;
    uint8_t transport_scrambling_control;
    uint8_t adaptation_field_control;
    uint8_t continuity_counter;
    struct ts_packet_adaptation_field adaptation_field;
    uint8_t data_length;
    uint8_t* data_byte;
};

const char *ts = "/home/mjkim/playground/hls/Mirae_435MHz.ts";


int program_map_parser(uint8_t *buf, struct program_map_table *pmt)
{
    int i;
    uint8_t offset = 0;

    uint8_t table_id = buf[offset];
    offset += 1;

    uint8_t section_syntax_indicator                = (buf[offset] & 0x80) >> 7;
    uint16_t section_length                         = (buf[offset] & 0x0f) << 8 | buf[offset+1];
    offset += 2;

    uint16_t program_number                         = (buf[offset] & 0xff) << 8 | buf[offset+1];
    offset += 2;

    uint8_t version_number                          = (buf[offset] & 0x1e) >> 1;
    uint8_t current_next_indicator                  = (buf[offset] & 0x01);
    offset += 1;

    uint8_t section_number                          = (buf[offset] & 0xff);
    offset += 1;

    uint8_t last_section_number                     = (buf[offset] & 0xff);
    offset += 1;

    uint16_t pcr_pid                                = ((buf[offset] & 0x1f) << 8) | (buf[offset+1] & 0xff);
    offset += 2;

    uint16_t program_info_length                    = ((buf[offset] & 0x0f) << 8) | (buf[offset+1] & 0xff);

    // program description

    offset += program_info_length;

}


uint8_t adaptation_field_parser(uint8_t *buf, struct ts_packet_adaptation_field *adaptation)
{
    int i;
    uint8_t offset = 0;

    uint8_t length = buf[offset];
    offset += 1;

    uint8_t discontinuity_indicator                 = (buf[offset] & 0x80) >> 7;
    uint8_t random_access_indicator                 = (buf[offset] & 0x40) >> 6;
    uint8_t elementary_stream_priority_indicator    = (buf[offset] & 0x20) >> 5;
    uint8_t pcr_flag                                = (buf[offset] & 0x10) >> 4;
    uint8_t opcr_flag                               = (buf[offset] & 0x08) >> 3;
    uint8_t splicing_point_flag                     = (buf[offset] & 0x04) >> 2;
    uint8_t transport_private_data_flag             = (buf[offset] & 0x02) >> 1;
    uint8_t adaptation_field_extension_flag         = (buf[offset] & 0x01);
    offset += 1;

    if (pcr_flag == 1) {
        uint8_t program_clock_reference_base[6];
        for (i = 0; i < 6; i++) {
            program_clock_reference_base[i] = buf[offset + i];
        }
        offset += 6;
    }

    if (opcr_flag == 1) {
        uint8_t original_program_clock_reference_base[6];
        for (i = 0; i < 6; i++) {
            original_program_clock_reference_base[i] = buf[offset + i];
        }
        offset += 6;
    }

    if (splicing_point_flag == 1){
        uint8_t splice_countdown = buf[offset];
        offset += 1;
    }

    if (transport_private_data_flag == 1) {
        uint8_t transport_private_data_length = buf[offset];
        offset +=1;

        for (i = 0; i < transport_private_data_length; i++) {
            uint8_t private_data_byte = buf[offset + i];
        }

        offset += transport_private_data_length;
    }

    if (adaptation_field_extension_flag == 1) {
        uint8_t adaptation_field_extension_length = buf[offset];
        offset += 1;

        uint8_t ltw_flag                = buf[offset] & 0x80 >> 7;
        uint8_t piecewise_rate_flag     = buf[offset] & 0x40 >> 6;
        uint8_t seamless_splice_flag    = buf[offset] & 0x20 >> 5;
        offset += 1;

        if (ltw_flag == 1) {
            uint8_t ltw_valid_flag      = buf[offset] & 0x80 >> 7;
            uint8_t ltw_offset          = buf[offset] & 0x7f << 8 | buf[offset+1];
            offset =+ 2;
        }

        if (piecewise_rate_flag == 1) {
        }

        if (seamless_splice_flag == 1) {
        }

        // RESERVE
    }

    //for (i = 0; i < N; i++) {
    //    stuffing_byte;
    //}

    return length;
}


int ts_packet_parser(uint8_t *buf, struct ts_packet *packet)
{
    uint8_t offset = 0;
    if (buf[offset] != sync_byte) {
        return 0;
    }
    offset += 1;

    packet->transport_error_indicator           = (buf[offset] & 0x80) >> 7;
    packet->payload_unit_start_indicator        = (buf[offset] & 0x40) >> 6;
    packet->transport_priority                  = (buf[offset] & 0x20) >> 5;
    packet->pid                                 = (buf[offset] & 0x1f) << 8 | buf[offset + 1];
    offset += 2;


    packet->transport_scrambling_control        = (buf[offset] & 0xc0) >> 6;
    packet->adaptation_field_control            = (buf[offset] & 0x30) >> 4;
    packet->continuity_counter                  = (buf[offset] & 0x0f);
    offset += 1;

    if (packet->adaptation_field_control & 0x2) {
        uint8_t adaptation_field_length = adaptation_field_parser(&buf[offset], &packet->adaptation_field);
        offset += adaptation_field_length;
    }

    if (packet->adaptation_field_control & 0x1) {
        packet->data_length = 188 - offset;
        packet->data_byte = &buf[offset];
    }
}


uint8_t* ts_packet_sync(uint8_t *buf)
{
    uint8_t offset = 0;
    do {
        if (buf[offset] == 0x47) {
            return &buf[offset];
        } else {
            offset += 1;
        }

        if (offset >= 188) {
            break;
        }
    } while(1);
    return NULL;
}

void dump_ts_packet(struct ts_packet *packet)
{
    printf("PacketInfo\n");
    printf("\tPID:0x%x(%d)\n", packet->pid, packet->pid);
    printf("\ttransport_error_indicator:%d\n", packet->transport_error_indicator);
    printf("\tpayload_unit_start_indicator:%d\n", packet->payload_unit_start_indicator);
    printf("\ttransport_priority:%d\n", packet->transport_priority);
    printf("\ttransport_scrambling_control:0x%x\n", packet->transport_scrambling_control);
    printf("\tadaptation_field_control:0x%x\n", packet->adaptation_field_control);
    printf("\tcontinuity_counter:%d\n", packet->continuity_counter);
    printf("\tdata_length:%d\n", packet->data_length);
}

int main()
{
    FILE *fp = NULL;
    uint8_t buf[188];
    uint32_t packet = 0;
    size_t r;

    fp = fopen(ts, "rb");
    if (fp == NULL) {
        perror("fopen");
        printf("ERROR\n");
        return 0;
    }

    while((r = fread(buf, 1, 188, fp) == 188)) {
        if (ts_packet_sync(buf) == NULL) {
            printf("SYNC ERROR\n");
            break;
        }
        struct ts_packet packet;
        ts_packet_parser(buf, &packet);
        if (packet.adaptation_field_control & 0x2) {
            dump_ts_packet(&packet);
        }
    }

    fclose(fp);
    return 0;
}
