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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef BUY_PRODUCTION_DIALOG_H
#define BUY_PRODUCTION_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/button.h>

class Army;
class City;

// dialog for buying a production slot for a city
class BuyProductionDialog: public sigc::trackable
{
 public:
    BuyProductionDialog(City *city);

    void set_parent_window(Gtk::Window &parent);

    void run();

    enum { NO_ARMY_SELECTED = -1 };
    int get_selected_army() { return selected_army; }
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::Label *production_info_label1;
    Gtk::Label *production_info_label2;
    Gtk::Button *buy_button;

    City *city;
    int selected_army;

    std::vector<Gtk::ToggleButton *> production_toggles;
    bool ignore_toggles;

    void on_production_toggled(Gtk::ToggleButton *toggle);
    void fill_in_production_info();
    void set_buy_button_state();
    const Army *army_id_to_army();
};

#endif
