// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2008 Ben Asselstine
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

#ifndef GAME_CLIENT_DECODER_H
#define GAME_CLIENT_DECODER_H

#include "config.h"
#include "chat-client.h"

#include <list>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
class XML_Helper;
#include "network-action.h"
#include "network-history.h"

class GameScenario;
class Player;

class GameClientDecoder: public ChatClient
{
public:
  GameClientDecoder();
  ~GameClientDecoder();

  sigc::signal<void, std::string> game_scenario_received;
  sigc::signal<void, Player *> remote_player_moved;
  sigc::signal<void, Player *> remote_player_named;
  sigc::signal<void, Player *> remote_player_died;

protected:
  class ActionLoader 
    {
  public:
      bool loadAction(std::string tag, XML_Helper* helper)
	{
	  if (tag == Action::d_tag)
	    {
	      NetworkAction *action = &*actions.back();
	      action->setAction(Action::handle_load(helper));
	      return true;
	    }
	  if (tag == NetworkAction::d_tag) 
	    {
	      NetworkAction * action = new NetworkAction(helper);
	      actions.push_back(action);
	      return true;
	    }
	  return false;
	}

      std::list<NetworkAction *> actions;
    };
  class HistoryLoader 
    {
  public:
      bool loadHistory(std::string tag, XML_Helper* helper)
	{
	  if (tag == History::d_tag)
	    {
	      NetworkHistory *history = &*histories.back();
	      history->setHistory(History::handle_load(helper));
	      return true;
	    }
	  if (tag == NetworkHistory::d_tag) 
	    {
	      NetworkHistory* history = new NetworkHistory(helper);
	      histories.push_back(history);
	      return true;
	    }
	  return false;
	}

      std::list<NetworkHistory *> histories;
    };


protected:
  void gotActions(const std::string &payload);
  void gotHistories(const std::string &payload);
  void gotScenario(const std::string &payload);
  int decodeActions(std::list<NetworkAction*> actions,
		    Player *player);
  int decodeHistories(std::list<NetworkHistory*> histories);

};

#endif
