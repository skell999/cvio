#include <stdio.h>
#include <fcntl.h>		// file control options
#include <unistd.h>		// posix system api
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdlib.h>		// malloc
#include <stdint.h>		// int types

 
#define KHZ_500 500000
#define MHZ_1 1000000
#define MHZ_2 2000000
#define MHZ_4 4000000
#define MHZ_8 8000000
#define MHZ_16 16000000
#define MHZ_32 32000000

#define MCP3208_START_BIT 0x16
#define MCP3208_SINGLE_ENDED 0x8
#define MCP3208_CHAN_0 0x0
#define MCP3208_CHAN_1 0x1
#define MCP3208_CHAN_2 0x2
#define MCP3208_CHAN_3 0x3
#define MCP3208_CHAN_4 0x4
#define MCP3208_CHAN_5 0x5
#define MCP3208_CHAN_6 0x6
#define MCP3208_CHAN_7 0x7

static void dumpstat(const char *name, int fd);
static int spi_transfer(int fd, struct spi_ioc_transfer *tr, int len);
static int spi_open(const char* device);
static int spi_close(int fd);
static int spi_mode(int fd, uint8_t mode);
static int spi_bits_per_word(int fd, uint8_t bits);
static int spi_speed(int fd, uint32_t speed);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

int main(){



	int fd = spi_open("/dev/spidev0.0");
	spi_mode(fd, SPI_MODE_0);
	// spi_mode(fd,  SPI_NO_CS | SPI_MODE_2);
	spi_speed(fd, MHZ_32);
	dumpstat("SPI device 0: ",fd);

	struct foo {
		int goo;
		int goo1;
		int goo2;
		int goo3;
		int goo4;
		int goo5;
		int goo6;
	}tb;

	tb.goo = 16;
	tb.goo1 = 26;
	tb.goo2 = 36;
	tb.goo3 = 46;
	tb.goo4 = 56;
	tb.goo5 = 66;
	tb.goo6 = 76;

	struct foo rb;

	printf("size of foo: %d\n", sizeof(tb));

	struct spi_ioc_transfer block = {
		.tx_buf = (unsigned long)&tb,
		.rx_buf = (unsigned long)&rb,
		.len = sizeof(tb),
		.delay_usecs = 0,
		.speed_hz = MHZ_32,
		.bits_per_word = 8,
	};

	spi_transfer(fd,&block,1);

	printf("rb.goo6: %d\n", rb.goo6);

	uint8_t tx[8] = {1,2,3,4,5,6,7,8};
	uint8_t tx2[8] = {9,10,11,12,13,14,15,16};

	uint8_t rx[8] = {0};
	uint8_t rx2[8] = {0};

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 8,
		.delay_usecs = 0,
		.speed_hz = MHZ_32,
		.bits_per_word = 8,
	};

	struct spi_ioc_transfer tr2 = {
		.tx_buf = (unsigned long)tx2,
		.rx_buf = (unsigned long)rx2,
		.len = 8,
		.delay_usecs = 0,
		.speed_hz = MHZ_32,
		.bits_per_word = 8,
	};

	// Transfer one message
	spi_transfer(fd, &tr, 1);

	// Transfer with array of messages
	struct spi_ioc_transfer list[2];
	list[0] = tr;
	list[1] = tr2;

	spi_transfer(fd, list, 2);

	for (int i = 0; i < 8; ++i)
	{
		printf("%d ",rx[i]);
	}
	printf("\n");

	for (int i = 0; i < 8; ++i)
	{
		printf("%d ",rx2[i]);
	}
	printf("\n");

	dumpstat("SPI device 0: ",fd);

	spi_close(fd);

}

static int spi_transfer(int fd, struct spi_ioc_transfer *tr, int len){
	int err = ioctl(fd, SPI_IOC_MESSAGE(len), tr);
	if (err < 0)	{
		perror("SPI transfer failed");
	}
	return err;
	// return 1;
}

static int spi_open(const char* device){
	int fd = open(device, O_RDWR);
	if (fd < 0){
		perror("Could not open SPI device");
	}
	return fd;
}

static int spi_close(int fd){
	int err = close(fd);
	if (err < 0){
		perror("Could not close SPI device");
	}
	return err;
}

static int spi_mode(int fd, uint8_t mode){
	int err = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (err <0){
		perror("Unable to change spi mode");
	}
	return err;
}

static int spi_bits_per_word(int fd, uint8_t bits){
	int err = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (err <0){
		perror("Unable to change spi bits per word");
	}
	return err;
}

static int spi_speed(int fd, uint32_t speed){
	int err = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (err <0){
		perror("Unable to change spi speed");
	}
	return err;
}

static void dumpstat(const char *name, int fd)
{
	__u8	lsb, bits;
	__u32	mode, speed;

	if (ioctl(fd, SPI_IOC_RD_MODE32, &mode) < 0) {
		perror("SPI rd_mode");
		return;
	}
	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb) < 0) {
		perror("SPI rd_lsb_fist");
		return;
	}
	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) {
		perror("SPI bits_per_word");
		return;
	}
	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
		perror("SPI max_speed_hz");
		return;
	}

	printf("%s: spi mode 0x%x, %d bits %sper word, %d Hz max\n",
		name, mode, bits, lsb ? "(lsb first) " : "", speed);
}