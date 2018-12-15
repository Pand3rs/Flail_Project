#pragma once
#include <stdlib.h>
#include <assert.h>

#pragma warning(disable:4996)

namespace Stack
{
	struct Stack
	{
		int *position;
		int *data;
		int n_data;
		int size;
	};

	void resize(Stack *c)
	{
		c->size += c->size >> 1;
		c->data = (int*)realloc(c->data, sizeof(int)*c->size); assert(c->data);
	}

	void init(Stack* c, unsigned int initial_size)
	{
		assert(initial_size != 0);
		c->n_data = 0;
		c->size = initial_size;
		c->data = (int*)malloc(sizeof(int)*c->size); assert(c->data);
		c->position = (int*)malloc(sizeof(int)*c->size); assert(c->position);
	}

	void push(Stack *c, int val, int paths)
	{
		if (c->n_data >= c->size) resize(c);
		c->n_data++;
		c->data[c->n_data] = val;
		c->position[c->n_data] = paths;
	}

	int pop(Stack *c)
	{
		return c->data[--c->n_data];
	}

	void clear(Stack *c)
	{
		c->n_data = 0;
	}

	void cleanup(Stack *c)
	{
		free(c->data);
		*c = {};
	}
}