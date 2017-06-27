/*
 * main.c
 * Copyright (C) Jens Askengren 2008 <jens.askengren@gmail.com>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libintl.h>

//#include <gnome.h>
#include <avahi-client/client.h>
#include <avahi-common/error.h>
#include <avahi-glib/glib-malloc.h>


#include "synergy-applet.h"

int
main (int argc, char *argv[])
{
	GMainLoop * main_loop;
	SynergyApplet* synergy_applet;
	
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	gtk_init(&argc, &argv);

//	gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
//                      argc, argv,
//                      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
//                      NULL);
	
	avahi_set_allocator(avahi_glib_allocator());

	synergy_applet = ((SynergyApplet *)g_object_new(synergy_applet_get_type(), NULL));	
	
	main_loop = g_main_loop_new (NULL, FALSE); 
	synergy_applet_start(synergy_applet, main_loop);	
	
	g_main_loop_run (main_loop);
	
	g_object_unref(G_OBJECT(synergy_applet));
	g_main_loop_unref(main_loop);
	
	return 0;
}
