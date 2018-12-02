#pragma once
#include <stdlib.h>
#include <assert.h>

#pragma warning(disable:4996)

namespace Stack
{
	struct Stack
	{
		int *data;
		int n_data;
		int enemy_id;
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
		c->enemy_id = 0;
		c->data = (int*)malloc(sizeof(int)*c->size); assert(c->data);
	}

	void push(Stack *c, int val)
	{
		if (c->n_data >= c->size) resize(c);
		c->data[c->n_data++] = val;
	}

	void push_chain(Stack *c, int val, int enemy_id)
	{
		if (c->n_data >= c->size) resize(c);
		c->data[c->n_data++] = val;
		c->enemy_id = enemy_id;
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