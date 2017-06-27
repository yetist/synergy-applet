
#include "synergy-applet.h"
#include "settings.h"
#include "browse.h"
#include "publish.h"
#include <avahi-client/client.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-glib/glib-watch.h>
#include <avahi-glib/glib-malloc.h>
#include <glib/gi18n.h>


/* self typedefs */
typedef SynergyApplet Self;
typedef SynergyAppletClass SelfClass;

static void synergy_applet_init (SynergyApplet * o) G_GNUC_UNUSED;
static void synergy_applet_class_init (SynergyAppletClass * c) G_GNUC_UNUSED;

static void on_status_icon_popup_menu(GtkStatusIcon* status_icon, guint button, guint activate_time, SynergyApplet* applet);
static void on_status_icon_activate(GtkStatusIcon* status_icon, SynergyApplet* applet);
static void on_settings_activated(GtkMenuItem* menu_item, SynergyApplet* applet);
static void on_quit_activated(GtkMenuItem* menu_item, SynergyApplet* applet);
static void on_change_mode_activated(GtkMenuItem* menu_item, SynergyApplet* applet);

static void avahi_client_callback (AVAHI_GCC_UNUSED AvahiClient *client, AvahiClientState state, SynergyApplet* applet);

/* pointer to the class of our parent */
static GObjectClass *parent_class = NULL;

GType
synergy_applet_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = {
			sizeof (SynergyAppletClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) synergy_applet_class_init,
			(GClassFinalizeFunc) NULL,
			NULL /* class_data */,
			sizeof (SynergyApplet),
			0 /* n_preallocs */,
			(GInstanceInitFunc) synergy_applet_init,
			NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT, "SynergyApplet", &info, (GTypeFlags)0);
	}

	return type;
}

/* a macro for creating a new object of our type */
#define GET_NEW ((SynergyApplet *)g_object_new(synergy_applet_get_type(), NULL))

/* a function for creating a new object of our type */
#include <stdarg.h>
static SynergyApplet * GET_NEW_VARG (const char *first, ...) G_GNUC_UNUSED;
static SynergyApplet *
GET_NEW_VARG (const char *first, ...)
{
	SynergyApplet *ret;
	va_list ap;
	va_start (ap, first);
	ret = (SynergyApplet *)g_object_new_valist (synergy_applet_get_type (), first, ap);
	va_end (ap);
	return ret;
}


static void
___dispose (GObject *obj_self)
{
	SynergyApplet *self G_GNUC_UNUSED = SYNERGY_APPLET (obj_self);
	if (G_OBJECT_CLASS (parent_class)->dispose) \
		(* G_OBJECT_CLASS (parent_class)->dispose) (obj_self);
	
	if(self->main_loop) 
	{ 
		g_main_loop_unref ((gpointer) self->main_loop); 
		self->main_loop = NULL; 
	}

	if(self->status_icon_disabled) 
	{ 
		g_object_unref ((gpointer) self->status_icon_disabled); 
		self->status_icon_disabled = NULL; 
	}

	if(self->status_icon_server_enabled) 
	{		
		g_object_unref ((gpointer) self->status_icon_server_enabled);
		self->status_icon_server_enabled = NULL; 
	}
	
	if(self->status_icon_client_enabled) 
	{
		g_object_unref ((gpointer) self->status_icon_client_enabled);
		self->status_icon_client_enabled = NULL; 
	}
	
	if(self->menu) 
	{
		gtk_widget_destroy ((gpointer) self->menu);
		self->menu = NULL; 
	}
	
	if (self->settings) 
	{
		gtk_widget_destroy(GTK_WIDGET(self->settings));
		self->settings = NULL;
	}

	if (self->client_settings) 
	{
		gtk_widget_destroy(GTK_WIDGET(self->client_settings));
		self->client_settings = NULL;
	}
	
	if (self->server_settings) 
	{
		gtk_widget_destroy(GTK_WIDGET(self->server_settings));
		self->server_settings = NULL;	
	}
	
	if (self->avahi_client)
	{
		avahi_client_free(self->avahi_client);
	}
	
	if (self->avahi_glib_poll)
	{
		avahi_glib_poll_free(self->avahi_glib_poll);
	}

	if (self->avahi_service_name)
	{
		g_free(self->avahi_service_name);
	}
}


static void
___finalize(GObject *obj_self)
{
	SynergyApplet *self G_GNUC_UNUSED = SYNERGY_APPLET (obj_self);
	if(G_OBJECT_CLASS(parent_class)->finalize) \
		(* G_OBJECT_CLASS(parent_class)->finalize)(obj_self);
}


static void 
synergy_applet_init (SynergyApplet * o G_GNUC_UNUSED)
{
	o->main_loop = NULL; 

	o->mode =  SYNERGY_MODE_DISABLED; ;
	o->clients = NULL ;
	o->servers = NULL ;

	o->status_icon_disabled =  
		gtk_status_icon_new_from_stock (GTK_STOCK_STOP);
		gtk_status_icon_set_tooltip (o->status_icon_disabled, 
									 _("Keyboard and mouse sharing disabled"));

	o->status_icon_server_enabled =  
		gtk_status_icon_new_from_stock (GTK_STOCK_NETWORK);
		gtk_status_icon_set_tooltip (o->status_icon_server_enabled, 
									 _("Sharing keyboard and mouse"));

	o->status_icon_client_enabled =  
		gtk_status_icon_new_from_stock (GTK_STOCK_CONNECT);
		gtk_status_icon_set_tooltip (o->status_icon_client_enabled,
									 _("Using keyboard and mouse from another computer"));

	o->menu = GTK_MENU(gtk_menu_new()); 

	o->settings = NULL; 
	o->client_settings = NULL; 
	o->server_settings = NULL;
	o->avahi_entry_group = NULL;
	o->avahi_service_name = NULL;
	
	if (o->avahi_service_browser)
	{
        avahi_service_browser_free(o->avahi_service_browser);
	}
}

static void 
synergy_applet_class_init (SynergyAppletClass * c G_GNUC_UNUSED)
{
	GObjectClass *g_object_class G_GNUC_UNUSED = (GObjectClass*) c;

	parent_class = g_type_class_ref (G_TYPE_OBJECT);

	g_object_class->dispose = ___dispose;
	g_object_class->finalize = ___finalize;
}


#define SYNERGY_MODE_PROPERTY "mode"

void synergy_applet_start(SynergyApplet* applet, GMainLoop * main_loop)
{
	if (applet->main_loop != NULL)
	{
		return;
	}
	
	applet->main_loop = main_loop;
	g_main_loop_ref(main_loop);
	

	gtk_status_icon_set_visible (applet->status_icon_disabled, TRUE);
	gtk_status_icon_set_visible (applet->status_icon_server_enabled, FALSE);
	gtk_status_icon_set_visible (applet->status_icon_client_enabled, FALSE);
		
	// Menu

	GtkWidget* server = gtk_radio_menu_item_new_with_mnemonic(NULL, 
		_("_Control other computers with my keyboard and mouse"));
	GSList* group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(server));
	gtk_menu_shell_append(GTK_MENU_SHELL(applet->menu), server);
	g_signal_connect_after(server, "activate", (GCallback)on_change_mode_activated, applet);
	g_object_set_data (G_OBJECT(server), SYNERGY_MODE_PROPERTY, GINT_TO_POINTER(SYNERGY_MODE_SERVER));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(server), FALSE);
	gtk_widget_show(server);

	GtkWidget* client = gtk_radio_menu_item_new_with_mnemonic(group, 
		_("_Use keyboard and mouse connected to another computer"));
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(server));
	gtk_menu_shell_append(GTK_MENU_SHELL(applet->menu), client);
	g_signal_connect_after(client, "activate", (GCallback)on_change_mode_activated, applet);
	g_object_set_data (G_OBJECT(client), SYNERGY_MODE_PROPERTY, GINT_TO_POINTER(SYNERGY_MODE_CLIENT));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(client), FALSE);
	gtk_widget_show(client);

	GtkWidget* disabled = gtk_radio_menu_item_new_with_mnemonic(group, _("_Disable"));
	gtk_menu_shell_append(GTK_MENU_SHELL(applet->menu), disabled);
	g_signal_connect_after(disabled, "activate", (GCallback)on_change_mode_activated, applet);
	g_object_set_data (G_OBJECT(disabled), SYNERGY_MODE_PROPERTY, GINT_TO_POINTER(SYNERGY_MODE_DISABLED));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(disabled), TRUE);
	gtk_widget_show(disabled);	

	GtkWidget* settings = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(applet->menu), settings);
	gtk_widget_show(settings);
	g_signal_connect_after(settings, "activate", (GCallback)on_settings_activated, applet);
	
	GtkWidget* quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(applet->menu), quit);
	gtk_widget_show(quit);
	g_signal_connect_after(quit, "activate", (GCallback)on_quit_activated, applet);
	
	
	g_signal_connect_after(applet->status_icon_disabled, "popup-menu", (GCallback)on_status_icon_popup_menu, applet);
	g_signal_connect_after(applet->status_icon_server_enabled, "popup-menu", (GCallback)on_status_icon_popup_menu, applet);
	g_signal_connect_after(applet->status_icon_client_enabled, "popup-menu", (GCallback)on_status_icon_popup_menu, applet);
	
	g_signal_connect_after(applet->status_icon_disabled, "activate", (GCallback)on_status_icon_activate, applet);
	g_signal_connect_after(applet->status_icon_server_enabled, "activate", (GCallback)on_status_icon_activate, applet);
	g_signal_connect_after(applet->status_icon_client_enabled, "activate", (GCallback)on_status_icon_activate, applet);	

	// Set up avahi poll
	int error;
    applet->avahi_glib_poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
    applet->avahi_poll_api = avahi_glib_poll_get(applet->avahi_glib_poll);

    applet->avahi_client = avahi_client_new (applet->avahi_poll_api,
		0,
		(AvahiClientCallback)avahi_client_callback,
		applet,
		&error);
	
    if (applet->avahi_client == NULL)
    {
        g_error("Error initializing Avahi: %s", avahi_strerror (error));
    }
}

/**
 * Right click on the status icon
 */
static void on_status_icon_popup_menu(GtkStatusIcon* status_icon, guint button, guint activate_time, SynergyApplet* applet)
{		
	gtk_menu_popup (applet->menu, NULL, NULL, 
		gtk_status_icon_position_menu, status_icon, 
		button, 
		activate_time);
}

/**
 * Left click on the status icon
 */
static void on_status_icon_activate(GtkStatusIcon* status_icon, SynergyApplet* applet)
{
	
	if (applet->mode == SYNERGY_MODE_CLIENT)
	{
		synergy_applet_open_client_settings (applet);
	}
	else if (applet->mode == SYNERGY_MODE_SERVER)
	{
		synergy_applet_open_server_settings (applet);
	}	
}

/**
 * Open the settings menu
 */
static void on_settings_activated(GtkMenuItem* menu_item, SynergyApplet* applet)
{
	synergy_applet_open_settings(applet);
}

static void on_quit_activated(GtkMenuItem* menu_item, SynergyApplet* applet)
{
	g_main_loop_quit(applet->main_loop);
}

static void on_change_mode_activated(GtkMenuItem* menu_item, SynergyApplet* applet)
{
	if (applet->avahi_client != NULL)
	{
		SynergyMode mode = GPOINTER_TO_INT(g_object_get_data (G_OBJECT(menu_item), SYNERGY_MODE_PROPERTY));
		synergy_applet_set_mode(applet, mode);
	}
}

void synergy_applet_set_mode(SynergyApplet* applet, SynergyMode mode)
{
	if (applet->mode == mode)
	{
		return;
	}
	
	applet->mode = mode;

	gtk_status_icon_set_visible (applet->status_icon_disabled, (mode == SYNERGY_MODE_DISABLED));
	gtk_status_icon_set_visible (applet->status_icon_server_enabled, (mode == SYNERGY_MODE_SERVER));
	gtk_status_icon_set_visible (applet->status_icon_client_enabled, (mode == SYNERGY_MODE_CLIENT));
	
	if (applet->client_settings != NULL && GTK_WIDGET_VISIBLE(applet->client_settings))
	{
		gtk_widget_hide (GTK_WIDGET(applet->client_settings));
	}
	
	if (applet->server_settings != NULL && GTK_WIDGET_VISIBLE(applet->server_settings))
	{
		gtk_widget_hide (GTK_WIDGET(applet->server_settings));
	}
	
	if (mode != SYNERGY_MODE_DISABLED)
	{
		// In both cases, we publish our precense
		// FIXME: Check client state before registering
		// synergy_applet_avahi_publish(applet);	
		
		synergy_applet_avahi_publish (applet);
		synergy_applet_avahi_browse(applet);
		
		if (mode == SYNERGY_MODE_CLIENT)
		{	
			if (FALSE /* we not yet has set up a server, or the server is not available */)
			{
				synergy_applet_open_client_settings(applet);
			}
		}
		else if (mode == SYNERGY_MODE_SERVER)
		{
			if (FALSE /* No clients are set up */)
			{
				synergy_applet_open_server_settings(applet);
			}
		}
	}

}

/* Callback for state changes on the Client */
static void avahi_client_callback (AVAHI_GCC_UNUSED AvahiClient* client, AvahiClientState state, SynergyApplet* applet)
{
	g_debug("(Avahi) State changed");
	
    switch (state) 
	{
        case AVAHI_CLIENT_S_RUNNING:

            /* The server has startup successfully and registered its host
             * name on the network, so it's time to create our services */
			if (applet->mode != SYNERGY_MODE_DISABLED)
			{
				synergy_applet_avahi_publish(applet);
			}
            break;

        case AVAHI_CLIENT_FAILURE:

            g_warning("(Avahi) Client failure: %s\n", 
					  avahi_strerror(avahi_client_errno(applet->avahi_client)));
            break;

        case AVAHI_CLIENT_S_COLLISION:

            /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */

        case AVAHI_CLIENT_S_REGISTERING:

            /* The server records are now being established. This
             * might be caused by a host name change. We need to wait
             * for our own records to register until the host name is
             * properly esatblished. */

            if (applet->avahi_entry_group != NULL)
			{
                avahi_entry_group_reset(applet->avahi_entry_group);
			}
			
            break;

        case AVAHI_CLIENT_CONNECTING:
            ;
    }	
}

SynergyHost* synergy_applet_host_new(const gchar* name, const gchar* avahi_name, const gchar* address, const uint16_t port)
{
	SynergyHost* h = g_new0(SynergyHost, 1);	
	h->name = g_strdup(name);
	h->avahi_name = g_strdup(avahi_name);
	h->address = g_strdup(address);
	h->port = port;	
	h->online = FALSE;
	return h;
}

void synergy_applet_host_free(SynergyHost* host)
{
	if (host->name) g_free(host->name);
	if (host->avahi_name) g_free(host->avahi_name);
	if (host->address) g_free(host->address);
	g_free(host);
}

/**
 * Return the first matching host.
 * If any of the parameters is null, that parameter is not matched.
 */
SynergyHost* synergy_applet_host_find(GList* hosts, const gchar* name, const gchar* avahi_name, const gchar* address, const uint16_t port)
{
	GList* i = hosts;
	for (i = hosts; i != NULL; i = g_list_next(i))
	{
		SynergyHost* h = (SynergyHost*)(i->data);
		if (name != NULL && g_strcmp0(name, h->name))
			continue;
		
		if (avahi_name != NULL && g_strcmp0(avahi_name, h->avahi_name))
			continue;
		
		if (address != NULL && g_strcmp0(address, h->address))
			continue;
		
		if (port != 0 && (port != h->port))
			continue;
		
		return h;
	}
	
	return NULL;
}
