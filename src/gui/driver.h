//  Copyright (C) 2007, Ole Laursen
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

#ifndef GUI_DRIVER_H
#define GUI_DRIVER_H

#include <memory>
#include <string>
#include <sigc++/trackable.h>

#include "splash-window.h"
#include "game-window.h"
#include "../game-parameters.h"

// takes care of setting up the splash window and the game window, the
// interaction between them and the model classes
class Driver: public sigc::trackable
{
 public:
    Driver();
    ~Driver();

 private:
    std::auto_ptr<GameWindow> game_window;
    std::auto_ptr<SplashWindow> splash_window;

    void on_new_game_requested(GameParameters g);
    void on_load_requested(std::string filename);
    void on_quit_requested();

    void on_game_ended();

    void init_game_window();
};


#endif
