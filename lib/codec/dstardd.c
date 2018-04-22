#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "crc32.h"
#include "viterbi.h"
#include "dstardd.h"

static uint16_t headercrc(const unsigned char *head)
{
	uint16_t genpoly = 0x8408;
	uint16_t crc = 0xffff;
	for(int i=0; i<39; i++) {
		crc ^= head[i];
		for(int j=0; j<8; j++) {
			if(crc&1) { crc>>=1; crc^=genpoly; }
			else { crc>>=1; }
		}
	}
	crc ^= 0xffff;
	return crc;
}

static void descramble(int *sr, unsigned char *bits, int len)
{
	int i;
	for (i=0; i<len; i++) {
		if( ((*sr>>3)&0x1) ^ (*sr&0x1) ) {
			*sr >>= 1;
			*sr |= 64;
			bits[i] ^= 0x1;
		} else {
			*sr >>= 1;
		}
	}
}

static void descramble_bytes_msb(int *sr, unsigned char *bytes, int len)
{
	int i;
	for (i=0; i<len; i++) {
		for(int j=0; j<8; j++) {
			if( ((*sr>>3)&0x1) ^ (*sr&0x1) ) {
				*sr >>= 1;
				*sr |= 64;
				bytes[i] ^= (0x80>>j);
			} else {
				*sr >>= 1;
			}
		}
	}
}
static void descramble_bytes(int *sr, unsigned char *bytes, int len)
{
	int i;
	for (i=0; i<len; i++) {
		for(int j=0; j<8; j++) {
			if( ((*sr>>3)&0x1) ^ (*sr&0x1) ) {
				*sr >>= 1;
				*sr |= 64;
				bytes[i] ^= (0x1<<j);
			} else {
				*sr >>= 1;
			}
		}
	}
}

#if 0
unsigned char descramblebit(unsigned char bit)
{
	if( ((sr>>3)&0x1) ^ (sr&0x1) ) {
		sr >>= 1;
		sr |= 64;
		return bit ^ 0x1;
	} else {
		sr >>= 1;
		return bit;
	}
}
#endif

static void deinterleave(unsigned char *bits, unsigned char *outbits) {
	int i,j;
	for(i=0; i<12; i++) for(j=0; j<28; j++) outbits[i+j*24]=bits[i*28+j];
	for(i=12; i<24; i++) for(j=0; j<27; j++) outbits[i+j*24]=bits[i*27+j+12];
}

static void interleave(unsigned char *bits, unsigned char *outbits) {
	int i,j;
	for(i=0; i<12; i++) for(j=0; j<28; j++) outbits[i*28+j]=bits[i+j*24];
	for(i=12; i<24; i++) for(j=0; j<27; j++) outbits[i*27+j+12]=bits[i+j*24];
}


#define RED "\033[31;1m"
#define GREEN "\033[32m"
#define NORMAL "\033[0m"

void dstar_printhead(unsigned char *data, int len, int istx) {
	unsigned char out[120];
	uint16_t datacrc = headercrc(data);
	uint16_t crc = (data[40]<<8) + data[39];
	unsigned char crcstr[20];
	if(istx) { sprintf(crcstr,""); }
	else if(datacrc==crc) { sprintf(crcstr,GREEN " OK " NORMAL); }
	else { sprintf(crcstr, RED "%04X" NORMAL, datacrc); }
	snprintf(out, 120, "%cX(%02X/%02X%02X) %.8s(%.4s)>%.8s via %.8s,%.8s CRC %04X[%s] DLen=%d",
		istx?'T':'R',data[0],data[1],data[2],data+27,data+35,data+19,data+3,data+11,crc,crcstr,len);
	fprintf(stderr,"\n%s\n",out);
}

void dstar_printdatainfo(unsigned char *d, uint32_t crc, uint32_t datacrc) {
	if(crc==datacrc) {
		fprintf(stderr,"Data CRC[%08x]: " GREEN " >>OK<<  " NORMAL, crc);
	} else {
		fprintf(stderr,"Data CRC[%08x]: " RED "%08x " NORMAL, crc, datacrc);
	}
	fprintf(stderr,"%02x:%02x:%02x:%02x:%02x:%02x>",d[6],d[7],d[8],d[9],d[10],d[11]);
	fprintf(stderr,"%02x:%02x:%02x:%02x:%02x:%02x ",d[0],d[1],d[2],d[3],d[4],d[5]);
	if(d[12]==8 && d[13]==0) { // IPv4
		fprintf(stderr, "IPv4 %d.%d.%d.%d>%d.%d.%d.%d\n",d[26],d[27],d[28],d[29],
			d[30],d[31],d[32],d[33]);
	} else if(d[12]==8 && d[13]==6 && d[16]==8 && d[17]==0) { // ARP IPv4
		fprintf(stderr, "ARP%s %d.%d.%d.%d>%d.%d.%d.%d\n", d[21]==1?"q":"r",
			d[28],d[29],d[30],d[31],
			d[38],d[39],d[40],d[41]);
	} else {
		fprintf(stderr, "Type %02x%02x\n",d[12],d[13]);
	}
}

static int sr;
int dstar_decode_head(unsigned char *headbits, unsigned char *head) {
	int outlen;
	unsigned char outbits[330+100];
	sr = 0x7f;
	descramble(&sr, headbits, HEADBITS);
	unsigned char tmpbits[HEADBITS];
	deinterleave(headbits, tmpbits);
	outlen=330+100;
	int res = viterbi(NULL, tmpbits, 660, outbits, &outlen);
	for(int i=0; i<41; i++) {
		head[i]=0;
		for(int j=0; j<8; j++) {
			head[i] += outbits[8*i+j]<<j;
		}
	}
	uint16_t datacrc = headercrc(head);
	uint16_t crc = (head[40]<<8) + head[39];
	if(datacrc != crc) { return -1; }

	// decode data len
	int len = 0;
	for(int i=660; i<660+16; i++) {
		len += (headbits[i]&0x01)<<(i-660);
	}
	if(len>1800) len=1800;
	return len;
}

void dstar_decode_data(unsigned char *data, int datalen, unsigned char *ethframe) {
	descramble_bytes(&sr, data, datalen);
	unsigned char allbytes[datalen+2-4];
	allbytes[0]= ((datalen-4)&0xff);
	allbytes[1] = ((datalen-4)>>8)&0xff;
	memcpy(allbytes+2, data, datalen-4);
	uint32_t crc = crc32(0, allbytes, 2+datalen-4);
	uint32_t datacrc = (data[datalen-1]<<24) | (data[datalen-2]<<16 )
		| (data[datalen-3]<<8) | (data[datalen-4]);
	dstar_printdatainfo(data, crc, datacrc);

	// print raw frame
#if 0
	fprintf(stderr,"raw rx data: ");
	for(int i=0; i<datalen; i++) { fprintf(stderr, "%02X ",data[i]); }
	fprintf(stderr,"\n");
#endif
	// prepare eth frame for writing to TAP device (with correct crc)
	//memcpy(ethframe, data+6, 6);
	//memcpy(ethframe+6, data, 6);
	memcpy(ethframe, data, 6);
	memcpy(ethframe+6, data+6, 6);
	memcpy(ethframe+12, data+12, datalen-12-4);
	crc = crc32(0, ethframe, datalen-4);
	for(int i=0; i<4; i++) { ethframe[datalen-4+i] = (crc>>(8*i))&0xff; }
}

// datalen: without CRC; data is writeable and has space for writing crc
// all: must have space for 41 + 2 + datalen + 4 bytes (head, len, data, datacrc)
// all is MSB first (now!)
int dstar_encode(const unsigned char *header, const unsigned char *data, int datalen, unsigned char *all) {
	// Add CRC to header
	unsigned char tmp[41];
	memcpy(tmp, header, 39);
	uint16_t hcrc = headercrc(header);
	tmp[39] = hcrc&0xff;
	tmp[40] = (hcrc>>8)&0xff;
	for(int i=0; i<41; i++) { fprintf(stderr, tmp[i]<31||tmp[i]>127?"\\%02x":"%c", tmp[i]); }

	// Convolutional encoding of header
	unsigned char symbols[HEADBITS];
	memset(symbols, -1, HEADBITS);
	encode(symbols, tmp, 41, 0, 0);
#if 0
	fprintf(stderr, "\nSYM:                    ");
	for(int i=0; i<HEADBITS-16; i++) {
		fprintf(stderr,"%0d, ", symbols[i]);
	}
#endif

	// experimental padding --- only for testing
	int paddingbytes = 0;
	if(datalen<60) paddingbytes = 60-datalen;
	datalen += paddingbytes;

	// Interleaving of header block
	unsigned char interleaved[HEADBITS+(2+datalen+4)*8];
	interleave(symbols, interleaved);
	fprintf(stderr, "\n");

	// Prepare payload (len + data + crc)
	unsigned char payload[2+datalen+4];
	payload[0] = datalen&0xff;
	payload[1] = (datalen>>8)&0xff;

	memcpy(payload+2, data, datalen-paddingbytes);   
	memset(payload+2 + datalen-paddingbytes, 0, paddingbytes);

	uint32_t crc = crc32(0, payload, datalen+2);
	for(int i=0; i<4; i++) { payload[datalen+2+i] = (crc>>(8*i))&0xff; }
	datalen += 6; // including len and crc32

	// Copy payload to end of interleaved
	for(int i=0; i<8*datalen; i++) {
		interleaved[HEADBITS-16+i] = (payload[i>>3]>>(i%8))&0x01;
	}

	// now, first bit goes into MSB
	//fprintf(stderr, "raw bits: ");
	for(int i=0; i<HEADBITS-16+8*datalen; i++) {
		//fprintf(stderr, "%02d ",interleaved[i]);
		if( (i&7)==0 ) { all[i>>3] = 0; }
		all[i>>3] |= interleaved[i]<<(7-(i&7));
	}
	// fprintf(stderr, "\n");
	
	// Scramble
	int sr = 0x7f;
	descramble_bytes_msb(&sr, all, 85 + datalen);
	return 85 + datalen;
}

void dstar_init() {
	init_viterbi();
}
