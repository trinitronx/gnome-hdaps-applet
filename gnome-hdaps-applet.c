/*
 * gnome-hdaps-applet
 * 2008-12-04
 * Copyright (C) 2006 - 2008 Jon Escombe <lists@dresco.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fcntl.h>
#include <string.h>
#include <panel-applet.h>
#include <gtk/gtk.h>

#define BUF_LEN				32
#define SYSFS_PROTECT_FILE_HDA		"/sys/block/hda/queue/protect"
#define SYSFS_PROTECT_FILE_SDA		"/sys/block/sda/queue/protect"
#define SYSFS_PROTECT_FILE_HDA_NEW	"/sys/block/hda/device/unload_heads"
#define SYSFS_PROTECT_FILE_SDA_NEW	"/sys/block/sda/device/unload_heads"

int check_status ()
{
	int fd, ret;
	char buf[BUF_LEN];

	memset(buf, 0, sizeof(buf));
	if ((fd = open (SYSFS_PROTECT_FILE_HDA, O_RDONLY)) < 1)
		if ((fd = open (SYSFS_PROTECT_FILE_SDA, O_RDONLY)) < 1)
			if ((fd = open (SYSFS_PROTECT_FILE_HDA_NEW, O_RDONLY)) < 1)
				if ((fd = open (SYSFS_PROTECT_FILE_SDA_NEW, O_RDONLY)) < 1)
			return -1;
	
	ret = read (fd, buf, BUF_LEN);
	close (fd);
	return atoi(buf);
}

gboolean timer_func (gpointer gdata)
{
	GtkWidget *image = gdata;
	GtkWidget *applet;
	GtkTooltipsData *tooltips_data;
	static int timerval = 0;
	gchar *text_label;
	int status;

	applet = image->parent;
	tooltips_data = gtk_tooltips_data_get (applet);

	status = check_status();
	if (status < 0) {
		gtk_image_set_from_file (GTK_IMAGE (image), "/usr/share/pixmaps/gnome-hdaps-applet/hdaps-error-small.png");
		gtk_tooltips_set_tip (tooltips_data->tooltips, applet,
				"Error in Hard Drive Protection System", "");
	} else if (status == 0) {
		gtk_image_set_from_file (GTK_IMAGE (image), "/usr/share/pixmaps/gnome-hdaps-applet/hdaps-running-small.png");
		gtk_tooltips_set_tip (tooltips_data->tooltips, applet,
				"Hard Drive Protection System is protecting "
				"your data", "");
	} else if (status > 0) {
		gtk_image_set_from_file (GTK_IMAGE (image), "/usr/share/pixmaps/gnome-hdaps-applet/hdaps-paused-small.png");
		gtk_tooltips_set_tip (tooltips_data->tooltips, applet,
				"Hard Drive Protection System has paused the "
				"hard drive to protect your data", "");
	}

	return TRUE;
}

static gboolean gnome_hdaps_applet_fill (PanelApplet *applet, const gchar *iid, gpointer data)
{
	GtkWidget *image;
	GtkTooltips *tooltips;
	guint timer;

	if (strcmp (iid, "OAFIID:GNOME_HDAPS_STATUSApplet") != 0)
		return FALSE;

	image = gtk_image_new_from_file ("/usr/share/pixmaps/gnome-hdaps-applet/hdaps-error-small.png");
	gtk_container_add (GTK_CONTAINER (applet), image);
	tooltips = gtk_tooltips_new ();
	gtk_tooltips_enable (tooltips);
	gtk_tooltips_set_tip (tooltips, GTK_WIDGET (applet), "", "");
	gtk_widget_show_all (GTK_WIDGET (applet));
	panel_applet_set_background_widget (PANEL_APPLET (applet), GTK_WIDGET (applet));

	timer = g_timeout_add (100, timer_func, (gpointer) image);

	return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_HDAPS_STATUS_Factory",
                             PANEL_TYPE_APPLET,
                             "HDAPS Status Applet",
                             "0",
                             gnome_hdaps_applet_fill,
                             NULL);
