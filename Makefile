MPICC:= mpicc

all: printMatrix generateMatrix seqMatrixMultiply dns dnsVariant

printMatrix: printMatrix.c inOutUtils.c
	$(CC) printMatrix.c inOutUtils.c -o printMatrix

generateMatrix: generateMatrix.c  inOutUtils.c
	$(CC) generateMatrix.c inOutUtils.c -o generateMatrix 

seqMatrixMultiply: seqMatrixMultiply.c inOutUtils.c
	$(CC) seqMatrixMultiply.c inOutUtils.c -o seqMatrixMultiply

dns: dns.c inOutUtils.c
	$(MPICC) dns.c inOutUtils.c -o dns

dnsVariant: dnsVariant.c inOutUtils.c
	$(MPICC) dnsVariant.c inOutUtils.c -o dnsVariant

clean:
	rm -f printMatrix generateMatrix seqMatrixMultiply dns dnsVariant