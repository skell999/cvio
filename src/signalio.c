#include <stdio.h>
#include <fcntl.h>		// file control options
#include <unistd.h>		// posix system api
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdlib.h>		// malloc
#include <stdint.h>		// int types
#include <sys/time.h>		// nanosleep
#include <time.h>		// nanosleep

#define KHZ_500 500000
#define MHZ_1 1000000
#define MHZ_2 2000000
#define MHZ_4 4000000
#define MHZ_8 8000000
#define MHZ_16 16000000
#define MHZ_32 32000000

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 

#define MCP_START_BIT 0x10
#define MCP_SINGLE_ENDED 0x8
#define MCP_CHAN_0 MCP_START_BIT | MCP_SINGLE_ENDED | 0x0
#define MCP_CHAN_1 MCP_START_BIT | MCP_SINGLE_ENDED | 0x1
#define MCP_CHAN_2 MCP_START_BIT | MCP_SINGLE_ENDED | 0x2
#define MCP_CHAN_3 MCP_START_BIT | MCP_SINGLE_ENDED | 0x3
#define MCP_CHAN_4 MCP_START_BIT | MCP_SINGLE_ENDED | 0x4
#define MCP_CHAN_5 MCP_START_BIT | MCP_SINGLE_ENDED | 0x5
#define MCP_CHAN_6 MCP_START_BIT | MCP_SINGLE_ENDED | 0x6
#define MCP_CHAN_7 MCP_START_BIT | MCP_SINGLE_ENDED | 0x7

static void print_byte(uint8_t byte);
static uint16_t get_mcp_channel_command(uint8_t chan);
static int spi_transfer(int fd, struct spi_ioc_transfer *tr, int len);
static int spi_open(const char* device);
static int spi_close(int fd);
static int spi_mode(int fd, uint8_t mode);
static int spi_bits_per_word(int fd, uint8_t bits);
static int spi_speed(int fd, uint32_t speed);
static void dumpstat(const char *name, int fd);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// static struct mcp_buffer {
// 	uint8_t byte01;
// 	uint8_t byte02;
// };

static const struct timespec requested = {
	.tv_sec = 0,
	.tv_nsec = 40000,
	// .tv_nsec = 0,
};

static struct timespec remaining;

int main(){



	int fd = spi_open("/dev/spidev0.0");
	spi_mode(fd, SPI_MODE_1);
	// spi_mode(fd,  SPI_NO_CS | SPI_MODE_2);
	spi_speed(fd, MHZ_32);
	dumpstat("SPI device 0: ",fd);

	// uint16_t command = MCP3208_START_BIT | MCP3208_SINGLE_ENDED

	printf("size %d\n", sizeof(MCP_CHAN_7));

	// uint32_t foo = MCP_CHAN_7;
	// uint32_t mcp_tx = ((uint32_t)MCP_CHAN_7) << 14;
	// uint32_t mcp_rx = 0;

	uint8_t tx[] = {1,8 + 0 << 4,0,0};
	uint8_t rx[4] = {0}; 


	struct spi_ioc_transfer mcp_tr ={
		.tx_buf = (unsigned long)&tx,
		.rx_buf = (unsigned long)&rx,
		.len = 4,
		.delay_usecs = 0,
		.speed_hz = KHZ_500,
		.bits_per_word = 8,
	};


	while(1){
		spi_transfer(fd, &mcp_tr, 1);

		// printf ("B "BYTETOBINARYPATTERN" ", BYTETOBINARY(mcp_tx >> 24));
		// printf (""BYTETOBINARYPATTERN" ", BYTETOBINARY(mcp_tx >> 16));
		// printf (""BYTETOBINARYPATTERN" ", BYTETOBINARY(mcp_tx >> 8));
		// printf (""BYTETOBINARYPATTERN" ", BYTETOBINARY(mcp_tx));
		// printf("\tbuf_tx: %u\n", mcp_tx);

		// mcp_rx |= 0xff000000;
		// mcp_rx &= 0x00ffffff;
		// mcp_rx = mcp_rx >> 0;

		uint32_t ret = 0;
		ret |= (uint32_t)rx[3];
		ret |= (uint32_t)rx[2] << 8;
		ret |= (uint32_t)rx[1] << 16;
		// ret |= (uint32_t)rx[0] << 24;

		print_byte(rx[0]);
		print_byte(rx[1]);
		print_byte(rx[2]);
		print_byte(rx[3]);
		printf(" rx %d", ret >> 6);
		printf("\n");

		// printf("%d\n", rx);

		// printf ("\nA "BYTETOBINARYPATTERN" ", BYTETOBINARY(mcp_rx >> 24));
		// printf (""BYTETOBINARYPATTERN" ", BYTETOBINARY(mcp_rx >> 16));
		// printf (""BYTETOBINARYPATTERN" ", BYTETOBINARY(mcp_rx >> 8));
		// printf (""BYTETOBINARYPATTERN" ", BYTETOBINARY(mcp_rx));
		// printf("\tbuf_rx: %u\n", mcp_rx);

		nanosleep(&requested,&remaining);
	}

	// uint16_t command = get_mcp_channel_command(0);
	// printf ("Byte 00 "BYTETOBINARYPATTERN"\n", BYTETOBINARY((uint8_t)command));
	// printf ("Byte 01 "BYTETOBINARYPATTERN"\n", BYTETOBINARY((uint8_t)command >> 8));

	// printf ("Byte 00 "BYTETOBINARYPATTERN"\n", BYTETOBINARY((uint8_t)0x8));
	// printf ("Byte 01 "BYTETOBINARYPATTERN"\n", BYTETOBINARY((uint8_t)0x10));
	// printf("command %d %s\n", command, &buf);

	// dumpstat("SPI device 0: ",fd);

	spi_close(fd);

}

static void print_byte(uint8_t byte){
	printf (""BYTETOBINARYPATTERN" ", BYTETOBINARY(byte));
} 

static uint16_t get_mcp_channel_command(uint8_t chan){
	// uint16_t command = 0;
	// return command = (MCP3208_START_BIT | MCP3208_SINGLE_ENDED | chan); 
	return 0;
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