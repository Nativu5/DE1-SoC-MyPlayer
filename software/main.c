#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <inttypes.h>
#include <stdbool.h>

#include "hwlib.h"
#include "socal/hps.h"
#include "socal/socal.h"
#include "hps_0.h"
#include "audio_control.h"
#include "pcm.h"

#define HW_REGS_BASE (ALT_STM_OFST)
#define HW_REGS_SPAN (0x04000000)
#define HW_REGS_MASK (HW_REGS_SPAN - 1)

//base addr
static volatile unsigned long *h2p_lw_axi_addr = NULL;

volatile unsigned long *oc_i2c_audio_addr = NULL;
volatile unsigned long *audio_addr = NULL;

int main(int argc, char **argv)
{
	void *virtual_base;
	int fd;

	if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1)
	{
		fprintf(stderr, "[ERROR] Fail to open \"/dev/mem\"...\n");
		return 1;
	}

	virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
	if (virtual_base == MAP_FAILED)
	{
		fprintf(stderr, "[ERROR] mmap() failed...\n");
		close(fd);
		return 1;
	}

	h2p_lw_axi_addr = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST) & (unsigned long)(HW_REGS_MASK));
	oc_i2c_audio_addr = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + OC_I2C_MASTER_0_BASE) & (unsigned long)(HW_REGS_MASK));
	audio_addr = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + AUDIO_IF_0_BASE) & (unsigned long)(HW_REGS_MASK));

	printf("[INFO] i2c_audio_addr:	%04Xh\n", (unsigned int)oc_i2c_audio_addr);
	printf("[INFO] audio_addr:		%04Xh\n", (unsigned int)audio_addr);

	oc_i2c_audio_init();
	init_audio();

	usleep(500 * 1000); ///// !!!!!!! note. this delay is necessary
	
	char AudioName[128] = {"TheDawn.mp3"};
	if (argc == 2 && argv[1])
		strncpy(AudioName, argv[1], 128);
	
	//TODO: Check if .pcm exists.
	
	char PCMName[128 + 4];
	strncpy(PCMName, AudioName, 128);
	strncat(PCMName, ".pcm", 4);

	AUDIO_SetSampleRate(RATE_ADC32K_DAC32K);

	FILE *is_exist;
	if (is_exist = fopen(PCMName, "r"), is_exist == NULL)
	{
		printf("[INFO] Converting PCM...\n");
		char cmd[512];
		sprintf(cmd, "ffmpeg -i \"%s\" -f s16le -ar 32000 -ac 2 -acodec pcm_s16le \"%s\" -y", AudioName, PCMName);
		if (system(cmd) != 0)
		{
			fprintf(stderr, "[ERROR] Convert PCM failed...");
			munmap(virtual_base, HW_REGS_SPAN);
			close(fd);
			return 1;
		}
	}

	printf("[INFO] Now playing %s...\n", AudioName);
	play_PCM(PCMName);
	printf("[INFO] Music over, quitting...\n");

	if (munmap(virtual_base, HW_REGS_SPAN) != 0)
	{
		fprintf(stderr, "[ERROR] munmap() failed...\n");
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}
