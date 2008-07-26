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

#ifndef PBM_H
#define PBM_H

#include <string>

class GameScenario;
class pbm
{
    public:
        pbm();
	~pbm();
	void init(std::string save_game_file);
	void run(std::string save_game_file, std::string turn_file);
	std::string getActiveplayerName() const {return d_player_name;};

    private:

	void humanize_active_player();
	void turn_all_players_to_networked();
	void playUntilFirstNetworkedPlayer(GameScenario *game_scenario);
	std::string d_player_name;
};

#endif // PBM_H

