/*
 * section and descriptor parser
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef _SI_MPEG_STD_DESCRIPTOR
#define _SI_MPEG_STD_DESCRIPTOR 1

#include <si/descriptor.h>
#include <si/common.h>

struct mpeg_std_descriptor {
	struct descriptor d;

  EBIT2(uint8_t reserved	: 7; ,
	uint8_t leak_valid_flag	: 1; );
} packed;

static inline struct mpeg_std_descriptor*
	mpeg_std_descriptor_parse(struct descriptor* d)
{
	if (d->len != (sizeof(struct mpeg_std_descriptor) - 2))
		return NULL;

	return (struct mpeg_std_descriptor*) d;
}

#endif
