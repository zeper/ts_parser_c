#include <stdio.h>
#include <stdint.h>

const char *ts = "/home/mjkim/playground/hls/Mirae_435MHz.ts";

int adaptation_field_parser(uint8_t *buf)
{
    uint8_t length;
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
