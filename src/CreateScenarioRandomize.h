//  Copyright (C) 2008, 2009, 2014, 2017 Ben Asselstine
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
#ifndef CREATE_SCENARIO_RANDOMIZE_H
#define CREATE_SCENARIO_RANDOMIZE_H

#include <fstream>
#include <vector>
#include <gtkmm.h>
#include "shield.h"
#include "namelist.h"

class Signpost;
class Army;
class Player;
class Reward;

//! Generates random values for various map buildings.
class CreateScenarioRandomize
{
    public:
        CreateScenarioRandomize();
        virtual ~CreateScenarioRandomize() {};

	/** take a random city name
	 */
	Glib::ustring popRandomCityName();

	/* give a random city name
	 */
	void pushRandomCityName(Glib::ustring name);
	Glib::ustring popRandomRuinName();
	void pushRandomRuinName(Glib::ustring name);
	Glib::ustring popRandomTempleName();
	void pushRandomTempleName(Glib::ustring name);
	Glib::ustring popRandomSignpost();
	void pushRandomSignpost(Glib::ustring name);
	guint32 getRandomCityIncome(bool capital = false);
	bool randomSignpostsEmpty() {return d_signposts->empty();}
	Glib::ustring getDynamicSignpost(Signpost *signpost);
	int getNumSignposts() {return d_signposts->size();}
	Army * getRandomRuinKeeper(Player *p);
	Reward *getNewRandomReward();


	//! Based on the difficulty, get how much gold each player should
	//start with.
	void getBaseGold (int difficulty, int *base_gold);

	int adjustBaseGold (int base_gold);

	//! Get the default player name for the given shield colour.
	static Glib::ustring getPlayerName(Shield::Colour id);

    protected:
        //the namelists
	NameList *d_citynames;
	NameList *d_signposts;
	NameList *d_templenames;
	NameList *d_ruinnames;

    private:

	Glib::ustring popRandomListName(std::vector<Glib::ustring>& list);
        Glib::ustring get_direction(int xdir, int ydir);
	bool loadNames(std::vector<Glib::ustring>& list, std::ifstream& file);
};

#endif  //CREATE_SCENARIO_RANDOMIZE_H
