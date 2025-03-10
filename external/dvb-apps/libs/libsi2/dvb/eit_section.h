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

#ifndef _SI_DVB_EIT_SECTION_H
#define _SI_DVB_EIT_SECTION_H 1

#include <si/section.h>

struct dvb_eit_section {
	struct section_ext head;

	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint8_t segment_last_section_number;
	uint8_t last_table_id;
	/* struct eit_event events[] */
} packed;

struct dvb_eit_event {
	uint16_t event_id;
	uint8_t start_time[5];
  EBIT4(uint64_t duration		:24;  ,
	uint64_t running_status		: 3;  ,
	uint64_t free_ca_mode		: 1;  ,
	uint64_t descriptors_loop_length:12; );
	/* struct descriptor descriptors[] */
} packed;

struct dvb_eit_section *dvb_eit_section_parse(struct section_ext *);

#define dvb_eit_section_events_for_each(eit, pos) \
	for ((pos) = dvb_eit_section_events_first(eit); \
	     (pos); \
	     (pos) = dvb_eit_section_events_next(eit, pos))

#define dvb_eit_event_descriptors_for_each(event, pos) \
	for ((pos) = dvb_eit_event_descriptors_first(event); \
	     (pos); \
	     (pos) = dvb_eit_event_descriptors_next(event, pos))










/******************************** PRIVATE CODE ********************************/
static inline struct dvb_eit_event *
	dvb_eit_section_events_first(struct dvb_eit_section *eit)
{
	int pos = sizeof(struct dvb_eit_section);

	if (pos >= section_ext_length(&eit->head))
		return NULL;

        return (struct dvb_eit_event*) ((uint8_t *) eit + pos);
}

static inline struct dvb_eit_event *
	dvb_eit_section_events_next(struct dvb_eit_section *eit,
				    struct dvb_eit_event *pos)
{
	uint8_t *end = (uint8_t*) eit + section_ext_length(&eit->head);
	uint8_t *next =	(uint8_t *) pos +
			sizeof(struct dvb_eit_event) +
			pos->descriptors_loop_length;

	if (next >= end)
		return NULL;

	return (struct dvb_eit_event *) next;
}

static inline struct descriptor *
	dvb_eit_event_descriptors_first(struct dvb_eit_event * t)
{
	if (t->descriptors_loop_length == 0)
		return NULL;

	return (struct descriptor *)
		((uint8_t *) t + sizeof(struct dvb_eit_event));
}

static inline struct descriptor *
	dvb_eit_event_descriptors_next(struct dvb_eit_event * t,
				       struct descriptor* pos)
{
	return next_descriptor((uint8_t*) t + sizeof(struct dvb_eit_event),
			       t->descriptors_loop_length,
			       pos);
}

#endif
