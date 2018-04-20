#ifdef __cplusplus
extern "C" {
#endif

void dstar_printhead(unsigned char *data, int len);
void dstar_printdatainfo(unsigned char *d, uint32_t crc, uint32_t datacrc);

int dstar_decode_head(unsigned char *headbits, unsigned char *head);
void dstar_decode_data(unsigned char *data, int datalen, unsigned char *ethdata);

void dstar_encode(const unsigned char *header, const unsigned char *data, int datalen, unsigned char *all);

void dstar_init();

#define HEADBITS (660+16)

#ifdef __cplusplus
}
#endif
