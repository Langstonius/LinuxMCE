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

#ifndef _SI_DVB_MOSAIC_DESCRIPTOR
#define _SI_DVB_MOSAIC_DESCRIPTOR 1

#include <si/descriptor.h>
#include <si/common.h>

struct dvb_mosaic_descriptor {
	struct descriptor d;

  EBIT4(uint8_t mosaic_entry_point		: 1; ,
	uint8_t number_of_horiz_elementary_cells: 3; ,
	uint8_t reserved			: 1; ,
	uint8_t number_of_vert_elementary_cells	: 3; );
	/* struct dvb_mosaic_info infos[] */
} packed;

struct dvb_mosaic_info {
  EBIT3(uint16_t logical_cell_id		: 6; ,
	uint16_t reserved			: 7; ,
	uint16_t logical_cell_presentation_info	: 3; );
	uint8_t elementary_cell_field_length;
	/* struct dvb_mosaic_elementary_cell_field fields[] */
	/* struct dvb_mosaic_info_part2 part2 */
	/* struct dvb_mosaic_linkage linkage */
} packed;

struct dvb_mosaic_elementary_cell_field {
  EBIT2(uint8_t reserved		: 2; ,
	uint8_t elementary_cell_id	: 6; );
} packed;

struct dvb_mosaic_info_part2 {
	uint8_t cell_linkage_info;
} packed;

struct dvb_mosaic_linkage {
	union {
		struct dvb_mosaic_linkage_01 {
			uint16_t bouquet_id;
		} packed linkage_01;

		struct dvb_mosaic_linkage_02 {
			uint16_t original_network_id;
			uint16_t transport_stream_id;
			uint16_t service_id;
		} packed linkage_02;

		struct dvb_mosaic_linkage_03 {
			uint16_t original_network_id;
			uint16_t transport_stream_id;
			uint16_t service_id;
		} packed linkage_03;

		struct dvb_mosaic_linkage_04 {
			uint16_t original_network_id;
			uint16_t transport_stream_id;
			uint16_t service_id;
			uint16_t event_id;
		} packed linkage_04;
	} u;
} packed;

static inline struct dvb_mosaic_descriptor*
	dvb_mosaic_descriptor_parse(struct descriptor* d)
{
	uint8_t* buf = (uint8_t*) d + 2;
	int pos = 0;
	int len = d->len;
	struct dvb_mosaic_descriptor * p =
		(struct dvb_mosaic_descriptor *) d;

	pos += (sizeof(struct dvb_mosaic_descriptor) - 2);

	if (pos > len)
		return NULL;

	while(pos < len) {
		struct dvb_mosaic_info *e =
			(struct dvb_mosaic_info*) (buf+pos);
		struct dvb_mosaic_info_part2 *e2;
		struct dvb_mosaic_linkage *linkage;

		if ((pos + sizeof(struct dvb_mosaic_info)) > len)
			return NULL;

		bswap16(buf + pos);

		pos += sizeof(struct dvb_mosaic_info) +
			e->elementary_cell_field_length;

		if (pos > len)
			return NULL;

		e2 = (struct dvb_mosaic_info_part2*) (buf+pos);

		pos += sizeof(struct dvb_mosaic_info_part2);

		if (pos > len)
			return NULL;

		linkage = (struct dvb_mosaic_linkage*) (buf+pos);

		switch(e2->cell_linkage_info) {
		case 0x01:
			if ((pos + sizeof(struct dvb_mosaic_linkage_01)) > len)
				return NULL;
			bswap16(buf+pos);
			pos += sizeof(struct dvb_mosaic_linkage_01);
			break;

		case 0x02:
			if ((pos + sizeof(struct dvb_mosaic_linkage_02)) > len)
				return NULL;
			bswap16(buf+pos);
			bswap16(buf+pos+2);
			bswap16(buf+pos+4);
			pos += sizeof(struct dvb_mosaic_linkage_02);
			break;

		case 0x03:
			if ((pos + sizeof(struct dvb_mosaic_linkage_03)) > len)
				return NULL;
			bswap16(buf+pos);
			bswap16(buf+pos+2);
			bswap16(buf+pos+4);
			pos += sizeof(struct dvb_mosaic_linkage_03);
			break;

		case 0x04:
			if ((pos + sizeof(struct dvb_mosaic_linkage_04)) > len)
				return NULL;
			bswap16(buf+pos);
			bswap16(buf+pos+2);
			bswap16(buf+pos+4);
			bswap16(buf+pos+6);
			pos += sizeof(struct dvb_mosaic_linkage_04);
			break;
		}
	}

	return p;
}

#define dvb_mosaic_descriptor_infos_for_each(d, pos) \
	for ((pos) = dvb_mosaic_descriptor_infos_first(d); \
	     (pos); \
	     (pos) = dvb_mosaic_descriptor_infos_next(d, pos))

#define dvb_mosaic_info_fields_for_each(info, pos) \
	for ((pos) = dvb_mosaic_info_fields_first(info); \
	     (pos); \
	     (pos) = dvb_mosaic_info_fields_next(info, pos))

static inline struct dvb_mosaic_info_part2*
	dvb_mosaic_info_part2(struct dvb_mosaic_info* entry)
{
	return (struct dvb_mosaic_info_part2*)
		((uint8_t*) entry + sizeof(struct dvb_mosaic_info) +
		 entry->elementary_cell_field_length);
}

static inline struct dvb_mosaic_linkage*
	dvb_mosaic_linkage(struct dvb_mosaic_info_part2* entry)
{
	if ((entry->cell_linkage_info != 0x01) &&
	    (entry->cell_linkage_info != 0x02) &&
	    (entry->cell_linkage_info != 0x03) &&
	    (entry->cell_linkage_info != 0x04))
		return NULL;

	return (struct dvb_mosaic_linkage*)
		((uint8_t*) entry + sizeof(struct dvb_mosaic_info_part2));
}










/******************************** PRIVATE CODE ********************************/
static inline struct dvb_mosaic_info*
	dvb_mosaic_descriptor_infos_first(struct dvb_mosaic_descriptor *d)
{
	if (d->d.len == 1)
		return NULL;

	return (struct dvb_mosaic_info *)
		(uint8_t*) d + sizeof(struct dvb_mosaic_descriptor);
}

static inline struct dvb_mosaic_info*
	dvb_mosaic_descriptor_infos_next(struct dvb_mosaic_descriptor *d,
					 struct dvb_mosaic_info *pos)
{
	struct dvb_mosaic_info_part2* part2 = dvb_mosaic_info_part2(pos);
	uint8_t *end = (uint8_t*) d + 2 + d->d.len;
	uint8_t *next =	(uint8_t *) pos + sizeof(struct dvb_mosaic_info) +
			pos->elementary_cell_field_length +
			sizeof(struct dvb_mosaic_info_part2);

	if (part2->cell_linkage_info == 0x01)
		next += sizeof(struct dvb_mosaic_linkage_01);
	else if (part2->cell_linkage_info == 0x02)
		next += sizeof(struct dvb_mosaic_linkage_02);
	else if (part2->cell_linkage_info == 0x03)
		next += sizeof(struct dvb_mosaic_linkage_03);
	else if (part2->cell_linkage_info == 0x04)
		next += sizeof(struct dvb_mosaic_linkage_04);

	if (next >= end)
		return NULL;

	return (struct dvb_mosaic_info *) next;
}

static inline struct dvb_mosaic_elementary_cell_field*
	dvb_mosaic_info_fields_first(struct dvb_mosaic_info *d)
{
	if (d->elementary_cell_field_length == 0)
		return NULL;

	return (struct dvb_mosaic_elementary_cell_field*)
		(uint8_t*) d + sizeof(struct dvb_mosaic_info);
}

static inline struct dvb_mosaic_elementary_cell_field*
	dvb_mosaic_info_fields_next(struct dvb_mosaic_info *d,
				    struct dvb_mosaic_elementary_cell_field* pos)
{
	uint8_t *end = (uint8_t*) d + sizeof(struct dvb_mosaic_info) +
			d->elementary_cell_field_length;
	uint8_t *next =	(uint8_t *) pos +
			sizeof(struct dvb_mosaic_elementary_cell_field);

	if (next >= end)
		return NULL;

	return (struct dvb_mosaic_elementary_cell_field *) next;
}

#endif
