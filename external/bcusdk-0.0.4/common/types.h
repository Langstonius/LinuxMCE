/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2008 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include "config.h"
#include "my_strings.h"
#include "array.h"

/** unsigned char */
typedef uint8_t uchar;

/** Array of characters */
typedef Array < uchar > CArray;

/** EIB address */
typedef uint16_t eibaddr_t;

/** EIB key */
typedef uint32_t eibkey_type;

#ifdef USE_NOLIBSTDC
#include "libstdc.h"
#endif

#endif
