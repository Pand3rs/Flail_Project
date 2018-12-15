#pragma once

#include "include/Engine_core.h"
#include "include/Light_core.h"
#include "include/Actor_core.h"
#include "include/Particle_core.h"

namespace My_Game
{
	namespace World
	{
		//manages a tileset image
		Tileset::Tileset tileset;

		//manages font
		Font::Font text;

		Grid::Point current_position;
		Grid::Grid map;
		Grid::Grid display_map;

		const int n_points = 3;
		Grid::Point points[n_points];

		Grid_Camera::Grid_Camera camera;
	}

	//do allocations and system initialization here
	void init(int screen_w, int screen_h)
	{
		srand(time(0));

		//initialize all systems and open game window
		Engine::init("hello", screen_w, screen_h);

		Font::init(&World::text, "font_tileset.txt", Engine::renderer);

		Tileset::init(&World::tileset, "tileset_levent.txt", Engine::renderer);

		Grid::init(&World::map, 48, 64);
		Grid::init(&World::display_map, 48, 64);

		Grid_Camera::init(&World::camera, Engine::screen_width, Engine::screen_height);
	}

	void begin_Play(unsigned int current_time)
	{
		World::camera.world_coord.x = 0;
		World::camera.world_coord.y = 0;
		World::camera.world_coord.w = World::display_map.n_cols;
		World::camera.world_coord.h = World::display_map.n_rows;

		memset(World::map.data, 0, sizeof(int)*World::map.n_cols*World::map.n_rows);
		memset(World::display_map.data, 0, sizeof(int)*World::display_map.n_cols*World::display_map.n_rows);

		World::current_position.x = World::display_map.n_cols / 2;
		World::current_position.y = World::display_map.n_rows / 2;

		for (int i = 0; i < World::n_points; i++)
		{
			World::points[i].x = rand() % 48;
			World::points[i].y = rand() % 64;
		}
	}

	int converter(int x, int y, int ncols)
	{
		return y * ncols + x;
	}

	void random_Walk_Step(Grid::Point *p, Grid::Grid *m)
	{
		switch (rand() % 4)
		{
		case 0:
			if (p->y > 1) p->y -= 1;
			break;
		case 1:
			if (p->x < m->n_cols - 1) p->x += 1;
			break;
		case 2:
			if (p->y < m->n_rows - 1) p->y += 1;
			break;
		case 3:
			if (p->x > 1) p->x -= 1;
			break;
		}
		m->data[converter(p->x, p->y, m->n_cols)] += 1;
	}

	void copy_map_to_display(Grid::Grid *m, Grid::Grid *dm)
	{
		for (int i = 0; i < m->n_rows * m->n_cols; i++)
		{
			if (m->data[i] == 0)
			{
				dm->data[i] = 0;
			}
			else if (m->data[i] == 1)
			{
				dm->data[i] = 1;
			}
			else if (m->data[i] == 2)
			{
				dm->data[i] = 7;
			}
			else if (m->data[i] == 3)
			{
				dm->data[i] = 8;
			}
			else if (m->data[i] == 4)
			{
				dm->data[i] = 12;
			}
			else if (m->data[i] == 5)
			{
				dm->data[i] = 13;
			}
			else if (m->data[i] >= 6)
			{
				dm->data[i] = 14;
			}
		}
	}

	void update(unsigned int current_time, float dt)
	{
		Engine::event_Loop();
		Grid_Camera::calibrate(&World::camera);

		for (int i = 0; i < World::n_points; i++)
		{
			random_Walk_Step(&World::points[i], &World::map);
		}
		copy_map_to_display(&World::map, &World::display_map);
	}

	void draw(unsigned int current_time)
	{
		SDL_RenderClear(Engine::renderer);

		Tileset::draw_Grid(&World::tileset, &World::camera, &World::display_map, &RGBA::default, Engine::renderer);

		//flip buffers
		SDL_RenderPresent(Engine::renderer);
	}
}