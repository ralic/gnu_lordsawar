//  Copyright (C) 2007, 2008 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
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

#ifndef FIGHT_ORDER_DIALOG_H
#define FIGHT_ORDER_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <list>
#include <SDL.h>

class Stack;
class Player;

#include "decorated.h"

// dialog for showing and changing the order in which army types fight
// in battle
class FightOrderDialog: public Decorated
{
 public:
    FightOrderDialog(Player *player);

    void set_parent_window(Gtk::Window &parent);

    void hide();

    void run();

 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Player *player;
    Gtk::TreeView *armies_treeview;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(image); add(name); add(army_type);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Uint32> army_type;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;
 private:
    void addArmyType(Uint32 army_type);
};

#endif
