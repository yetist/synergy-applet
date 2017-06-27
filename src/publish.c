
#include "publish.h"
#include "synergy-applet.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include <avahi-common/alternative.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>

static void entry_group_callback(AvahiEntryGroup* g, AvahiEntryGroupState state, SynergyApplet* applet) 
{
    applet->avahi_entry_group = g; // may be null

    /* Called whenever the entry group state changes */

    switch (state) 
	{
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            g_debug("(Avahi) Service '%s' successfully established.", 
					applet->avahi_service_name);
            break;

        case AVAHI_ENTRY_GROUP_COLLISION : 
		{			
		
			// TODO: This may indicate a forgery. Notify user?
            gchar* n = avahi_alternative_service_name(applet->avahi_service_name);
            avahi_free(applet->avahi_service_name);
            applet->avahi_service_name = n;

            g_debug("Service name collision, renaming service to '%s'", n);

			synergy_applet_avahi_publish(applet);

			break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            g_warning("(Avahi) Entry group failure: %s", 
					avahi_strerror(avahi_client_errno(applet->avahi_client)));

            /* Some kind of failure happened while we were registering our services */
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

void synergy_applet_avahi_publish(SynergyApplet* applet)
{
		
	if (applet->avahi_service_name == NULL)
	{
		applet->avahi_service_name = (applet->mode == SYNERGY_MODE_CLIENT 
									  ? "Synergy client" : "Synergy server");
	}
	
    /* If this is the first time we're called, let's create a new
     * entry group if necessary */

    if (applet->avahi_entry_group == NULL)
	{
		applet->avahi_entry_group 
			= avahi_entry_group_new(applet->avahi_client, entry_group_callback, applet);
			
        if (applet->avahi_entry_group == NULL) 
		{
            g_warning("(Avahi) avahi_entry_group_new() failed: %s", 
					  avahi_strerror(avahi_client_errno(applet->avahi_client)));
            return;
        }
	}

    /* If the group is empty (either because it was just created, or
     * because it was reset previously, add our entries.  */

    if (avahi_entry_group_is_empty(applet->avahi_entry_group)) 
	{
		int ret;

        g_debug("(Avahi) Adding service '%s'", applet->avahi_service_name);

        /* We will now add two services and one subtype to the entry
         * group. The two services have the same name, but differ in
         * the service type (IPP vs. BSD LPR). Only services with the
         * same name should be put in the same entry group. */

		ret = avahi_entry_group_add_service(
			applet->avahi_entry_group, 
			AVAHI_IF_UNSPEC,
			AVAHI_PROTO_UNSPEC, 
			0, 
			applet->avahi_service_name, 
			NULL, 
			NULL, 
			24800,
			NULL);
		
		if (ret < 0) 
		{

            if (ret == AVAHI_ERR_COLLISION)
			{
                goto collision;
			}

            g_warning("(Avahi) Failed to add %s service: %s", 
					  applet->avahi_service_name,
					  avahi_strerror(ret));
            return;
        }

        /* Tell the server to register the service */
		ret = avahi_entry_group_commit(applet->avahi_entry_group);
        if (ret < 0) 
		{
            g_warning("(Avahi) Failed to commit entry group: %s", avahi_strerror(ret));
            return;
        }
		
		g_debug("(Avahi) Publishing service %s", applet->avahi_service_name);
    }

    return;

collision:
	{
		/* A service name collision with a local service happened. Let's
 	     * pick a new name */
		gchar* n = avahi_alternative_service_name(applet->avahi_service_name);
		avahi_free(applet->avahi_service_name);
		applet->avahi_service_name = n;

		g_warning("(Avahi) Service name collision, renaming service to '%s'", n);

		avahi_entry_group_reset(applet->avahi_entry_group);
		synergy_applet_avahi_publish(applet);
		
		return;
	}
}
