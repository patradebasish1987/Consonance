/*************************************************************************/
/* Copyright (C) 2008 blub <woolf.linux@bumiller.com>                    */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#ifndef MOD_H
#define MOD_H

#ifdef ENABLE_MODPLUG

#include <libmodplug/modplug.h>

#define MODBUF_LEN 4096

struct con_modplug_decoder {
	gchar buf[MODBUF_LEN];
	gchar *data;
	gsize length;
	GTimeVal timeval;
	ModPlugFile *mf;
};

void play_modplug(struct con_win *cwin);

#endif /* ENABLE_MODPLUG */

#endif /* MODPLUG_H */
