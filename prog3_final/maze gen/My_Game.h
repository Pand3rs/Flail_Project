#include "Stack_Example.h"
#include "include/Engine_core.h"
#include "include/Light_core.h"
#include "include/Actor_core.h"
#include "include/Particle_core.h"


namespace My_Game
{
	void init_Maze(Grid::Grid *g);
	
	void next_Maze_Step(Stack::Stack *a, Grid::Grid *g);

	namespace World
	{
		namespace Parameters
		{
			const int maze_n_rows = 80;
			const int maze_n_cols = 128;
			int step_frequency = 50;
		}

		Tileset::Tileset tileset;

		RGBA::RGBA rgba = {255, 255, 255, 255};
		Grid::Grid maze;
		Grid::Point position;

		Grid_Camera::Grid_Camera camera;

		Stack::Stack logistics;

		unsigned int last_step_time = 0;
	}

	void init(int screen_w, int screen_h)
	{
		Engine::init("hello", screen_w, screen_h);

		Tileset::init(&World::tileset, "tileset_levent.txt", Engine::renderer);

		Grid::init(&World::maze, World::Parameters::maze_n_rows, World::Parameters::maze_n_cols);

		Grid_Camera::init(&World::camera, Engine::screen_width, Engine::screen_height);
		World::camera.world_coord.x = 0;
		World::camera.world_coord.y = 0;
		World::camera.world_coord.w = World::Parameters::maze_n_cols;
		World::camera.world_coord.h = World::Parameters::maze_n_rows;
	}

	void begin_Play(unsigned int current_time)
	{
		World::position.x = 1;
		World::position.y = 1;
		init_Maze(&World::maze);
		init(&World::logistics, 4919);
	}

	void update(unsigned int current_time, float dt)
	{
		Engine::event_Loop();

		if (current_time - World::last_step_time >= World::Parameters::step_frequency)
		{
			World::last_step_time = current_time;
			next_Maze_Step(&World::logistics, &World::maze);
		}
	}

	void draw(unsigned int current_time)
	{
		SDL_RenderClear(Engine::renderer);

		Grid_Camera::calibrate(&World::camera);
		Tileset::draw_Grid(&World::tileset, &World::camera, &World::maze, &World::rgba , Engine::renderer);

		SDL_RenderPresent(Engine::renderer);
	}

	void init_Maze(Grid::Grid *g)
	{
		for (int i = 0; i < World::Parameters::maze_n_rows; i++)
		{
			for (int j = 0; j < World::Parameters::maze_n_cols; j++)
			{
				g->data[i * g->n_cols + j] = 0;
			}
		}
		g->data[World::position.y * World::maze.n_cols + World::position.x] = 1;
	}

	void next_Maze_Step(Stack::Stack *a, Grid::Grid *g)
	{
		for (;;)
		{
			int goL = 0, goR = 0, goU = 0, goD = 0, paths = 0;
			
			if (World::position.x - 2 > 0)
			{
				int index = World::position.y * World::maze.n_cols + World::position.x - 2;
				if (g->data[index] == 0)
				{
					goL = 1;
					paths += 1;
				}
			}
			if (World::position.x + 2 < World::Parameters::maze_n_cols)
			{
				int index = World::position.y * World::maze.n_cols + World::position.x + 2;
				if (g->data[index] == 0)
				{
					goR = 1;
					paths += 1;
				}
			}
			if (World::position.y - 2 > 0)
			{
				int index = (World::position.y - 2) * World::maze.n_cols + World::position.x;
				if (g->data[index] == 0)
				{
					goU = 1;
					paths += 1;
				}
			}
			if (World::position.y + 2 < World::Parameters::maze_n_rows)
			{
				int index = (World::position.y + 2) * World::maze.n_cols + World::position.x;
				if (g->data[index] == 0)
				{
					goD = 1;
					paths += 1;
				}
			}

			int r = rand() % 4;
			
			if (r == 0 && goL == 1)
			{
				g->data[World::position.y * World::maze.n_cols + World::position.x - 2] = 1;
				g->data[World::position.y * World::maze.n_cols + World::position.x - 1] = 1;
				World::position.x -= 2;
				Stack::push(a, World::position.y * World::maze.n_cols + World::position.x, paths);
				break;
			}
			if (r == 1 && goR == 1)
			{
				g->data[World::position.y * World::maze.n_cols + World::position.x + 2] = 1;
				g->data[World::position.y * World::maze.n_cols + World::position.x + 1] = 1;
				World::position.x += 2;
				Stack::push(a, World::position.y * World::maze.n_cols + World::position.x, paths);
				break;
			}
			if (r == 2 && goU == 1)
			{
				g->data[(World::position.y - 2) * World::maze.n_cols + World::position.x] = 1;
				g->data[(World::position.y - 1) * World::maze.n_cols + World::position.x] = 1;
				World::position.y -= 2;
				Stack::push(a, World::position.y * World::maze.n_cols + World::position.x, paths);
				break;
			}
			if (r == 3 && goD == 1)
			{
				g->data[(World::position.y + 2) * World::maze.n_cols + World::position.x] = 1;
				g->data[(World::position.y + 1) * World::maze.n_cols + World::position.x] = 1;
				World::position.y += 2;
				Stack::push(a, World::position.y * World::maze.n_cols + World::position.x, paths);
				break;
			}
			else if (goL == 0 && goR == 0 && goU == 0 && goD == 0)
			{
				int tmp_x = World::position.x;
				int tmp_y = World::position.y;
				g->data[(tmp_y * World::maze.n_cols + tmp_x)] = 10;
				Stack::pop(a);
				int t = a->data[a->n_data];
				World::position.y = t / World::maze.n_cols;
				World::position.x = t - (World::position.y * World::maze.n_cols);
				int dif_x = (World::position.x - tmp_x) / 2;
				int dif_y = (World::position.y - tmp_y) / 2;
				g->data[(tmp_y + dif_y) * World::maze.n_cols + (tmp_x + dif_x)] = 10;
			}
		}
	}
}