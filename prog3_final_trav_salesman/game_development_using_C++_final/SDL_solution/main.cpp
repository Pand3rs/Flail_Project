#include <iostream>
#include <assert.h>
#include <time.h> 
#include <cmath> 
using namespace std;

//include SDL header
#include "SDL2-2.0.8\include\SDL.h"
#include "SDL2-2.0.8\include\SDL_image.h"

//load libraries
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2main.lib")

#pragma comment(linker,"/subsystem:console")

namespace Simulation
{
	SDL_Renderer *renderer = NULL;
	int screen_width = 800;
	int screen_height = 600;

	unsigned char prev_key_state[256];
	unsigned char *keys = NULL;

	int rect_hw = 40;

	const int nodes = 12;
	float temp = 1;
	float cooling_rate = 0.98;
	int starting_index = 0;
	int ending_index = 0;
	int solution_1[nodes];
	int solution_2[nodes];
	float fitness_1 = 0;
	float fitness_2 = 0;

	struct Locations
	{
		float *x_pos = 0;
		float *y_pos = 0;
		int n_data = 0;
		int size = nodes;
	};
	Locations points;

	SDL_Rect rect[nodes];

	/*
	SDL_Texture* spriteSheet = NULL;
	SDL_Rect textureRect;
	SDL_Rect windowRect;
	*/

	void init_Locations(Locations* p, unsigned int initial_size)
	{
		assert(initial_size != 0);
		p->n_data = 0;
		p->size = initial_size;
		p->x_pos = (float*)malloc(sizeof(float)*p->size); assert(p->x_pos);
		p->y_pos = (float*)malloc(sizeof(float)*p->size); assert(p->y_pos);
	}

	float calc_fitness(int *which_solution)
	{
		float fitness = 0;

		for (int i = 0; i < nodes - 1; i++)
		{
			float diff_x = points.x_pos[which_solution[i]] + (rect_hw/2) - points.x_pos[which_solution[i+1]] + (rect_hw / 2);
			float diff_y = points.y_pos[which_solution[i]] + (rect_hw / 2) - points.y_pos[which_solution[i+1]] + (rect_hw / 2);
			fitness += sqrt(diff_x * diff_x + diff_y * diff_y);
		}
		return fitness;
	}

	void create_random_nodes()
	{
		for (int i = 0; i < nodes; i++)
		{
			points.x_pos[i] = rand() % (screen_width - rect_hw);
			points.y_pos[i] = rand() % (screen_height - rect_hw);
		}
	}

	void retrieve_nodes()
	{
		create_random_nodes();
	}

	void generate_begining_and_end()
	{
		starting_index = rand() % nodes;
		ending_index = rand() % nodes;
		while (starting_index == ending_index) ending_index = rand() % nodes;
	}

	void generate_initial_solution()
	{
		for (int i = 0; i < nodes; i++)
		{
			solution_1[i] = i;
		}

		for (int i = nodes - 1; i > 0; i--)
		{
			int j = rand() % (i);
			if (j == i) continue;

			int tmp = solution_1[i];
			solution_1[i] = solution_1[j];
			solution_1[j] = tmp;
		}

		for (int i = 1; i < nodes; i++)
		{
			if (solution_1[i] == starting_index)
			{
				int tmp = solution_1[0];
				solution_1[0] = starting_index;
				solution_1[i] = tmp;
			}

			if (solution_1[i] == ending_index)
			{
				int tmp = solution_1[nodes - 1];
				solution_1[nodes - 1] = ending_index;
				solution_1[i] = tmp;
			}
		}
	}

	void modify_solution_2()
	{
		int a = 1 + (rand() % (nodes - 2));
		int b = 1 + (rand() % (nodes - 2));

		while (a == b)
		{
			b = 1 + (rand() % (nodes - 2));
		}

		int tmp = solution_2[a];
		solution_2[a] = solution_2[b];
		solution_2[b] = tmp;
	}

	void generate_everything()
	{
		retrieve_nodes();
		generate_begining_and_end();
		generate_initial_solution();
		temp = 1.0;
	}

	void init()
	{
		SDL_Init(SDL_INIT_VIDEO);

		prev_key_state[256];
		keys = (unsigned char*)SDL_GetKeyboardState(NULL);

		SDL_Window *window = SDL_CreateWindow("Pizza Time", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		init_Locations(&points, nodes);

		generate_everything();

		/*
		SDL_Surface* temp = IMG_Load("saitama_pink.png");
		spriteSheet = SDL_CreateTextureFromSurface(renderer, temp);
		SDL_FreeSurface(temp);

		//'windowRect' defines the dimensions of the rendering sprite on window
		windowRect.x = 0;
		windowRect.y = 0;
		windowRect.w = 140; //wrong numbers
		windowRect.h = 200; //wrong numbers

		//'textureRect' defines the dimensions of the rendering sprite on texture
		textureRect.x = 0;
		textureRect.y = 0;

		//SDL_QueryTexture() gets the width and height of the texture
		SDL_QueryTexture(spriteSheet, NULL, NULL, &textureRect.w, &textureRect.h);

		textureRect.w /= 3;
		*/
	}

	void update()
	{
		//copy previous key state
		memcpy(prev_key_state, keys, 256);

		//consume all window events, key state gets updated here automatically
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
		}

		fitness_1 = calc_fitness(solution_1);
		//printf("fitness_1: %f\n", fitness_1);

		for (int i = 0; i < nodes; i++)
		{
			solution_2[i] = solution_1[i];
		}

		modify_solution_2();

		fitness_2 = calc_fitness(solution_2);
		//printf("fitness_2: %f\n", fitness_2);

		float fitness_diff = fitness_1 - fitness_2;

		if (fitness_2 < fitness_1)
		{
			//printf("fitness_2 was better\n\n");
			for (int i = 0; i < nodes; i++)
			{
				solution_1[i] = solution_2[i];
			}
		}
		else
		{
			double p = (double)rand() / RAND_MAX;
			if (exp(fitness_diff / temp) > 0)
			{
				printf("p is: %f fit_diff is : %f temp is: %f  exp of them is: %f\n\n", p, fitness_diff, temp, exp(fitness_diff / temp));
			}
			if (p < exp(fitness_diff / temp))
			{
				printf("temp selected fitness_2\n\n");
				for (int i = 0; i < nodes; i++)
				{
					solution_1[i] = solution_2[i];
				}
			}
			else
			{
				//printf("stuck with fitness_1\n\n");
			}
		}

		temp *= cooling_rate;
		//printf("ftemp is: %f\n", temp);
		/*
		if (temp < 0.005)
		{
			SDL_Rect source_rect = { 0, 0, 32, 64 };
			SDL_Rect destination_rect = { 10, 10, 32, 64 };

			int totalFrames = 3;

			int delayPerFrame = 100;

			SDL_GetTicks()

			int frame = (SDL_GetTicks() / delayPerFrame) % totalFrames;

			textureRect.x = frame * textureRect.w;

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			SDL_RenderCopy(renderer, spriteSheet, &textureRect, &windowRect);

			SDL_RenderPresent(renderer);
		}
		*/

		if (temp < 0.0001)
		{
			printf("found route\n\n Press any key to restart\n\n");
			//getchar();
			generate_everything();
		}
	}

	void draw()
	{
		//set color to white
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		//clear screen with white
		SDL_RenderClear(renderer);

		for (int i = 0; i < nodes; i++)
		{
			rect[i].x = points.x_pos[solution_1[i]];
			rect[i].y = points.y_pos[solution_1[i]];
			rect[i].w = rect_hw;
			rect[i].h = rect_hw;

			SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

			if (i == 0)
			{
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
			}

			if (i == nodes - 1)
			{
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			}

			SDL_RenderFillRect(renderer, &rect[i]);
		}

		for (int i = 1; i < nodes; i++)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderDrawLine(renderer, 
				points.x_pos[solution_1[i - 1]] + (rect_hw / 2), 
				points.y_pos[solution_1[i - 1]] + (rect_hw / 2), 
				points.x_pos[solution_1[i]] + (rect_hw / 2), 
				points.y_pos[solution_1[i]] + (rect_hw / 2));
		}

		SDL_RenderPresent(renderer);
	}
}

int main(int argc, char **argv)
{
	srand(time(0));
	Simulation::init();

	unsigned int t1 = SDL_GetTicks();
	unsigned int t2;

	for (;;)
	{
		t2 = SDL_GetTicks();
		if (t2 - t1 > 50)
		{
			Simulation::update();

			Simulation::draw();
			t1 = t2;
		}
	}
	return 0;
}
