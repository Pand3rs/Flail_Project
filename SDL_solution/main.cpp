#include <iostream>
#include <assert.h>
#include <time.h> 
using namespace std;

//include SDL header
#include "SDL2-2.0.8\include\SDL.h"

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

	const int nodes = 10;
	float temp;
	float cooling_rate = 0.95;
	int starting_index;
	int ending_index;
	int solution_1[nodes];
	int solution_2[nodes];
	float fitness_1;
	float fitness_2;

	struct Stack
	{
		float *x_pos;
		float *y_pos;
		int n_data;
		int size;
	};
	Stack points;

	SDL_Rect rect[nodes];

	void init_stack(Stack* p, unsigned int initial_size)
	{
		printf("stack init\n");
		assert(initial_size != 0);
		p->n_data = 0;
		p->size = initial_size;
		p->x_pos = (float*)malloc(sizeof(float)*p->size); assert(p->x_pos);
		p->y_pos = (float*)malloc(sizeof(float)*p->size); assert(p->y_pos);
	}

	float calc_fitness(int *a)
	{
		float fitness = 0;

		for (int i = 0; i < nodes - 1; i++)
		{
			float diff_x = points.x_pos[a[i]] + (rect_hw / 2) - points.x_pos[a[i + 1]] + (rect_hw / 2);
			float diff_y = points.y_pos[a[i]] + (rect_hw / 2) - points.y_pos[a[i + 1]] + (rect_hw / 2);
			fitness += sqrt(diff_x * diff_x + diff_y * diff_y);
		}
		return fitness;
	}

	void create_random_nodes()
	{
		printf("create_random_nodes (x,y)\n\n");
		for (int i = 0; i < nodes; i++)
		{
			points.x_pos[i] = rand() % (screen_width - 40);
			points.y_pos[i] = rand() % (screen_height - 40);
			printf(" index:%d  x:%f  y:%f\n", i, points.x_pos[i], points.y_pos[i]);
		}
	}

	void retrieve_nodes()
	{
		create_random_nodes();
	}

	void generate_BandE()
	{
		printf("generate_BandE\n");
		starting_index = rand() % nodes;
		ending_index = rand() % nodes;
		while (starting_index == ending_index) ending_index = rand() % nodes;
		printf("begin:%d  end:%d\n\n", starting_index, ending_index);
	}

	void generate_solution()
	{
		printf("generate_solution\n");
		for (int i = 0; i < nodes; i++)
		{
			solution_1[i] = i;
		}

		for (int i = nodes - 1; i > 0; i--)//changed check from 0 to 1 /////////WATCH THIIS//////////
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
		printf("starting solution: ");
		for (int i = 0; i < nodes; i++)
		{
			printf("%d, ", solution_1[i]);
		}
		printf("\n");
	}

	void modify_solution()
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
		printf("generate_everything\n");
		retrieve_nodes();
		generate_BandE();
		generate_solution();
		temp = 1.0;
	}

	void init()
	{
		SDL_Init(SDL_INIT_VIDEO);

		prev_key_state[256];
		keys = (unsigned char*)SDL_GetKeyboardState(NULL);

		SDL_Window *window = SDL_CreateWindow("Not Fortnite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		init_stack(&points, nodes);

		generate_everything();
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

		//calc fitness for first solution
		fitness_1 = calc_fitness(solution_1);
		printf("fitness_1: %f\n", fitness_1);

		//make a copy of solution 1
		for (int i = 0; i < nodes; i++)
		{
			solution_2[i] = solution_1[i];
		}

		//modify solution 2
		modify_solution();

		//check solution 2 fitness
		fitness_2 = calc_fitness(solution_2);
		printf("fitness_2: %f\n", fitness_2);

		float fitness_diff = fitness_1 - fitness_2;   /////if result is 0, bad juju
		double p;

		//compare solutions
		if (fitness_2 < fitness_1)
		{
			printf("fitness_2 was better\n\n");
			for (int i = 0; i < nodes; i++)
			{
				solution_1[i] = solution_2[i];
			}
		}
		else
		{
			p = (double)rand() / RAND_MAX;
			//chance to take worse result
			if (p < exp(fitness_diff / temp))
			{
				printf("temp said fitness_2 was better\n\n");
				for (int i = 0; i < nodes; i++)
				{
					solution_1[i] = solution_2[i];
				}
			}
			else
			{
				printf("stuck with fitness_1\n\n");
			}
		}

		temp *= cooling_rate;

		if (temp < 0.0001)
		{
			printf("found route\n");
			getchar();
			generate_everything();
		}
	}

	void draw()
	{
		printf("draw\n\n");
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