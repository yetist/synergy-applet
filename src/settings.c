#include <glib/gi18n.h>
#include "settings.h"

static GtkListStore* synergy_applet_get_known_clients_tree_model(SynergyApplet* applet);

void synergy_applet_open_settings(SynergyApplet* applet)
{
	if (applet->settings == NULL)
	{	
		applet->settings = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Keyboard and mouse sharing"),
			NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
			NULL));

		GtkWidget* use_last_settings = gtk_check_button_new_with_mnemonic(_("Activate previous settings at startup"));	
		gtk_container_add(GTK_CONTAINER(applet->settings->vbox), use_last_settings);
		
		gtk_widget_show_all(GTK_WIDGET(applet->settings));
	}
	
	gtk_widget_show(GTK_WIDGET(applet->settings));
	gtk_dialog_run(applet->settings);
	gtk_widget_hide(GTK_WIDGET(applet->settings));
}


void synergy_applet_open_client_settings(SynergyApplet* applet)
{
	if (applet->client_settings == NULL)
	{
		applet->client_settings = GTK_DIALOG(gtk_dialog_new_with_buttons (_("Client settings"),
			NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
			NULL));
	}
	gtk_widget_show_all(GTK_WIDGET(applet->client_settings));
	gtk_dialog_run(applet->client_settings);
	gtk_widget_hide (GTK_WIDGET(applet->client_settings));	
}

void synergy_applet_open_server_settings(SynergyApplet* applet)
{
	if (applet->server_settings == NULL)
	{
		applet->server_settings = GTK_DIALOG(gtk_dialog_new_with_buttons (_("Server settings"),
			NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
			NULL));
		
		GtkTable* table = GTK_TABLE(gtk_table_new(3, 3, TRUE));
		gtk_container_add(GTK_CONTAINER(applet->server_settings->vbox), GTK_WIDGET(table));
		
		GtkListStore* model = synergy_applet_get_known_clients_tree_model(applet);
		
		// Top
		GtkComboBox* combo = GTK_COMBO_BOX(gtk_combo_box_new_with_model (GTK_TREE_MODEL(model)));
		gtk_table_attach(table, GTK_WIDGET(combo), 1, 2, 0, 1, 
						 0, 0, 0, 0);
		
		// Botton
		combo = GTK_COMBO_BOX(gtk_combo_box_new_text ());
		gtk_table_attach(table, GTK_WIDGET(combo), 1, 2, 2, 3, 
						 0, 0, 0, 0);
		
		// Left
		combo = GTK_COMBO_BOX(gtk_combo_box_new_text ());
		gtk_table_attach(table, GTK_WIDGET(combo), 0, 1, 1, 2,
						 0, 0, 0, 0);		
		
		// Right
		combo = GTK_COMBO_BOX(gtk_combo_box_new_text ());
		gtk_table_attach(table, GTK_WIDGET(combo), 2, 3, 1, 2,
						 0, 0, 0, 0);		
		
	}
	
	gtk_widget_show_all(GTK_WIDGET(applet->server_settings));
	gtk_dialog_run(applet->server_settings);
	gtk_widget_hide(GTK_WIDGET(applet->server_settings));
}

static GtkListStore* synergy_applet_get_known_clients_tree_model(SynergyApplet* applet)
{
	int i;
	GtkListStore* list_store = gtk_list_store_new (1, G_TYPE_STRING);
	GtkTreeIter iter;
	for (i=0; i<10; i++)
	{
		gtk_list_store_append(list_store, &iter);
	    gtk_list_store_set(list_store, &iter, 
			0, "Apa", -1);	
	}
	return list_store;
}

