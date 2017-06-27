
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include "browse.h"
#include "synergy-applet.h"

static gchar* get_name(const gchar* host_name, const gchar* domain)
{
	gchar* name = NULL;
	gchar* suffix = g_strconcat(".", domain, NULL);
	if (g_str_has_suffix(host_name, suffix))
	{
		name = g_strndup(host_name, strlen(host_name) - strlen(suffix));
	}
	else
	{
		name = g_strdup(host_name);
	}
	
	g_free(suffix);
	return name;
}

static void server_online(SynergyApplet* applet, const gchar* avahi_name, const gchar* simple_name, const gchar* address, const uint16_t port)
{
	SynergyHost* h = synergy_applet_host_find(applet->servers, NULL, avahi_name, address, port);
	if (h == NULL)
	{				
		h = synergy_applet_host_new(simple_name, avahi_name, address, port);
		applet->servers = g_list_prepend(applet->servers, h);
		h->online = TRUE;				
		g_debug("(Avahi) New server host: %s (%s:%u)", avahi_name, address, port);
		// TODO: Notify user
	}
	else
	{
		if (h->online == FALSE)
		{
			h->online = TRUE;
			g_debug("(Avahi) Server host online: %s (%s:%u)", avahi_name, address, port);
			// TODO: Notify user
			if (FALSE /* Used in client configuration */)
			{
				// TODO: Notfiy user / Autoconnect
			} else {
				// TODO: Notify user
			}
		}
	}
}

static void client_online(SynergyApplet* applet, const gchar* avahi_name, const gchar* simple_name, const gchar* address, const uint16_t port)
{
	SynergyHost* h = synergy_applet_host_find(applet->clients, NULL, simple_name, address, port);
	if (h == NULL)
	{				
		h = synergy_applet_host_new(simple_name, avahi_name, address, port);
		applet->servers = g_list_prepend(applet->clients, h);
		h->online = TRUE;				
		g_debug("(Avahi) New client host: %s (%s:%u)", avahi_name, address, port);
		// TODO: Notify user
	}
	else
	{
		if (h->online == FALSE)
		{
			h->online = TRUE;
			g_debug("(Avahi) Client host online: %s (%s:%u)", avahi_name, address, port);
			
			if (FALSE /* used in server configuration */)
			{
				// TODO: Notify user / Autoconnect
			}
			else
			{
				// TODO: Notify user
			}
		}
	}
}

static void server_offline(SynergyApplet* applet, const gchar* avahi_name)
{
	SynergyHost* h = synergy_applet_host_find(applet->servers, NULL, avahi_name, NULL, 0);
	if (h != NULL)
	{
		if (h->online)
		{
			h->online = FALSE;
			g_debug("(Avahi) Server host %s offline", avahi_name);
			// TODO: If using, terminate synergyc(?) and notify user
		}
	}
}

static void client_offline(SynergyApplet* applet, const gchar* avahi_name)
{
	SynergyHost* h = synergy_applet_host_find(applet->clients, NULL, avahi_name, NULL, 0);
	if (h != NULL)
	{
		if (h->online)
		{
			h->online = FALSE;
			g_debug("(Avahi) Client host %s offline", avahi_name);
			// TODO: Notify user
		}
	}
}

/**
 * Called whenever a service has been resolved successfully or timed out 
 */
static void resolve_callback(
    AvahiServiceResolver *service_resolver,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AvahiLookupResultFlags flags,
    SynergyApplet* applet) 
{    
    switch (event) 
	{
        case AVAHI_RESOLVER_FAILURE:
            g_warning("Failed to resolve service '%s' of type '%s' in domain '%s': %s", 
					name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(service_resolver))));
			
			// FIXME: Update lists, Notify user?
				
            break;

        case AVAHI_RESOLVER_FOUND: 
		{
            gchar a[AVAHI_ADDRESS_STR_MAX];
            
            g_debug("Service '%s' of type '%s' in domain '%s':", name, type, domain);
            
            avahi_address_snprint(a, sizeof(a), address);
			gchar* simple_name = get_name((gchar*)host_name, (gchar*)domain);

			if (g_strcmp0(type, SYNERGY_AVAHI_TYPE_CLIENT) == 0)
			{
				client_online(applet, name, simple_name, a, port);
			}
			else if (g_strcmp0(type, SYNERGY_AVAHI_TYPE_SERVER) == 0)
			{
				server_online(applet, name, simple_name, a, port);
			}			
			g_free(simple_name);
        }
    }

    avahi_service_resolver_free(service_resolver);
}

static void browse_callback(
    AvahiServiceBrowser* service_browser,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const gchar* name,
    const gchar* type,
    const gchar* domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    SynergyApplet* applet) 
{
    
    /* Called whenever a new services becomes available on the LAN or is removed from the LAN */

    switch (event) 
	{
        case AVAHI_BROWSER_FAILURE:            
            g_warning("(Avahi) %s", avahi_strerror(avahi_client_errno(applet->avahi_client)));            
            return;

        case AVAHI_BROWSER_NEW:
            g_debug("(Avahi) New service '%s' of type '%s' in domain '%s'", name, type, domain);

            /* We ignore the returned resolver object. In the callback
               function we free it. If the server is terminated before
               the callback function is called the server will free
               the resolver for us. */

            if (!(avahi_service_resolver_new(applet->avahi_client, 
											 interface, 
											 protocol, 
											 name, 
											 type, 
											 domain, 
											 AVAHI_PROTO_UNSPEC, 
											 0, 
											 resolve_callback, 
											 applet)))
			{
                g_debug("(Avahi) Failed to resolve service '%s': %s", name, 
						avahi_strerror(avahi_client_errno(applet->avahi_client)));
				// Switch client to offline, if used. Otherwise remove
			}
			
            break;

        case AVAHI_BROWSER_REMOVE:
            g_debug("(Avahi) Removed service '%s' of type '%s' in domain '%s'", name, type, domain);
			
			if (g_strcmp0(type, SYNERGY_AVAHI_TYPE_CLIENT) == 0)
			{
				client_offline(applet, name);
			}
			else if (g_strcmp0(type, SYNERGY_AVAHI_TYPE_SERVER) == 0)
			{
				server_offline(applet, name);
			}			
			

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            g_debug("(Avahi) %s", event == AVAHI_BROWSER_CACHE_EXHAUSTED 
					? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
            break;
    }
}

void synergy_applet_avahi_browse(SynergyApplet* applet)
{
	if (applet->avahi_service_browser != NULL)
	{
		avahi_service_browser_free(applet->avahi_service_browser);
		applet->avahi_service_browser = NULL;
	}

	if (applet->mode == SYNERGY_MODE_DISABLED)
	{
		return;
	}
	
	// Search for the opposite type
	applet->avahi_service_browser = avahi_service_browser_new(applet->avahi_client, 
		AVAHI_IF_UNSPEC, 
		AVAHI_PROTO_UNSPEC, 
		(applet->mode == SYNERGY_MODE_CLIENT 
			? SYNERGY_AVAHI_TYPE_SERVER : SYNERGY_AVAHI_TYPE_CLIENT), 
		NULL, 
		0, 
		browse_callback, 
		applet);
	
    if (applet->avahi_service_browser == NULL) 
	{
        g_warning("(Avahi) Failed to create service browser: %s\n", 
				  avahi_strerror(avahi_client_errno(applet->avahi_client)));
    }	

}

