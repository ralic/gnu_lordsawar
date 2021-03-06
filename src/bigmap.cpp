// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2005 Bryan Duff
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006-2010, 2014, 2015, 2016, 2017 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include <assert.h>

#include "bigmap.h"

#include "army.h"
#include "Item.h"
#include "stacklist.h"
#include "stack.h"
#include "city.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "port.h"
#include "bridge.h"
#include "road.h"
#include "playerlist.h"
#include "File.h"
#include "stacktile.h"
#include "GameMap.h"
#include "ImageCache.h"
#include "MapRenderer.h"
#include "FogMap.h"
#include "MapBackpack.h"
#include "GameScenarioOptions.h"

#include <iostream>
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

BigMap::BigMap(bool headless)
    : d_headless (headless), d_renderer(0), buffer(0), d_fighting(LocationBox(Vector<int>(-1,-1)))
{
    // note: we are not fully initialized before set_view is called
    view.x = view.y = 0;
    view.w = 0;
    view.h = 0;
    deltax = 0;
    deltay = 0;
    view_pos = Vector<int>(0,0);
    input_locked = false;
    d_grid_toggled = false;

    blank_screen = false;
    image = Gtk::Allocation(0, 0, 320, 200);
}

BigMap::~BigMap()
{
    if (buffer == true)
      buffer.clear();

    delete d_renderer;
}

void BigMap::set_view(Rectangle new_view)
{
    int tilesize = GameMap::getInstance()->getTileSize();
    
    int width = image.get_width();
    int height = image.get_height();
    if (view.dim == new_view.dim && buffer && image.get_width() == width && image.get_height() == height)
    {
	// someone wants us to move the view, not resize it, no need to
	// construct new surfaces and all that stuff
	//
	// fixme: if we're moving the view, maybe there's some pixmap in common
	// between this view and the new view.  why render?

	view = new_view;
	Vector<int> new_view_pos = get_view_pos_from_view();

	if (view_pos != new_view_pos)
	{
	    view_pos = new_view_pos;
	    draw();
	}
	
	return;
    }

    view = new_view;
    view_pos = get_view_pos_from_view();
    
    // now create a buffer surface which is two maptiles wider and
    // higher than the screen you actually see. That is how smooth scrolling
    // becomes comparatively easy. You just blit from the extended screen to
    // the screen with some offset.
    // this represents a 1 tile border around the outside of the picture.
    // it gets rid of the black border.

    if (buffer == true)
      buffer.clear();
    
    buffer_view.dim = view.dim + Vector<int>(2, 2);

    Cairo::RefPtr<Cairo::Surface> empty = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, buffer_view.w *tilesize, buffer_view.h * tilesize);
    buffer = Cairo::Surface::create (empty, Cairo::CONTENT_COLOR_ALPHA, buffer_view.w * tilesize, buffer_view.h * tilesize);
    buffer_gc = Cairo::Context::create(buffer);

    //now create the part that will go out to the gtk::image
    if (outgoing == true)
      outgoing.clear();
    outgoing = Cairo::Surface::create(buffer, Cairo::CONTENT_COLOR_ALPHA, image.get_width(), image.get_height());


    if (d_renderer)
        delete d_renderer;
    // now set the MapRenderer so that it draws on the buffer
    d_renderer = new MapRenderer(buffer);
}

void BigMap::clip_viewable_buffer(Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> pos, Cairo::RefPtr<Cairo::Surface> out)
{
  Cairo::RefPtr<Cairo::Context> out_gc = Cairo::Context::create(out);
  out_gc->rectangle(0, 0, image.get_width(), image.get_height());
  out_gc->clip();
  out_gc->save();
  out_gc->set_source(pixmap, -pos.x, -pos.y);
  out_gc->rectangle (0, 0, image.get_width(), image.get_height());
  out_gc->clip();
  out_gc->paint();
  out_gc->restore();
  return;
}

void BigMap::draw(bool redraw_buffer)
{
    // no size and buffer yet, return
    if (!buffer || d_headless)
        return;
    //Playerlist::getInstance()->setViewingplayer(player);

    int tilesize = GameMap::getInstance()->getTileSize();

    // align the buffer view
    Vector<int> new_buffer_view = clip(
	Vector<int>(0, 0),
	view.pos - Vector<int>(1, 1),
	GameMap::get_dim() - buffer_view.dim + Vector<int>(1, 1));
    buffer_view.pos = new_buffer_view;

    // redraw the buffer
    if (redraw_buffer)
      draw_buffer();

    // blit the visible part of buffer to the screen
    Vector<int> p = view_pos - (buffer_view.pos * tilesize);
    outgoing.clear();
    outgoing = Cairo::Surface::create(buffer, Cairo::CONTENT_COLOR_ALPHA, image.get_width(), image.get_height());
    clip_viewable_buffer(buffer, p, outgoing);

    if (blank_screen)
      {
	int width = image.get_width();
	int height = image.get_height();
	Cairo::RefPtr<Cairo::Context> outgoing_gc = Cairo::Context::create(outgoing);
	outgoing_gc->set_source_rgba(FOG_COLOUR.get_red(), FOG_COLOUR.get_green(), FOG_COLOUR.get_blue(), FOG_COLOUR.get_alpha());
	outgoing_gc->rectangle(0, 0, width, height);
	outgoing_gc->fill();
      }
    map_changed.emit(outgoing);
}

void BigMap::screen_size_changed(Gtk::Allocation box)
{
    int ts = GameMap::getInstance()->getTileSize();

    Rectangle new_view = view;
    
    new_view.w = box.get_width() / ts;
    new_view.h = box.get_height() / ts;

    if (new_view.w <= GameMap::getWidth() && new_view.h <= GameMap::getHeight()
	&& new_view.w >= 0 && new_view.h >= 0)
      {
	new_view.pos = clip(Vector<int>(0,0), new_view.pos,
			    GameMap::get_dim() - new_view.dim);
	image = box;
	set_view(new_view);
	view_changed.emit(view);
      }
    image = box;
}

Vector<int> BigMap::get_view_pos_from_view()
{
    Vector<int> screen_dim(image.get_width(), image.get_height());
    int ts = GameMap::getInstance()->getTileSize();

    // clip to make sure we don't see a black border at the bottom and right
    return clip(Vector<int>(0, 0), view.pos * ts,
		GameMap::get_dim() * ts - screen_dim);
}

Vector<int> BigMap::tile_to_buffer_pos(Vector<int> tile)
{
    int ts = GameMap::getInstance()->getTileSize();
    return (tile - buffer_view.pos) * ts;
}

Vector<int> BigMap::mouse_pos_to_tile(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileSize();
    return (view_pos + pos) / ts;
}

Vector<int> BigMap::mouse_pos_to_tile_offset(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileSize();
    return (view_pos + pos) % ts;
}

MapTipPosition BigMap::map_tip_position(Vector<int> tile)
{
  return map_tip_position (Rectangle(tile.x, tile.y, 1, 1)); 
}

MapTipPosition BigMap::map_tip_position(Rectangle tile_area)
{
    // convert area to pixels on the screen
    int tilesize = GameMap::getInstance()->getTileSize();

    Rectangle area(tile_area.pos * tilesize - view_pos,
		   tile_area.dim * tilesize);

    // calculate screen edge distances
    int left, right, top, bottom;
    
    left = area.x;
    right = image.get_width() - (area.x + area.w);
    top = area.y;
    bottom = image.get_height() - (area.y + area.h);

    int const MARGIN = 2;
    
    // then set the position
    MapTipPosition m;
    if (right >= left && right >= top && right >= bottom)
    {
	m.pos.x = area.x + area.w + MARGIN;
	m.pos.y = area.y;
	m.justification = MapTipPosition::LEFT;
    }
    else if (left >= top && left >= bottom)
    {
	m.pos.x = area.x - MARGIN;
	m.pos.y = area.y;
	m.justification = MapTipPosition::RIGHT;
    }
    else if (bottom >= top)
    {
	m.pos.x = area.x;
	m.pos.y = area.y + area.h + MARGIN;
	m.justification = MapTipPosition::TOP;
    }
    else
    {
	m.pos.x = area.x;
	m.pos.y = area.y - MARGIN;
	m.justification = MapTipPosition::BOTTOM;
    }
    
    return m;
}

void BigMap::blit_object(const Location &obj, Vector<int> tile, PixMask *im, Cairo::RefPtr<Cairo::Surface> surface)
{
  Vector<int> diff = tile - obj.getPos();
  int tilesize = GameMap::getInstance()->getTileSize();
  Vector<int> p = tile_to_buffer_pos(tile);
  im->blit(diff, tilesize, surface, p);
}

void BigMap::draw_stack(Stack *s, Cairo::RefPtr<Cairo::Surface> surface)
{
  //this routine is for drawing the active stack.
  //for all other stacks see ImageCache::draw_tile_pic
  Vector<int> p = s->getPos();
  Player *player = s->getOwner();
  int tilesize = GameMap::getInstance()->getTileSize();

  // check if the object lies in the viewed part of the map
  // otherwise we shouldn't draw it
  if (is_inside(buffer_view, p) && !s->getDeleting())
    {
      if (s->empty())
	{
	  std::cerr << "WARNING: empty stack found" << std::endl;
	  return;
	}

      p = tile_to_buffer_pos(p);

      // draw stack

      bool show_army = true;
      //we don't show the army or the flag if we're in fortified tent.
      if (s->hasShip())
	{
          PixMask *ship = ImageCache::getInstance()->getShipPic(player)->copy();
          ship->scale (ship, tilesize, tilesize);
          ship->blit(surface, p);
          delete ship;
	}
      else
	{
	  if (s->getFortified() == true)
	    {
	      //We don't show the active stack here.
	      if (player->getStacklist()->getActivestack() != s &&
		  player == Playerlist::getActiveplayer())
		show_army = false;
	      Maptile *tile = GameMap::getInstance()->getTile(s->getPos());
	      if (tile->getBuilding() != Maptile::CITY &&
		  tile->getBuilding() != Maptile::RUIN &&
		  tile->getBuilding() != Maptile::TEMPLE)
                {
                  PixMask *tower = ImageCache::getInstance()->getTowerPic(player)->copy();
                  tower->scale (tower, tilesize, tilesize);
                  tower->blit(surface, p);
                  delete tower;
                }
	      else
		show_army = true;
	    }

	  if (show_army == true)
	    {
	      Army *a = *s->begin();
	      PixMask *armypic = ImageCache::getInstance()->getArmyPic(a)->copy();
              armypic->scale (armypic, tilesize, tilesize);
              armypic->blit(surface, p);
              delete armypic;
	    }
	}

      if (show_army)
        {
          //does our position have us on that stacktile?
          /*
           * sometimes the stack tile isn't updated right away.
           */
          StackTile *st = GameMap::getStacks(s->getPos());
          guint32 stacksize;
          if (!st->contains(s->getId()))
            stacksize = st->countNumberOfArmies(player) + s->size();
          else
            stacksize = st->countNumberOfArmies(player);
          if (stacksize > MAX_STACK_SIZE)
            stacksize = MAX_STACK_SIZE;
          if (stacksize > 0)
            {
              PixMask *flag = ImageCache::getInstance()->getFlagPic(stacksize, player)->copy();
              flag->scale (flag, tilesize, tilesize);
              flag->blit(surface, p);
              delete flag;

            }
        }
    }
}

void BigMap::draw_buffer()
{
  draw_buffer (buffer_view, buffer);
  // if we're hidden map this is hosed.
  after_draw();
  if (blank_screen == false)
    {
      ImageCache *gc = ImageCache::getInstance();
      int tilesize = GameMap::getInstance()->getTileSize();
      if (d_fighting.getPos() != Vector<int>(-1,-1))
        {
          Vector<int> p = tile_to_buffer_pos(d_fighting.getPos());
          PixMask *tmp = gc->getExplosionPic()->copy();
          if (d_fighting.getSize() > 1)
            {
              PixMask::scale(tmp, d_fighting.getSize() * tilesize,
                             d_fighting.getSize() * tilesize);
            }
          tmp->blit(buffer, p);
          delete tmp;
        }
    }
}

bool BigMap::saveAsBitmap(Glib::ustring filename)
{
  int tilesize = GameMap::getInstance()->getTileSize();
  int width = GameMap::getWidth() * tilesize;
  int height = GameMap::getHeight() * tilesize;
  Cairo::RefPtr<Cairo::Surface> empty = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, width, height);
  Cairo::RefPtr<Cairo::Surface> surf = Cairo::Surface::create (empty, Cairo::CONTENT_COLOR_ALPHA, width, height);
  
  bool orig_grid = d_grid_toggled;
  d_grid_toggled = false;
  draw_buffer(Rectangle (0, 0, GameMap::getWidth(), GameMap::getHeight()), surf);
  d_grid_toggled = orig_grid;
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create(surf, 0, 0, width, height);
  pixbuf->save (filename, "png");
  return true;
}

void BigMap::draw_buffer_tile(Vector<int> tile, Cairo::RefPtr<Cairo::Surface> surface)
{
  guint32 tilesize = GameMap::getInstance()->getTileSize();
  Player *viewing = Playerlist::getViewingplayer();
  ImageCache *gc = ImageCache::getInstance();
  int tile_style_id = GameMap::getInstance()->getTile(tile)->getTileStyle()->getId();
  int fog_type_id = 0;
  if (Playerlist::getViewingplayer()->getType() != Player::HUMAN &&
      GameScenarioOptions::s_hidden_map == true)
    fog_type_id = FogMap::ALL;
  else
    fog_type_id = viewing->getFogMap()->getShadeTile(tile);

  bool has_bag = false;
  bool has_standard = false;
  guint32 player_standard_id = 0;
  int stack_size = -1;
  int stack_player_id = -1;
  int army_type_id = -1;
  bool has_ship = false;
  bool has_tower = false;
  auto building_type = GameMap::getInstance()->getTile(tile)->getBuilding();
  Vector<int> building_tile = Vector<int>(-1,-1);
  int building_subtype = -1;
  int building_player_id = -1;

  if (fog_type_id == FogMap::ALL)
    {
      //short circuit.  the tile is completely fogged.
      PixMask *pixmask =
	gc->getTilePic(tile_style_id, fog_type_id, has_bag, has_standard, 
		       player_standard_id, stack_size, stack_player_id, 
		       army_type_id, has_tower, has_ship, building_type, 
		       building_subtype, building_tile, building_player_id, 
		       tilesize, d_grid_toggled);
      pixmask->blit(surface, tile_to_buffer_pos(tile));
      return;
    }
  MapBackpack *backpack = GameMap::getInstance()->getTile(tile)->getBackpack();
  if (backpack && backpack->empty() == false)
    {
      bool standard_planted = false;
      Item *flag = backpack->getFirstPlantedItem();
      if (flag)
	standard_planted = true;

      //only show one of the bag or the flag
      if (standard_planted && flag)
	{
	  has_standard = true;
	  player_standard_id = flag->getPlantableOwner()->getId();
	}
      else
	has_bag = true;
    }

  Stack *stack = GameMap::getStrongestStack(tile);
  if (stack)
    {
      if (viewing->getFogMap()->isCompletelyObscuredFogTile(tile) == false)
	{
	  //selected stack gets drawn in gamebigmap
	  if (Playerlist::getActiveplayer()->getActivestack() != stack)
	    {
	      stack_player_id = stack->getOwner()->getId();
	      Maptile *m = GameMap::getInstance()->getTile(tile);
	      if (stack->getFortified() == true &&
		  m->getBuilding() != Maptile::CITY &&
		  m->getBuilding() != Maptile::RUIN &&
		  m->getBuilding() != Maptile::TEMPLE)
		has_tower = true;
	      else if (stack->hasShip() == true)
                {
                  has_ship = true;
                  stack_size = GameMap::getStacks(stack->getPos())->countNumberOfArmies(Playerlist::getInstance()->getPlayer(stack_player_id));
                  if (stack_size > 0 && (guint)stack_size > MAX_STACK_SIZE)
                    stack_size = stack->size();
                  //here we show the number of armies on the tile.
                  //instead of the number of armies in the stack.
                  //so that stacks appear whole before we click on them.
                  //and that a stack of 1 can't hide a stack of 7.
                }
	      else
		{
		  army_type_id = (*stack->begin())->getTypeId();
                  stack_size = GameMap::getStacks(stack->getPos())->countNumberOfArmies(Playerlist::getInstance()->getPlayer(stack_player_id));
                  if (stack_size > 0 && (guint)stack_size > MAX_STACK_SIZE)
                    stack_size = stack->size();
		}
	    }
	}
    }

  if (building_type != Maptile::NONE)
    {
      switch (building_type)
	{
	case Maptile::CITY:
	    {
	      City *city = GameMap::getCity(tile);
	      building_player_id = city->getOwner()->getId();
	      building_tile = tile - city->getPos();
	      if (city->isBurnt())
		building_subtype = -1;
	      else
		building_subtype = 0;
	    }
	  break;
	case Maptile::RUIN:
	    {
	      Ruin *ruin = GameMap::getRuin(tile);
	      if (ruin->isHidden() == true && ruin->getOwner() == viewing)
		{
		  building_tile = tile - ruin->getPos();
		  building_subtype = ruin->getType();
		}
	      else if (ruin->isHidden() == false)
		{
		  building_tile = tile - ruin->getPos();
		  building_subtype = ruin->getType();
		}
	      else
		building_type = Maptile::NONE;
	    }
	  break;
	case Maptile::TEMPLE:
	    {
	      Temple *temple = GameMap::getTemple(tile);
	      building_tile = tile - temple->getPos();
	      building_subtype = temple->getType();
	    }
	  break;
	case Maptile::SIGNPOST:
	    {
	      Signpost *signpost = GameMap::getSignpost(tile);
	      building_tile = tile - signpost->getPos();
	    }
	  break;
	case Maptile::ROAD:
	    {
	      Road *road = GameMap::getRoad(tile);
	      building_tile = tile - road->getPos();
	      building_subtype = road->getType();
	    }
	  break;
	case Maptile::PORT:
	    {
	      Port *port = GameMap::getPort(tile);
	      building_tile = tile - port->getPos();
	    }
	  break;
	case Maptile::BRIDGE:
	    {
	      Bridge *bridge = GameMap::getBridge(tile);
	      building_tile = tile - bridge->getPos();
	      building_subtype = bridge->getType();
	    }
	  break;
	case Maptile::NONE: default:
	  break;
	}
    }
  PixMask *pixmask = 
    gc->getTilePic(tile_style_id, fog_type_id, has_bag, has_standard, 
		   player_standard_id, stack_size, stack_player_id, 
		   army_type_id, has_tower, has_ship, building_type, 
		   building_subtype, building_tile, building_player_id, 
		   tilesize, d_grid_toggled);
  pixmask->blit(surface, tile_to_buffer_pos(tile));
}

void BigMap::draw_buffer_tiles(Rectangle map_view, Cairo::RefPtr<Cairo::Surface> surface)
{
  for (int i = map_view.x; i < map_view.x + map_view.w; i++)
    for (int j = map_view.y; j < map_view.y + map_view.h; j++)
      if (i < GameMap::getWidth() && j < GameMap::getHeight())
	draw_buffer_tile(Vector<int>(i,j), surface);
}

void BigMap::draw_buffer(Rectangle map_view, Cairo::RefPtr<Cairo::Surface> surface)
{
  draw_buffer_tiles(map_view, surface);
}

void BigMap::toggle_grid()
{
  d_grid_toggled = !d_grid_toggled;
  draw(true);
}

void BigMap::blank(bool on)
{
  blank_screen = on;
  draw (Playerlist::getViewingplayer());
}

bool BigMap::scroll(GdkEventScroll *event)
{
  if (input_locked)
    return true;
  Rectangle n = view;
  switch (event->direction)
    {
    case GDK_SCROLL_SMOOTH:
        {
          double dx, dy;
          if (gdk_event_get_scroll_deltas ((GdkEvent*)event, &dx, &dy))
            {
              if (event->state & GDK_SHIFT_MASK)
                {
                  dx*=-1;
                  dy*=-1;
                }
              if (event->state & GDK_CONTROL_MASK)
                {
                  double swap;
                  swap = dx;
                  dx = dy;
                  dy = swap;
                }
              deltax += dx;
              deltay += dy;
              if (deltax <= -1.0 || deltax >= 1.0)
                {
                  n.x += deltax;
                  deltax += (int)deltax * -1;
                }
              if (deltay <= -1.0 || deltay >= 1.0)
                {
                  n.y += deltay;
                  deltay += (int)deltay * -1;
                }
            }
        }
      break;
    default:
      break;
    }
  if (n.x < 0)
    n.x = 0;
  else if (n.x > GameMap::getWidth()-1)
    n.x = GameMap::getWidth()-1;
  if (n.y < 0)
    n.y = 0;
  else if (n.y > GameMap::getHeight()-1)
    n.y = GameMap::getHeight()-1;
  set_view(n);
  view_changed.emit(view);
  return true;
}
