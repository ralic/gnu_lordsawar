//  Copyright (C) 2008, 2009, 2010 Ben Asselstine
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

#include <iostream>
#include <expat.h>
#include <assert.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "shieldsetlist.h"
#include "shieldset.h"
#include "File.h"
#include "defs.h"
#include "ucompose.hpp"
#include "tarhelper.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Shieldsetlist* Shieldsetlist::s_instance = 0;

Shieldsetlist* Shieldsetlist::getInstance()
{
    if (!s_instance)
        s_instance = new Shieldsetlist();

    return s_instance;
}

void Shieldsetlist::deleteInstance()
{
    if (s_instance)
      delete s_instance;

    s_instance = 0;
}


void Shieldsetlist::loadShieldsets(std::list<std::string> shieldsets)
{
    for (std::list<std::string>::const_iterator i = shieldsets.begin(); 
	 i != shieldsets.end(); i++)
      {
        Shieldset *shieldset = loadShieldset(*i);
	if (!shieldset)
	  continue;
	add(shieldset);
      }
}

Shieldsetlist::Shieldsetlist()
{
    // load all shieldsets
    std::list<std::string> shieldsets = Shieldset::scanSystemCollection();
    loadShieldsets(shieldsets);
    shieldsets = Shieldset::scanUserCollection();
    loadShieldsets(shieldsets);

}

Shieldsetlist::~Shieldsetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

std::list<std::string> Shieldsetlist::getNames() const
{
  std::list<std::string> names;
  for (const_iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  return names;
}
std::string Shieldsetlist::getShieldsetDir(std::string name) const
{
  DirMap::const_iterator it = d_dirs.find(name);
  if (it == d_dirs.end())
    return "";
  else
    return (*it).second;
}

Shieldset* Shieldsetlist::loadShieldset(std::string name)
{
  debug("Loading shieldset " <<File::get_basename(File::get_dirname(name)));

  Shieldset *shieldset = Shieldset::create(name);
  if (!shieldset)
    return NULL;

  if (shieldset->validate() == false)
    {
      cerr << "Error!  shieldset: `" << shieldset->getName() << 
	"' is invalid.  Skipping." << endl;
      delete shieldset;
      return NULL;
    }

  if (d_shieldsetids.find(shieldset->getId()) != d_shieldsetids.end())
    {
      Shieldset *s = (*d_shieldsetids.find(shieldset->getId())).second;
      cerr << "Error!  shieldset: `" << shieldset->getName() << 
	"' shares a duplicate shieldset id with `" << s->getConfigurationFile() 
	<< "'.  Skipping." << endl;
      delete shieldset;
      return NULL;
    }
    
  return shieldset;
}

void Shieldsetlist::add(Shieldset *shieldset)
{
  std::string subdir = File::get_basename(shieldset->getDirectory());
  push_back(shieldset);
  shieldset->setSubDir(subdir);
  d_dirs[shieldset->getName()] = subdir;
  d_shieldsets[subdir] = shieldset;
  d_shieldsetids[shieldset->getId()] = shieldset;
}
        
Gdk::Color Shieldsetlist::getColor(guint32 shieldset, guint32 owner) const
{
  Shieldset *s = getShieldset(shieldset);
  if (!s)
    return Gdk::Color("black");
  return s->getColor(owner);
}

ShieldStyle *Shieldsetlist::getShield(guint32 shieldset, guint32 type, guint32 colour) const
{
  Shieldset *s = getShieldset(shieldset);
  if (!s)
    return NULL;
  return s->lookupShieldByTypeAndColour(type, colour);
}

void Shieldsetlist::instantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiateImages();
}

void Shieldsetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}
	
Shieldset *Shieldsetlist::getShieldset(guint32 id)  const
{ 
  ShieldsetIdMap::const_iterator it = d_shieldsetids.find(id);
  if (it == d_shieldsetids.end())
    return NULL;
  return (*it).second;
}

Shieldset *Shieldsetlist::getShieldset(std::string dir) const
{ 
  ShieldsetMap::const_iterator it = d_shieldsets.find(dir);
  if (it == d_shieldsets.end())
    return NULL;
  return (*it).second;
}

Shieldset *Shieldsetlist::import(Tar_Helper *t, std::string f, bool &broken)
{
  std::string filename = t->getFile(f, broken);
  Shieldset *shieldset = Shieldset::create(filename);
  assert (shieldset != NULL);
  shieldset->setSubDir(File::get_basename(f));

  //extract all the files and remember where we extracted them
  std::list<std::string> delfiles;
  delfiles.push_back(filename);
  std::list<std::string> files;
  shieldset->getFilenames(files);
  for (std::list<std::string>::iterator i = files.begin(); i != files.end(); i++)
    {
      std::string file = t->getFile(*i + ".png", broken);
      delfiles.push_back (file);
    }

  std::string subdir = "";
  guint32 id = 0;
  addToPersonalCollection(shieldset, subdir, id);

  for (std::list<std::string>::iterator it = delfiles.begin(); it != delfiles.end(); it++)
    File::erase(*it);
  return shieldset;

}

bool Shieldsetlist::addToPersonalCollection(Shieldset *shieldset, std::string &new_subdir, guint32 &new_id)
{
  //do we already have this one?
      
  if (getShieldset(shieldset->getSubDir()) == getShieldset(shieldset->getId()) 
      && getShieldset(shieldset->getSubDir()) != NULL)
    {
      shieldset->setDirectory(getShieldset(shieldset->getId())->getDirectory());
      return true;
    }

  //if the subdir conflicts with any other subdir, then change it.
  if (getShieldset(shieldset->getSubDir()) != NULL)
    {
      if (new_subdir != "" && getShieldset(new_subdir) == NULL)
        shieldset->setSubDir(new_subdir);
      else
        {
          bool found = false;
          for (int count = 0; count < 100; count++)
            {
              new_subdir = String::ucompose("%1%2", shieldset->getSubDir(), count);
              if (getShieldset(new_subdir) == NULL)
                {
                  found = true;
                  break;
                }
            }
          if (found == false)
            return false;
          shieldset->setSubDir(new_subdir);
        }
    }
  else
    new_subdir = shieldset->getSubDir();

  //if the id conflicts with any other id, then change it
  if (getShieldset(shieldset->getId()) != NULL)
    {
      if (new_id != 0 && getShieldset(new_id) == NULL)
        shieldset->setId(new_id);
      else
        {
          new_id = Shieldsetlist::getNextAvailableId(shieldset->getId());
          shieldset->setId(new_id);
        }
    }
  else
    new_id = shieldset->getId();

  //make the directory where the shieldset is going to live.
  std::string directory = 
    File::getUserShieldsetDir() + shieldset->getSubDir() + "/";

  if (File::create_dir(directory) == false)
    return false;

  //okay now we copy the image files into the new directory 
  std::list<std::string> files;
  shieldset->getFilenames(files);
  for (std::list<std::string>::iterator it = files.begin(); it != files.end();
       it++)
    File::copy(shieldset->getFile(*it), directory + *it + ".png");

  //save out the shieldset file
  shieldset->setDirectory(directory);
  XML_Helper helper(shieldset->getConfigurationFile(), std::ios::out, false);
  shieldset->save(&helper);
  helper.close();
  add (shieldset);
  return true;
}

int Shieldsetlist::getNextAvailableId(int after)
{
  std::list<guint32> ids;
  std::list<std::string> shieldsets = Shieldset::scanSystemCollection();
  for (std::list<std::string>::const_iterator i = shieldsets.begin(); 
       i != shieldsets.end(); i++)
    {
      Shieldset *shieldset = Shieldset::create(*i);
      if (shieldset != NULL)
	{
	  ids.push_back(shieldset->getId());
	  delete shieldset;
	}
    }
  shieldsets = Shieldset::scanUserCollection();
  for (std::list<std::string>::const_iterator i = shieldsets.begin(); 
       i != shieldsets.end(); i++)
    {
      Shieldset *shieldset = Shieldset::create(*i);
      if (shieldset != NULL)
	{
	  ids.push_back(shieldset->getId());
	  delete shieldset;
	}
    }
  for (guint32 i = after + 1; i < 1000000; i++)
    {
      if (find(ids.begin(), ids.end(), i) == ids.end())
	return i;
    }
  return -1;
}

bool Shieldsetlist::contains(std::string name) const
{
  std::list<std::string> n = getNames();
  for (std::list<std::string>::iterator it = n.begin(); it != n.end(); it++)
    {
      if (*it == name)
        return true;
    }
  return false;
}
