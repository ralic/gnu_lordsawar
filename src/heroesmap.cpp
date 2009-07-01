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

#include "heroesmap.h"

#include "sdl-draw.h"
#include "city.h"
#include "citylist.h"
#include "playerlist.h"
#include "GraphicsCache.h"
#include "player.h"
#include "stacklist.h"
#include "hero.h"

HeroesMap::HeroesMap(std::list<Hero*> h)
{
  heroes = h;
  active_hero = *(heroes.begin());
}

void HeroesMap::draw_hero(Hero *hero, bool active)
{
  Player *player = Playerlist::getActiveplayer();
  Vector<int> start = player->getStacklist()->getPosition(hero->getId());

  start = mapToSurface(start);

  start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));

  SDL_Surface *tmp = GraphicsCache::getInstance()->getSmallHeroPic(active);
    
  SDL_Rect r;
  r.x = start.x - (tmp->w/2);
  if (r.x < 0)
    r.x = 0;
  r.y = start.y - (tmp->h/2);
  if (r.y < 0)
    r.y = 0;
  r.w = tmp->w;
  r.h = tmp->h;
  SDL_BlitSurface(tmp, 0, surface, &r);
}

void HeroesMap::after_draw()
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  OverviewMap::after_draw();
  draw_cities(false);
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end();
       it++)
    {
      if (*it == active_hero)
	draw_hero(*it, true);
      else
	draw_hero(*it, false);
    }

  map_changed.emit(surface);
}

void HeroesMap::mouse_button_event(MouseButtonEvent e)
{
  Player *active = Playerlist::getActiveplayer();
  if (e.button == MouseButtonEvent::LEFT_BUTTON && 
      e.state == MouseButtonEvent::PRESSED)
    {
      Vector<int> dest;
      dest = mapFromScreen(e.pos);

      //is dest close to one of our heroes?
      Hero *hero = active->getStacklist()->getNearestHero(dest, 4);
      if (hero)
	{
	  active_hero = hero;
	  hero_selected.emit(hero);
	}
    }

}