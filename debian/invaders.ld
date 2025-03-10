/*
 * Minimal linker script for GRUB Invaders.
 *
 * Copyright © 2020, Fabian Greffrath <fabian@debian.org>
 * Copyright © 2025, downadow <downadow998@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

/* [03/09/25, 03/10/25] change SECTIONS;      --downadow */

PHDRS
{
	text PT_LOAD;
	data PT_LOAD;
}

SECTIONS
{
	.text 4M :
	{
		*(.text*)
		*(.rodata*)
	} : text

	.data :
	{
		*(.data*)
		_edata = .;
	} : data

	.bss (NOLOAD) :
	{
		*(.bss*)
		*(COMMON)
	}
	_end = .;

	PROVIDE(cmain = _cmain);
}
