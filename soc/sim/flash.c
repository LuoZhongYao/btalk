/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

static int flash;

void flash_erase(unsigned addr)
{
}

void flash_write(unsigned addr, uint32_t *data, unsigned size)
{
	lseek(flash, addr, SEEK_SET);
	write(flash, data, size);
}

void flash_read(unsigned addr, uint32_t *data, unsigned size)
{
	lseek(flash, addr, SEEK_SET);
	read(flash,data, size);
}

void flash_init(void)
{
	flash = open("/tmp/btalk.flash", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
}
