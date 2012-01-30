/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 *
 * libsamsung-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libsamsung-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libsamsung-ipc.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define isprint(c)	((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
void hexdump(const uint8_t *buf, int32_t len)
{
	char str[80], octet[10];
	int32_t ofs, i, l;

	for (ofs = 0; ofs < len; ofs += 16) {
		sprintf( str, "0x%02x: ", ofs );

		for (i = 0; i < 16; i++) {
			if ((i + ofs) < len)
				sprintf( octet, "%02x ", buf[ofs + i] );
			else
				strcpy( octet, "   " );

			strcat( str, octet );
		}
			strcat( str, "  " );
			l = strlen( str );

		for (i = 0; (i < 16) && ((i + ofs) < len); i++)
			str[l++] = isprint( buf[ofs + i] ) ? buf[ofs + i] : '.';

		str[l] = '\0';
		printf( "%s\n", str );
	}
}

