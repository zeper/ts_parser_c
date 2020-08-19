#include <stdio.h>
#include <stdint.h>

const char *ts = "/home/mjkim/playground/hls/Mirae_435MHz.ts";

int adaptation_field_parser(uint8_t *buf)
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
    }
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
        //printf("%x %x %x\n", buf[0], buf[1], buf[2]);
        //if (((buf[1] & 0x1f) << 8 | buf[2]) == 0) {
            printf("PACKET:%4u\n\tsync_byte:0x%x\n\ttransport_error_indicator:%d\n\tpayload_unit_start_indicator:%d\n\ttransport_priority:%d\n\tPID:0x%x",
                    packet,
                    buf[0],
                    (buf[1] & 0x80) >> 7,
                    (buf[1] & 0x40) >> 6,
                    (buf[1] & 0x20) >> 5,
                    (buf[1] & 0x1f) << 8 | buf[2]
                  );
            printf("\n\ttransport_scrambling_control:0x%x\n\tadaptation_field_control:0x%x\n\tcontinuity_counter:0x%x\n",
                    (buf[3] & 0xc0) >> 6,
                    (buf[3] & 0x30) >> 4,
                    (buf[3] & 0x0f)
                    );

        //}
        packet++;
        if (packet > 10) {
            break;
        }
    }

    fclose(fp);
    return 0;
}
