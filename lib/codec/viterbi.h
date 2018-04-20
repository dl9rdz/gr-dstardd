#ifdef __cplusplus
extern "C" {
#endif

int init_viterbi();
int viterbi(char *logfile, unsigned char *ibuf, int ilen, unsigned char *obuf, int *olen);

int encode( unsigned char *symbols, unsigned char *data, unsigned int nbytes, unsigned int startstate, unsigned int endstate);

#ifdef __cplusplus
}
#endif

