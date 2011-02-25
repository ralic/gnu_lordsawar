//  Copyright (C) 2011 Ben Asselstine
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

//#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "profile.h"
#include "xmlhelper.h"
#include "ucompose.hpp"
#include "GameScenario.h"

std::string Profile::d_tag = "profile";

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Profile::Profile(std::string nickname)
{
  d_id = GameScenario::generate_guid();
  d_nickname = nickname;
  d_user = Glib::get_user_name();
  d_creation_date.assign_current_time();
  d_last_played_date.assign_current_time();
}

Profile::Profile(XML_Helper* helper)
{
  helper->getData(d_id, "id");
  helper->getData(d_nickname, "nickname");
  helper->getData(d_user, "user");
  std::string s;
  helper->getData(s, "created_on");
  d_creation_date.assign_from_iso8601(s);
  helper->getData(s, "last_played_on");
  d_last_played_date.assign_from_iso8601(s);
}

Profile::~Profile()
{
}

bool Profile::saveContents(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("nickname", d_nickname);
  retval &= helper->saveData("user", d_user);
  std::string s = d_creation_date.as_iso8601();
  retval &= helper->saveData("created_on", s);
  s = d_last_played_date.as_iso8601();
  retval &= helper->saveData("last_played_on", s);
  return retval;
}

Profile* Profile::handle_load(XML_Helper *helper)
{
  return new Profile(helper);
}

bool Profile::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(Profile::d_tag);
  retval &= saveContents(helper);
  retval &= helper->closeTag();
  return retval;
}

void Profile::play()
{
  d_last_played_date.assign_current_time();
}
