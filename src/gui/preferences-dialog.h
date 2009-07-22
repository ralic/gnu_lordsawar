//  Copyright (C) 2007, Ole Laursen
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

#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <map>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm.h>

#include "vector.h"

#include "decorated.h"
class Game;
class Player;
// dialog for showing sound and game preferences
class PreferencesDialog: public Decorated
{
 public:
    PreferencesDialog(bool readonly);

    void set_parent_window(Gtk::Window &parent);

    void run(Game *game);
    void hide();

 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::CheckButton *show_turn_popup_checkbutton;
    Gtk::CheckButton *play_music_checkbutton;
    Gtk::Scale *music_volume_scale;
    Gtk::Box *music_volume_hbox;
    Gtk::VBox *players_vbox;

    bool d_readonly;
    void on_show_turn_popup_toggled();
    void on_play_music_toggled();
    void on_music_volume_changed();
    void on_observe_toggled(Gtk::CheckButton *button);
    void on_type_changed(Gtk::ComboBoxText *combo);

    typedef std::map<Player*, Gtk::ComboBoxText*> PlayerTypeMap;
    PlayerTypeMap player_types;

    typedef std::map<Player*, Gtk::CheckButton*> PlayerObserveMap;
    PlayerObserveMap player_observed;
};

#endif
