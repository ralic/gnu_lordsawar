// Copyright (C) 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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

#ifndef CITYSET_H
#define CITYSET_H

#include <vector>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "PixMask.h"
#include "set.h"

#include "defs.h"

class XML_Helper;

//! A list of city graphic objects in a city theme.
/** 
 * Every scenario has a city set; it is the theme of the city graphics 
 * within the game. 
 *
 * The Cityset dictates the size of city images.
 *
 * Citysets are referred to by their base name.  The base name is the last
 * part of the file's path minus the file extension.
 *
 * The cityset configuration file is a tar file that contains an XML file, 
 * and a set of png files.  Filenames have the following form:
 * cityset/${Cityset::d_basename}.lwc.
 */
class Cityset : public sigc::trackable, public Set
{
    public:
	//! The xml tag of this object in a cityset configuration file.
	static Glib::ustring d_tag; 
	static Glib::ustring file_extension; 

	//! Default constructor.
	/**
	 * Make a new Cityset.
	 *
	 * @param id    The unique Id of this Cityset among all other Cityset
	 *              objects.  Must be more than 0.  
	 * @param name  The name of the Cityset.  Analagous to Cityset::d_name.
	 */
	Cityset(guint32 id, Glib::ustring name);

        //! Copy constructor.
        Cityset(const Cityset& c);

	//! Loading constructor.
	/**
	 * Make a new Cityset object by reading it in from the cityset
	 * configuration file.
	 *
	 * @param helper  The opened cityset configuration file to load the
	 *                Cityset from.
	 */
        Cityset(XML_Helper* helper, Glib::ustring directory);

	static Cityset *create(Glib::ustring file, bool &unsupported_version);

        static Cityset *copy (const Cityset *orig);
	//! Destructor.
        ~Cityset();

	bool save(XML_Helper *helper) const;

        bool save(Glib::ustring filename, Glib::ustring extension) const;

	//! Get the base name of the file holding the cityset configuration file.
        Glib::ustring getBaseName() const {return d_basename;}

	//! Set the base name of the shieldset configuration file.
        void setBaseName(Glib::ustring base) {d_basename = base;}

        //! Returns the name of the cityset.
        Glib::ustring getName() const {return _(d_name.c_str());}

        //! Returns the copyright holders for the cityset.
        Glib::ustring getCopyright() const {return d_copyright;};

        //! Returns the license for the cityset.
        Glib::ustring getLicense() const {return d_license;};

	/**
	 * Analagous to the cityset.d_id XML entity in the cityset
	 * configuration file.
	 */
        guint32 getId() const {return d_id;}

	//! Set the unique identifier for this cityset.
        void setId(guint32 id) {d_id = id;}

	//! Set the name of the cityset.
	/**
	 * @note This method is only used in the cityset editor.
	 */
        void setName(Glib::ustring name) {d_name = name;}

	//! Set the copyright holders on the cityset.
	void setCopyright(Glib::ustring copy) {d_copyright = copy;};

	//! Set the license for this cityset.
	void setLicense(Glib::ustring license) {d_license = license;};

        //! Returns the description of the cityset.
        Glib::ustring getInfo() const {return _(d_info.c_str());}

	//! Sets the description of the cityset.
	void setInfo(Glib::ustring description) {d_info = description;};

        //! Returns the width and height in pixels of a square on the map.
        guint32 getTileSize() const {return d_tileSize;}

        void setTileSize(guint32 tile_size) {d_tileSize = tile_size;}

	void setCitiesFilename(Glib::ustring s) {d_cities_filename = s;};
	Glib::ustring getCitiesFilename() {return d_cities_filename;};
	void setRazedCitiesFilename(Glib::ustring s) {d_razedcities_filename = s;};
	Glib::ustring getRazedCitiesFilename() {return d_razedcities_filename;};
	void setPortFilename(Glib::ustring s) {d_port_filename = s;};
	Glib::ustring getPortFilename() {return d_port_filename;};
	void setSignpostFilename(Glib::ustring s) {d_signpost_filename = s;};
	Glib::ustring getSignpostFilename() {return d_signpost_filename;};
	void setRuinsFilename(Glib::ustring s) {d_ruins_filename = s;};
	Glib::ustring getRuinsFilename() {return d_ruins_filename;};
	void setTemplesFilename(Glib::ustring s) {d_temples_filename = s;};
	Glib::ustring getTemplesFilename() {return d_temples_filename;};
	void setTowersFilename(Glib::ustring s) {d_towers_filename = s;};
	Glib::ustring getTowersFilename() {return d_towers_filename;};

	void setCityImage(guint32 i, PixMask *p) {citypics[i] = p;};
	PixMask *getCityImage(guint32 i) {return citypics[i];};
	void setRazedCityImage(guint32 i, PixMask *p) {razedcitypics[i] = p;};
	PixMask *getRazedCityImage(guint32 i) {return razedcitypics[i];};
	PixMask *getPortImage() {return port;};
	void setPortImage(PixMask *p) {port = p;};
	PixMask *getSignpostImage() {return signpost;};
	void setSignpostImage(PixMask *p) {signpost = p;};
	void setRuinImage(guint32 i, PixMask *p) {ruinpics[i] = p;};
	PixMask *getRuinImage(guint32 i) {return ruinpics[i];};
	void setTempleImage(guint32 i, PixMask *p) {templepics[i] = p;};
	PixMask *getTempleImage(guint32 i) {return templepics[i];};
	void setTowerImage(guint32 i, PixMask *p) {towerpics[i] = p;};
	PixMask *getTowerImage(guint32 i) {return towerpics[i];};

	//! get filenames in this cityset, excepting the configuration file.
	void getFilenames(std::list<Glib::ustring> &files);

        //! Delete the cityset's temporary directory.
        void clean_tmp_dir() const;

	void instantiateImages(bool &broken);
	void instantiateImages(Glib::ustring port_filename,
			       Glib::ustring signpost_filename,
			       Glib::ustring cities_filename,
			       Glib::ustring razed_cities_filename,
			       Glib::ustring towers_filename,
			       Glib::ustring ruins_filename,
			       Glib::ustring temples_filename,
                               bool &broken);
	void uninstantiateImages();

	Glib::ustring getConfigurationFile() const;

	static std::list<Glib::ustring> scanSystemCollection();
	static std::list<Glib::ustring> scanUserCollection();
        static bool copy(Glib::ustring src, Glib::ustring dest);
        guint32 countEmptyImageNames() const;
	guint32 getCityTileWidth() {return d_city_tile_width;};
	void setCityTileWidth(guint32 tiles) {d_city_tile_width = tiles;};
	guint32 getTempleTileWidth() {return d_temple_tile_width;};
	void setTempleTileWidth(guint32 tiles) {d_temple_tile_width = tiles;};
	guint32 getRuinTileWidth() {return d_ruin_tile_width;};
	void setRuinTileWidth(guint32 tiles) {d_ruin_tile_width = tiles;};
	bool validate();
	bool validateCitiesFilename();
	bool validateRazedCitiesFilename();
	bool validateSignpostFilename();
	bool validatePortFilename();
	bool validateRuinsFilename();
	bool validateTemplesFilename();
	bool validateTowersFilename();
	bool validateCityTileWidth();
	bool validateRuinTileWidth();
	bool validateTempleTileWidth();
	bool tileWidthsEqual(Cityset *cityset);
        Glib::ustring getFileFromConfigurationFile(Glib::ustring file);
        bool replaceFileInConfigurationFile(Glib::ustring file, Glib::ustring new_file);

        //! Callback to convert old files to new ones.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

        //! Load the cityset again.
        void reload(bool &broken);
        guint32 calculate_preferred_tile_size() const;
    private:

        // DATA
	//! A unique numeric identifier among all citysets.
	guint32 d_id;

	//! The name of the cityset.
	/**
	 * This equates to the cityset.d_name XML entity in the cityset
	 * configuration file.
	 * This name appears in the dialogs where the user is asked to 
	 * select a particular Cityset.
	 */
        Glib::ustring d_name;

	//! The copyright holders for this cityset.
	Glib::ustring d_copyright;

	//! The license of this cityset.
	Glib::ustring d_license;

	//! The description of the cityset.
	/**
	 * Equates to the cityset.d_info XML entity in the cityset 
	 * configuration file.
	 * This value is not used.
	 */
        Glib::ustring d_info;

	//! The size of each city image onscreen.
	/**
	 * Equates to the cityset.d_tilesize XML entity in the cityset
	 * configuration file.
	 * It represents the size in pixels of the width and height of city
	 * imagery onscreen.
	 */
        guint32 d_tileSize;

	//! The base name of the cityset.
	/**
         * The basename is the final portion of the file's path, but with the
         * file extension removed.
	 * Cityset directories sit in the citysets/ directory.
	 */
        Glib::ustring d_basename;


	Glib::ustring d_cities_filename;
	Glib::ustring d_razedcities_filename;
	Glib::ustring d_port_filename;
	Glib::ustring d_signpost_filename;
	Glib::ustring d_ruins_filename;
	Glib::ustring d_temples_filename;
	Glib::ustring d_towers_filename;
	PixMask *citypics[MAX_PLAYERS + 1];
	PixMask *razedcitypics[MAX_PLAYERS];
	PixMask *port;
	PixMask *signpost;
	PixMask *ruinpics[RUIN_TYPES];
	PixMask *templepics[TEMPLE_TYPES];
	PixMask *towerpics[MAX_PLAYERS];

	guint32 d_city_tile_width;
	guint32 d_temple_tile_width;
	guint32 d_ruin_tile_width;
};

#endif // CITYSET_H

// End of file
