/*
 * Minimal linker script for GRUB Invaders.
 *
 * Copyright © 2020, Fabian Greffrath <fabian@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

PHDRS
{
	text PT_LOAD;
	data PT_LOAD;
}

SECTIONS
{
	.text 1M :
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
