#pragma once

#include "include/Stack_core.h"
#include "include/Engine_core.h"
#include "include/Light_core.h"
#include "include/Actor_core.h"
#include "include/Particle_core.h"

namespace My_Game
{

	namespace World
	{

		namespace Parameters
		{
			Vec2D::Vec2D player_spawn_point = {4, 4};
			int player_force_mag = 250;
			float player_clip = 10.0;

			int bullet_life = 5000;
			float bullet_force_magnitude = 600;
			float bullet_clip = 16.0;
			unsigned int last_bullet_time = 0;
			unsigned int bullet_freq = 120;

			int num_enemies = 10;
		
			int min_chain_len = 2;
			int max_chain_len = 40;
			int flail_link_force_min = 50;
			int flail_link_force_max = 50;

			float separation_force_mag = 0.2 / 2;
			float cohesion_force_mag = 0.2 / 2;
			float alignment_force_mag = 0.2 / 2;
			float target_force_mag = 14 / 2;
			//float obstacle_force_mag = 32.0 / 2;

			//float obstacle_radius = 2.0;
			float separation_radius = 0.2;
			float cohesion_radius = 2.0;
		}

		Tileset::Tileset tileset;
		Font::Font text;

		Grid::Grid map;
		Grid::Grid collision;
		Grid::Grid imprint;
		Grid::Grid particle_imprint;

		Actor::Factory player;
		Actor::Factory enemy;
		Actor::Factory bullet;
		Particle::Emitter flail_link;

		Grid_Camera::Grid_Camera camera;

		Vec2D::Vec2D mouse_grid_point;
		Vec2D::Vec2D *chain;
		Vec2D::Vec2D calm_chain_distance = { 2, 2 };
		Stack::Stack links;
		int *neighbor_array = NULL;
	}

	void set_chain(Vec2D::Vec2D *p, Vec2D::Vec2D *e);

	void init(int screen_w, int screen_h)
	{
		srand(time(0));
		//initialize all systems and open game window
		Engine::init("hello", screen_w, screen_h);

		Font::init(&World::text, "font_tileset.txt", Engine::renderer);

		Tileset::init(&World::tileset, "map_tileset.txt", Engine::renderer);

		Grid::load(&World::map, "sandbox_map.csv");
		Grid::load(&World::collision, "sandbox_collision.csv");
		Grid::init(&World::imprint, World::map.n_rows, World::map.n_cols);
		Grid::init(&World::particle_imprint, World::map.n_rows, World::map.n_cols);

		Actor::init(&World::player, 2);
		Actor::init(&World::enemy, 512);
		Actor::init(&World::bullet, 1024);

		Actor::add_Animated_Sprite(&World::player, "saitama_pink_run.txt", Engine::renderer);
		Actor::add_Animated_Sprite(&World::enemy, "box.txt", Engine::renderer);
		Actor::add_Animated_Sprite(&World::bullet, "box.txt", Engine::renderer);

		Particle::init(&World::flail_link, "box.txt", 4096, Engine::renderer);

		World::chain = (Vec2D::Vec2D*)malloc(sizeof(int) * World::Parameters::max_chain_len);
		Stack::init(&World::links, World::Parameters::max_chain_len);
		World::neighbor_array = (int *)malloc(sizeof(int) * 4096);

		Grid_Camera::init(&World::camera, Engine::screen_width, Engine::screen_height);
	}

	void begin_Play(unsigned int current_time)
	{
		World::camera.world_coord.x = 0;
		World::camera.world_coord.y = 0;
		World::camera.world_coord.w = 50;
		World::camera.world_coord.h = World::camera.world_coord.w * Engine::screen_height / Engine::screen_width;

		int player_id = Actor::spawn(&World::player, 4.0, current_time);
		float x = World::Parameters::player_spawn_point.x;
		float y = World::Parameters::player_spawn_point.y;
		Actor::set_Pos(player_id, x, y, &World::player);

		/*for (int y = 1; y < World::map.n_rows-1; y+=2)
		{
		for (int x = 1; x < World::map.n_cols - 1; x+=2)
		{
		double p = (double)rand() / RAND_MAX;
		if (p <= 0.1)
		{
		int enemy_id = Actor::spawn(&World::enemy, 1.0, current_time);
		Actor::set_Pos(enemy_id, x, y, &World::enemy);
		}
		}
		}*/

		for (int i = 0; i < World::Parameters::num_enemies; i++)
		{
			double x = 1.0 + (World::map.n_cols - 2) * rand() / RAND_MAX;
			double y = 1.0 + (World::map.n_rows - 2) * rand() / RAND_MAX;

			int enemy_id = Actor::spawn(&World::enemy, 2.0, current_time);
			Actor::set_Pos(enemy_id, x, y, &World::enemy);
		}
	}

	void update(unsigned int current_time, float dt)
	{
		Engine::event_Loop();

		//UPDATE CAMERA
		World::camera.world_coord.x = Actor::get_World_Coord(0, &World::player)->x - 0.5*World::camera.world_coord.w;
		World::camera.world_coord.y = Actor::get_World_Coord(0, &World::player)->y - 0.5*World::camera.world_coord.h;

		if (World::camera.world_coord.x < 0) World::camera.world_coord.x = 0;
		if (World::camera.world_coord.y < 0) World::camera.world_coord.y = 0;
		if (World::camera.world_coord.x >= World::map.n_cols - World::camera.world_coord.w) World::camera.world_coord.x = World::map.n_cols - World::camera.world_coord.w;
		if (World::camera.world_coord.y >= World::map.n_rows - World::camera.world_coord.h) World::camera.world_coord.y = World::map.n_rows - World::camera.world_coord.h;

		Grid_Camera::calibrate(&World::camera);
		Grid_Camera::screen_to_Grid(&World::mouse_grid_point, Input::mouse_x, Input::mouse_y, &World::camera);

		//INPUT
		if (Input::keys[SDL_SCANCODE_A])
		{
			Vec2D::Vec2D force = { -1 * World::Parameters::player_force_mag,0 };
			Actor::add_Force(0, &World::player, &force);
		}
		if (Input::keys[SDL_SCANCODE_D])
		{
			Vec2D::Vec2D force = { World::Parameters::player_force_mag,0 };
			Actor::add_Force(0, &World::player, &force);
		}
		if (Input::keys[SDL_SCANCODE_W])
		{
			Vec2D::Vec2D force = { 0,-1 * World::Parameters::player_force_mag };
			Actor::add_Force(0, &World::player, &force);
		}
		if (Input::keys[SDL_SCANCODE_S])
		{
			Vec2D::Vec2D force = { 0,World::Parameters::player_force_mag };
			Actor::add_Force(0, &World::player, &force);
		}

		if (Input::mouse_left == 1 && Input::prev_mouse_left == 1)
		{
			if (current_time - World::Parameters::last_bullet_time > World::Parameters::bullet_freq)
			{
				World::Parameters::last_bullet_time = current_time;

				int bullet_id = Actor::spawn(&World::bullet, 0.4, current_time);
				Shape::Rect::Data *player_rect = Actor::get_World_Coord(0, &World::player);
				Actor::set_Pos(bullet_id, player_rect->x + player_rect->w*0.5, player_rect->y + player_rect->h*0.5, &World::bullet);
				Vec2D::Vec2D *bullet_pos = Actor::get_Pos(bullet_id, &World::bullet);

				Vec2D::Vec2D force = World::mouse_grid_point;
				Vec2D::sub(&force, bullet_pos);

				Vec2D::norm(&force);

				Vec2D::scale(&force, World::Parameters::bullet_force_magnitude);
				Actor::add_Force(bullet_id, &World::bullet, &force);
			}
		}

		//UPDATE PLAYER
		Actor::update_Vel(0, &World::player, dt);
		Actor::get_Vel(0, &World::player)->x *= 0.95;
		Actor::get_Vel(0, &World::player)->y *= 0.95;
		Vec2D::clip(Actor::get_Vel(0, &World::player), -1 * World::Parameters::player_clip, World::Parameters::player_clip, -1 * World::Parameters::player_clip, World::Parameters::player_clip);
		Actor::update_Pos(0, &World::player, dt);

		//RESET COLLISION GRID
		int len = World::imprint.n_rows*World::imprint.n_cols;
		for (int i = 0; i < len; i++)
		{
			World::imprint.data[i] = -1;
		}

		//ENEMY COLLISION
		for (int i = 0; i < World::enemy.array_size; i++)
		{
			if (Actor::is_Spawned(i, &World::enemy) == 0) continue;

			Actor::update_Vel(i, &World::enemy, dt);
			//limit enemy speed
			Vec2D::clip(Actor::get_Vel(i, &World::enemy), -2.0, 2.0, -2.0, 2.0);

			//ENEMY COLLISION
			Grid::Region enemy_imprint_region;
			Actor::get_Grid_Collision(&enemy_imprint_region, &World::imprint, i, &World::enemy);

			for (int y = enemy_imprint_region.first_row; y <= enemy_imprint_region.last_row; y++)
			{
				for (int x = enemy_imprint_region.first_col; x <= enemy_imprint_region.last_col; x++)
				{
					//there is a possible collision another enemy
					int e = Grid::get_Tile(x, y, &World::imprint);
					if (e != -1 && e != i)
					{
						Shape::Rect::Data *enemy_world = Actor::get_World_Coord(i, &World::enemy);
						Shape::Rect::Data *target_enemy_world = Actor::get_World_Coord(e, &World::enemy);
						int r = Shape::Rect::collision(enemy_world, target_enemy_world);

						if (r == 1)
						{
							Vec2D::Vec2D *enemy_vel = Actor::get_Vel(i, &World::enemy);
							Vec2D::Vec2D *target_enemy_vel = Actor::get_Vel(e, &World::enemy);
							for (int k = 0; k < 2; k++)
							{
								Collision::impulse(enemy_world, enemy_vel, 1.0, target_enemy_world, target_enemy_vel, 1.0);
							}

							Vec2D::Vec2D initial_vel = {};
							Vec2D::Vec2D f_min = { -50,-50 };
							Vec2D::Vec2D f_max = { 50,50 };
							Vec2D::Vec2D pos = { 0.5*(enemy_world->x + target_enemy_world->x + target_enemy_world->w), 0.5*(enemy_world->y + target_enemy_world->y + target_enemy_world->h) };
							Particle::spawn(&World::flail_link, 5, 0.125, &pos, &initial_vel, &f_min, &f_max, 1000, 2000, current_time);

						}
					}
				}
			}

			//ENEMY WALL COLLISION
			Grid::Region region;
			Actor::get_Grid_Collision(&region, &World::collision, i, &World::enemy);

			for (int y = region.first_row; y <= region.last_row; y++)
			{
				for (int x = region.first_col; x <= region.last_col; x++)
				{
					//there is a possible collision with wall
					if (Grid::get_Tile(x, y, &World::collision) != -1)
					{
						Shape::Rect::Data *enemy_world = Actor::get_World_Coord(i, &World::enemy);
						Shape::Rect::Data this_tile = { x,y,1.0,1.0 };
						int r = Shape::Rect::collision(enemy_world, &this_tile);

						if (r == 1)
						{
							Vec2D::Vec2D *enemy_vel = Actor::get_Vel(i, &World::enemy);
							for (int k = 0; k < 2; k++)
							{
								Collision::impulse(enemy_world, enemy_vel, 1.0, &this_tile);
							}
						}
					}
				}
			}

			Actor::update_Pos(i, &World::enemy, dt);

			Vec2D::Vec2D pos = *Actor::get_Pos(i, &World::enemy);
			Vec2D::clip(&pos, 0, World::map.n_cols - 1, 0, World::map.n_rows - 1);
			Actor::set_Pos(i, pos.x, pos.y, &World::enemy);

			Grid::imprint_Set(&World::imprint, i, Actor::get_World_Coord(i, &World::enemy));
		}

		//BULLET COLLISION
		for (int i = 0; i < World::bullet.array_size; i++)
		{
			if (Actor::is_Spawned(i, &World::bullet) == 0) continue;

			if (current_time - World::bullet.creation_time[i] > World::Parameters::bullet_life)
			{
				Actor::destroy(i, &World::bullet);
				continue;
			}

			//update vel
			Actor::update_Vel(i, &World::bullet, dt);
			Vec2D::clip(Actor::get_Vel(i, &World::bullet), -1 * World::Parameters::bullet_clip, World::Parameters::bullet_clip, -1 * World::Parameters::bullet_clip, World::Parameters::bullet_clip);

			Grid::Region region;
			Actor::get_Grid_Collision(&region, &World::collision, i, &World::bullet);

			for (int y = region.first_row; y <= region.last_row; y++)
			{
				for (int x = region.first_col; x <= region.last_col; x++)
				{
					//there is a possible collision with wall
					if (Grid::get_Tile(x, y, &World::collision) != -1)
					{
						Shape::Rect::Data *bullet_world = Actor::get_World_Coord(i, &World::bullet);
						Shape::Rect::Data this_tile = { x,y,1.0,1.0 };
						int r = Shape::Rect::collision(bullet_world, &this_tile);

						if (r == 1)
						{
							Vec2D::Vec2D *bullet_vel = Actor::get_Vel(i, &World::bullet);
							for (int k = 0; k < 2; k++)
							{
								Collision::impulse(bullet_world, bullet_vel, 1.0, &this_tile);
							}
						}
					}

					//there is a possible collision with the enemy
					if (Grid::get_Tile(x, y, &World::imprint) != -1)
					{
						int enemy_id = Grid::get_Tile(x, y, &World::imprint);
						Shape::Rect::Data *bullet_world = Actor::get_World_Coord(i, &World::bullet);
						Shape::Rect::Data *enemy_world = Actor::get_World_Coord(enemy_id, &World::enemy);
						int r = Shape::Rect::collision(bullet_world, enemy_world);
						
						if (r == 1)
						{	
							if (Actor::is_Spawned(i, &World::bullet) == 0) continue;

							Vec2D::Vec2D *bullet_vel = Actor::get_Vel(i, &World::bullet);
							Vec2D::Vec2D *enemy_vel = Actor::get_Vel(enemy_id, &World::enemy);

							//disabled impulse
							/*for (int k = 0; k < 2; k++)
							{
								Collision::impulse(bullet_world, bullet_vel, 1.0, enemy_world, enemy_vel, 1.0);
							}*/

							Vec2D::Vec2D *p_pos = Actor::get_Pos(0, &World::player);
							Vec2D::Vec2D f_min = { -1 * World::Parameters::flail_link_force_min, -1 * World::Parameters::flail_link_force_min };
							Vec2D::Vec2D f_max = { World::Parameters::flail_link_force_max, World::Parameters::flail_link_force_max };
							Vec2D::Vec2D pos = { enemy_world->x + 0.5*enemy_world->w, enemy_world->y + 0.5*enemy_world->h };

							//edited initial vel, was blank. May need to offset to make it go to center of player for aesthetics
							Vec2D::Vec2D initial_vel = { (pos.x - p_pos->x), (pos.y - p_pos->y) };
							Vec2D::norm(&initial_vel);

							//spawn particle and push to links
							int particle_id = Particle::spawn(&World::flail_link, 1, 0.4, &pos, &initial_vel, &f_min, &f_max, 5000, 5000, current_time);
							printf("spawn: %d, %d\n", particle_id, enemy_id);
							Stack::push_chain(&World::links, particle_id, enemy_id);
							///edit its creation time
							Actor::destroy(i, &World::bullet);
						}
					}
				}
			}
			Actor::update_Pos(i, &World::bullet, dt);
		}

		//UPDATE CHAIN
		if (World::flail_link.spawn_stack.n_unspawned < World::flail_link.spawn_stack.array_size)
		{
			//clear collision grid
			for (int i = 0; i < World::particle_imprint.n_cols*World::particle_imprint.n_rows; i++)
			{
				World::particle_imprint.data[i] = -1;
			}

			//fill collision grid
			for (int i = 0; i < World::flail_link.array_size; i++)
			{
				if (Particle::is_Spawned(i, &World::flail_link) == 0) continue;

				Grid::imprint_Set(&World::particle_imprint, i, Particle::get_World_Coord(i, &World::flail_link));
			}

			//update chain
			for (int i = 0; i < World::flail_link.array_size; i++)
			{
				if (Particle::is_Spawned(i, &World::flail_link) == 0) continue;

				//tempreture
				/*
				Vec2D::Vec2D random_disturbance;
				random_disturbance.x = 4.1*(1.0 - 2.0*rand() / RAND_MAX);
				random_disturbance.y = 4.1*(1.0 - 2.0*rand() / RAND_MAX);
				Particle::add_Force(i, &World::flail_link, &random_disturbance);
				*/

				Shape::Rect::Data *particle_world = Particle::get_World_Coord(i, &World::flail_link);

				//create cohesion box			
				Shape::Rect::Data cohesion_box;
				cohesion_box.w = 2.0*World::Parameters::cohesion_radius;
				cohesion_box.h = 2.0*World::Parameters::cohesion_radius;
				cohesion_box.x = particle_world->x - 0.5*cohesion_box.w;
				cohesion_box.y = particle_world->y - 0.5*cohesion_box.h;

				memset(World::neighbor_array, 0, sizeof(int)*World::flail_link.array_size);

				//fill region
				Grid::Region region;
				Grid::get_Region_Under_Shape(&region, &cohesion_box);
				Grid::clip_Grid_Region(&region, World::map.n_cols, World::map.n_rows);

				int n_neighbors = 0;
				Vec2D::Vec2D avg_pos = { 0,0 };
				Vec2D::Vec2D avg_vel = { 0,0 };

				//Seperation force
				for (int y = region.first_row; y <= region.last_row; y++)
				{
					for (int x = region.first_col; x <= region.last_col; x++)
					{
						int k = Grid::get_Tile(x, y, &World::particle_imprint);
						if (k == -1) continue;
						if (k == i) continue;

						Vec2D::Vec2D *neighbor_pos = Particle::get_Pos(k, &World::flail_link);
						float distance = sqrt((neighbor_pos->x - particle_world->x)*(neighbor_pos->x - particle_world->x) + (neighbor_pos->y - particle_world->y)*(neighbor_pos->y - particle_world->y));

						//you found an agent within your radius
						if (World::neighbor_array[k] == 0)
						{
							World::neighbor_array[k] = 1;

							Vec2D::Vec2D *neighbor_pos = Particle::get_Pos(k, &World::flail_link);
							float distance = sqrt((neighbor_pos->x - particle_world->x)*(neighbor_pos->x - particle_world->x) + (neighbor_pos->y - particle_world->y)*(neighbor_pos->y - particle_world->y));

							//seperation radius check/force
							if (distance <= World::Parameters::separation_radius)
							{
								Vec2D::Vec2D separation_force = { particle_world->x - neighbor_pos->x,particle_world->y - neighbor_pos->y };
								Vec2D::norm(&separation_force);
								Vec2D::scale(&separation_force, World::Parameters::separation_force_mag);

								Particle::add_Force(i, &World::flail_link, &separation_force);
							}

							//check cohesion distance/add up neighbors for average
							if (distance <= World::Parameters::cohesion_radius)
							{
								avg_pos.x += neighbor_pos->x;
								avg_pos.y += neighbor_pos->y;

								Vec2D::Vec2D *neighbor_vel = Particle::get_Vel(k, &World::flail_link);
								avg_vel.x += neighbor_vel->x;
								avg_vel.y += neighbor_vel->y;

								n_neighbors++;
							}
						}
					}
				}

				//Cohesion and alignment force				
				if (n_neighbors != 0)
				{
					avg_pos.x /= n_neighbors;
					avg_pos.y /= n_neighbors;
					avg_vel.x /= n_neighbors;
					avg_vel.y /= n_neighbors;


					Vec2D::Vec2D cohesion_force = { avg_pos.x - particle_world->x,avg_pos.y - particle_world->y };
					Vec2D::norm(&cohesion_force);
					Vec2D::scale(&cohesion_force, World::Parameters::cohesion_force_mag);

					Particle::add_Force(i, &World::flail_link, &cohesion_force);

					Vec2D::Vec2D alignment_force = { avg_vel.x, avg_vel.y };
					Vec2D::norm(&alignment_force);
					Vec2D::scale(&alignment_force, World::Parameters::alignment_force_mag);

					Particle::add_Force(i, &World::flail_link, &alignment_force);
				}
			
				//target force
				set_chain(Actor::get_Pos(0, &World::player), Actor::get_Pos(World::links.enemy_id, &World::enemy));
				printf("call: %d\n", World::links.enemy_id);
				
				//chain wip
				for (int i = 0; i < World::links.n_data; i++)
				{
					Vec2D::Vec2D *pos = Particle::get_Pos(World::links.data[i] , &World::flail_link);
					Vec2D::Vec2D target_force = { World::chain[i].x - pos->x, World::chain[i].y - pos->y };
					
					if (target_force.x < World::calm_chain_distance.x && target_force.y < World::calm_chain_distance.y)
					{
						target_force.x *= .5;
						target_force.y *= .5;
					}

					Vec2D::norm(&target_force);
					Vec2D::scale(&target_force, World::Parameters::target_force_mag);
					Particle::add_Force(i, &World::flail_link, &target_force);
				}
			}
		}

		Particle::update_Vel_and_Life(&World::flail_link, current_time, dt);
		Particle::update_Pos(&World::flail_link, current_time, dt);

		//VORONOI
		for (int x = 1; x < World::map.n_cols - 1; x++)
		{
			for (int y = 1; y < World::map.n_rows - 1; y++)
			{
				int closest_enemy;
				int distance = 1000;
				int distance_tmp;
				for (int i = 0; i < World::Parameters::num_enemies; i++)
				{
					Shape::Rect::Data *enemy_pos = Actor::get_World_Coord(i, &World::enemy);
					int x_dif = (x + 0.5) - World::enemy.bodies.pos[i].x + 0.5 * enemy_pos->w;
					int y_dif = (y + 0.5) - World::enemy.bodies.pos[i].y + 0.5 * enemy_pos->h;
					int distance_tmp = sqrt((x_dif * x_dif) + (y_dif * y_dif));
					if (distance_tmp < distance)
					{
						distance = distance_tmp;
						closest_enemy = i;
					}
				}
				World::map.data[y * World::map.n_cols + x] = closest_enemy + 8;
			}
		}
	}

	//SET CHAIN wip
	void set_chain(Vec2D::Vec2D *p, Vec2D::Vec2D *e)
	{
		float increment_x = (e->x - p->x) / World::Parameters::min_chain_len;
		float increment_y = (e->y - p->y) / World::Parameters::min_chain_len;
		for (int i = 0; i < World::Parameters::min_chain_len; i++)
		{
			World::chain[i].x = e->x + increment_x * i;
			World::chain[i].y = e->y + increment_y * i;
		}
	}

	void draw(unsigned int current_time)
	{
		SDL_RenderClear(Engine::renderer);

		Tileset::draw_Grid(&World::tileset, &World::camera, &World::map, &RGBA::default, Engine::renderer);

		Actor::draw(&World::player, &World::camera, current_time, Engine::renderer);
		Actor::draw(&World::enemy, &World::camera, current_time, Engine::renderer);
		Actor::draw(&World::bullet, &World::camera, current_time, Engine::renderer);

		Particle::draw(&World::flail_link, &World::camera, current_time, Engine::renderer);

		//flip buffers
		SDL_RenderPresent(Engine::renderer);
	}
}