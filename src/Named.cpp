//  Copyright (C) 2008, Ben Asselstine
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

#include "Named.h"

#include "xmlhelper.h"

Named::Named(Glib::ustring name)
  :d_name(name)
{
}

Named::Named(const Named& object)
  :d_name(object.d_name)
{
}

Named::Named(XML_Helper* helper)
{
  if (!helper)
    return;
  helper->getData(d_name, "name");
}

Named::~Named()
{
}

