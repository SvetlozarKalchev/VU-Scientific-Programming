#ifndef COMMON_H_HEADER_GUARD
#define COMMON_H_HEADER_GUARD

#ifdef __cplusplus
#include <cstddef>
#include <cassert>
using std::size_t;
#define INLINE static inline
#else
#include <stddef.h>
#include <assert.h>
#define INLINE static
#endif

typedef int cell_type;
#define CELL_TYPE_MPI_TYPE (MPI_INT)
typedef long long result_type;
#define RESULT_TYPE_MPI_TYPE (MPI_LONG_LONG)

enum direction
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
};

INLINE cell_type evolve(cell_type center, cell_type up, cell_type down, cell_type left, cell_type right)
{
	center=(up+down+left+right)/4;
	return center;
}
INLINE cell_type initialize(size_t x, size_t y)
{
	return (cell_type)(x*12+y*1024);
}
INLINE cell_type border(enum direction dir)
{
	switch(dir)
	{
	case DIR_UP: return 10;
	case DIR_DOWN: return 100;
	case DIR_LEFT: return 1000;
	case DIR_RIGHT: return 10000;
	}
	assert(0);
	return 0;
}
INLINE result_type finalize(cell_type cell)
{
	return (result_type)cell;
}

#endif
