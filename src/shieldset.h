//  Copyright (C) 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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

#pragma once
#ifndef SHIELDSET_H
#define SHIELDSET_H

#include <gtkmm.h>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "shield.h"
#include "set.h"
#include "defs.h"

class XML_Helper;

//! A list of Shield graphic objects in a shield theme.
/**
 * Every scenario has a shield set; it is the theme of the shield graphics 
 * within the game.  Shields come in three sizes -- small, medium and large.  
 * Small shields appear on the OverviewMap.  Medium shields appear in the turn 
 * indicator in the top right of the GameWindow.  Large shields appear in many 
 * dialogs, chiefly the FightWindow, and DiplomacyDialog.
 * Every shield belongs to one of 9 players (the ninth is the Neutral player).
 * The players aren't Player objects in this case; instead it refers to a 
 * Shield::ShieldColour.  e.g. Not `The Sirians' but rather the `White player'
 * of the scenario.
 *
 * The Shieldset dictates the dimensions of these three sizes of shields.
 *
 * Shieldsets are referred to by their basename.  This is the last part of the
 * filename, minus the file extension.
 *
 * The shieldset configuration file is a tar file that contains an XML file, 
 * and a set of png files.  Filenames have the following form:
 * shield/${Shieldset::d_basename}.lws.
 */
class Shieldset: public std::list<Shield *>, public sigc::trackable, public Set
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static Glib::ustring d_tag; 

	//! The file extension for shieldset files.  It includes the dot.
	static Glib::ustring file_extension;


	//! Default constructor.
	/**
	 * Make a new shieldset given a unique id and a basename name.
	 */
	Shieldset(guint32 id, Glib::ustring name);

        //! Copy constructor.
        Shieldset(const Shieldset& s);

	//! Load a Shieldset from an opened shieldset configuration file.
	/**
	 * Make a new Shieldset object by reading it in from the shieldset
	 * configuration file.
	 *
	 * @param helper  The opened shieldset configuration file to load the
	 *                Shieldset from.
	 */
        Shieldset(XML_Helper* helper, Glib::ustring directory);

	//! Destructor.
        ~Shieldset();

	// Get Methods

	//! Return the mask colour for the given player.
	Gdk::RGBA getColor(guint32 owner) const;

	//! Return the number of pixels high the small shields are.
	guint32 getSmallHeight() const {return d_small_height;}

	//! Return the number of pixels wide the small shields are.
	guint32 getSmallWidth() const {return d_small_width;}

	//! Return the number of pixels high the medium shields are.
	guint32 getMediumHeight() const {return d_medium_height;}

	//! Return the number of pixels wide the medium shields are.
	guint32 getMediumWidth() const {return d_medium_width;}

	//! Return the number of pixels the large shields are.
	guint32 getLargeHeight() const {return d_large_height;}

	//! Return the number of pixels wide the large shields are.
	guint32 getLargeWidth() const {return d_large_width;}

	//! Return the total number of shields in this shieldset.
        guint32 getSize() const {return size();}


	// Set Methods

        //! Load the shieldset again.
        void reload(bool &broken);

        //! Set the dimensions based on the largest image of that shieldstyle.
        void setHeightsAndWidthsFromImages();

	// Methods that operate on the class data but do not modify the class.

	bool save(XML_Helper *helper) const;

        bool save(Glib::ustring filename, Glib::ustring extension) const;

	//! Find the shield of a given size and colour in this Shieldset.
	/**
	 * Scan through all Shield objects in this set for first one that is 
	 * the desired size, and for the desired player.
	 *
	 * @param type    One of the values in Shield::ShieldType.
	 * @param colour  One of the values in Shield::ShieldColour.
	 *
	 * @return A pointer to the shield that matches the size and player.
	 *         If no Shield object could be found that matches the given
	 *         parameters, NULL is returned.
	 */
	ShieldStyle* lookupShieldByTypeAndColour(guint32 type, guint32 colour) const;

	//! Get filenames in this shieldset, excepting the configuration file.
	void getFilenames(std::list<Glib::ustring> &filenames) const;

	//! Check to see if this shieldset can be used in the game.
	bool validate() const;

	//! Check to see if the number of shields is sufficient.
	bool validateNumberOfShields() const;

	//! Check to see if the images for the shieldset are supplied.
	bool validateShieldImages(Shield::Colour c) const;

        guint32 countEmptyImageNames() const;

	// Methods that operate on the class data and also modify the class.

	//! Load images associated with this shieldset.
	void instantiateImages(bool &broken);

	//! Destroy images associated with this shieldset.
	void uninstantiateImages();

	// Static Methods

	//! Create a shieldset from the given shieldset configuration file.
	static Shieldset *create(Glib::ustring filename, bool &unsupported);

        static Shieldset *copy (const Shieldset *orig);

        //! rewrite old shieldset files.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

    private:

	//! Callback function to load Shield objects into the Shieldset.
	bool loadShield(Glib::ustring tag, XML_Helper* helper);

	// DATA

	//! The number of pixels high the small shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_small_height XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_small_height;

	//! The number of pixels wide the small shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_small_width XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_small_width;

	//! The number of pixels high the medium shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_medium_height XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_medium_height;

	//! The number of pixels wide the medium shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_medium_width XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_medium_width;

	//! The number of pixels high the large shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_large_height XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_large_height;

	//! The number of pixels wide the large shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_large_width XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_large_width;
};

#endif // SHIELDSET_H

