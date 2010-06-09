//  Copyright (C) 2010 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef TILESET_SMALLMAP_BUILDING_COLORS_DIALOG_H
#define TILESET_SMALLMAP_BUILDING_COLORS_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
class Tileset;

class TilesetSmallmapBuildingColorsDialog: public sigc::trackable
{
 public:
    TilesetSmallmapBuildingColorsDialog(Tileset *tileset);
    ~TilesetSmallmapBuildingColorsDialog();

    void set_parent_window(Gtk::Window &parent);

    int run();

    void hide();
    
 private:
    Tileset *d_tileset;
    Gtk::Dialog *dialog;
    Gtk::ColorButton *road_colorbutton;
    Gtk::ColorButton *ruin_colorbutton;
    Gtk::ColorButton *temple_colorbutton;

    void on_road_color_chosen();
    void on_ruin_color_chosen();
    void on_temple_color_chosen();
};

#endif
