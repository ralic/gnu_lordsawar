//  Copyright (C) 2009, 2011, 2014 Ben Asselstine
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

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <vector>

#include "backpack-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "Item.h"
#include "ItemProto.h"
#include "Backpack.h"
#include "select-item-dialog.h"

#define method(x) sigc::mem_fun(*this, &BackpackEditorDialog::x)

BackpackEditorDialog::BackpackEditorDialog(Gtk::Window &parent, Backpack *pack)
 : LwEditorDialog(parent, "backpack-editor-dialog.ui")
{
  backpack = pack;
  working = new Backpack(*pack);
    
    xml->get_widget("remove_button", remove_button);
    xml->get_widget("add_button", add_button);
    remove_button->signal_clicked().connect(method(on_remove_item_clicked));
    add_button->signal_clicked().connect(method(on_add_item_clicked));
    
    item_list = Gtk::ListStore::create(item_columns);
    xml->get_widget("treeview", item_treeview);
    item_treeview->set_model(item_list);
    item_treeview->append_column(_("Name"), item_columns.name);
    item_treeview->append_column(_("Attributes"), item_columns.attributes);

    item_treeview->get_selection()->signal_changed().connect(method(on_item_selection_changed));
}

void BackpackEditorDialog::hide()
{
  dialog->hide();
}

int BackpackEditorDialog::run()
{
  dialog->show_all();
  fill_bag();
  on_item_selection_changed();
  int response = dialog->run();
  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      backpack->removeAllFromBackpack();
      backpack->add(working);
    }
  return response;
}

void BackpackEditorDialog::on_item_selection_changed()
{
    Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
    if (i)
      remove_button->set_sensitive(true);
    else
      remove_button->set_sensitive(false);
}

void BackpackEditorDialog::on_remove_item_clicked()
{
    Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
    if (i)
    {
	Item *item = (*i)[item_columns.item];
	working->removeFromBackpack(item);
	item_list->erase(item_treeview->get_selection()->get_selected());
	on_item_selection_changed();
    }
}

void BackpackEditorDialog::on_add_item_clicked()
{
  SelectItemDialog d(*dialog);
  d.run();
  guint32 id = 0;
  const ItemProto *itemproto = d.get_selected_item(id);
  if (itemproto)
    {
      Item *item = new Item(*itemproto, id);
      working->addToBackpack(item);
      add_item(item);
      on_item_selection_changed();
    }
}

void BackpackEditorDialog::add_item(Item *item)
{
    Gtk::TreeIter i = item_list->append();
    (*i)[item_columns.name] = item->getName();

    (*i)[item_columns.attributes] = item->getBonusDescription();
    
    (*i)[item_columns.item] = item;
}

void BackpackEditorDialog::fill_bag()
{
	
    // populate the item list
    item_list->clear();
    for (Backpack::iterator i = working->begin(); i != working->end(); ++i)
	add_item(*i);

  return;
}
