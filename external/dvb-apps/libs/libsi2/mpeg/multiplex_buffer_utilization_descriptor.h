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

#ifndef _SI_MPEG_MULTIPLEX_BUFFER_UTILIZATION_DESCRIPTOR
#define _SI_MPEG_MULTIPLEX_BUFFER_UTILIZATION_DESCRIPTOR 1

#include <si/descriptor.h>
#include <si/common.h>

struct mpeg_multiplex_buffer_utilization_descriptor {
	struct descriptor d;

  EBIT2(uint16_t bound_valid_flag		: 1;  ,
	uint16_t ltw_offset_lower_bound		: 15; );
  EBIT2(uint16_t reserved			: 1;  ,
	uint16_t ltw_offset_upper_bound		: 15; );
} packed;

static inline struct mpeg_multiplex_buffer_utilization_descriptor*
	mpeg_multiplex_buffer_utilization_descriptor_parse(struct descriptor* d)
{
	if (d->len != (sizeof(struct mpeg_multiplex_buffer_utilization_descriptor) - 2))
		return NULL;

	bswap16((uint8_t*) d + 2);
	bswap16((uint8_t*) d + 4);

	return (struct mpeg_multiplex_buffer_utilization_descriptor*) d;
}

#endif
