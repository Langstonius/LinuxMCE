/*
 * section and descriptor parser
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 * Copyright (C) 2005 Patrick Boettcher (pb@linuxtv.org)
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

#include <si/dvb/int_section.h>

struct dvb_int_section * dvb_int_section_parse(struct section_ext *ext)
{
	uint8_t *buf = (uint8_t *) ext;
	struct dvb_int_section *in = (struct dvb_int_section *) ext;

	unsigned int pos = 0;
	unsigned int len = section_ext_length(ext);

	if (len < sizeof(struct dvb_int_section))
		return NULL;

	bswap32(buf+8);
	bswap16(buf+12);

	pos = sizeof(struct dvb_int_section);

	if (len - pos < in->platform_descriptor_loop_length)
		return NULL;

	if (verify_descriptors(buf + pos, in->platform_descriptor_loop_length))
		return NULL;

	pos += in->platform_descriptor_loop_length;

	while (pos < len) {
		struct dvb_int_section_target_loop *s2 = (struct dvb_int_section_target_loop *) (buf + pos);
		struct dvb_int_section_operational_loop *s3;

		bswap16(buf + pos); /* target_descriptor_loop_length swap */

		if (len - pos < s2->target_descriptor_loop_length)
			return NULL;

		pos += sizeof(struct dvb_int_section_target_loop);

		if (verify_descriptors(buf + pos, s2->target_descriptor_loop_length))
			return NULL;

		pos += s2->target_descriptor_loop_length;

		s3 = (struct dvb_int_section_operational_loop *) (buf + pos);

		bswap16(buf + pos); /* operational_descriptor_loop_length swap */

		if (len - pos < s3->operational_descriptor_loop_length)
			return NULL;

		pos += sizeof(struct dvb_int_section_operational_loop);

		if (verify_descriptors(buf + pos, s3->operational_descriptor_loop_length))
			return NULL;

		pos += s3->operational_descriptor_loop_length;
	}

	return (struct dvb_int_section *) ext;
}
