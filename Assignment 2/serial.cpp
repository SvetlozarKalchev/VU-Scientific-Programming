#include <iostream>
#include "common.h"
#include <vector>
#include <exception>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

extern "C" 
{
	result_type test(size_t size_x, size_t size_y, size_t iterations);
}

result_type test_mpi(size_t size_x, size_t size_y, size_t iterations,int& argc, char**& argv);

struct field
{
	typedef vector<cell_type> matrix_type;

	void initialize()
	{
		for(size_t y=0;y<y_;++y)
		{
			for(size_t x=0;x<x_;++x)
			{
				get(x,y)=::initialize(x,y);
			}
		}
	}
	void initialize(cell_type* data)
	{
		for(size_t y=0;y<y_;++y)
		{
			for(size_t x=0;x<x_;++x)
			{
				get(x,y)=data[y*x_+x];
			}
		}
	}
	field(size_t x, size_t y) : x_(x), y_(y), data(x*y)
	{
	}
	void iterate(const field& old)
	{
		for(size_t y=0;y<y_;++y)
		{
			for(size_t x=0;x<x_;++x)
			{
				get(x,y)=evolve(old.get(x,y),old.get(x,y,DIR_UP),old.get(x,y,DIR_DOWN),old.get(x,y,DIR_LEFT),old.get(x,y,DIR_RIGHT));
			}
		}
	}
	result_type finalize()
	{
		result_type res=0;
		for(matrix_type::iterator it=data.begin();it!=data.end();++it)
		{
			res+=::finalize(*it);
		}
		return res;
	}
	result_type finalize(cell_type* data_ptr)
	{
		result_type res=0;
		for(matrix_type::iterator it=data.begin();it!=data.end();++it)
		{
			res+=::finalize(*it);
			*data_ptr=*it;
			++data_ptr;
		}
		return res;
	}
	void swap(field& other)
	{
		data.swap(other.data);
		::swap(x_,other.x_);
		::swap(y_,other.y_);
	}
	cell_type get(size_t x, size_t y, direction dir) const
	{
		if (x==0 && dir==DIR_LEFT) return border(dir);
		if (y==0 && dir==DIR_UP) return border(dir);
		if (x>=x_-1 && dir==DIR_RIGHT) return border(dir);
		if (y>=y_-1 && dir==DIR_DOWN) return border(dir);
		switch(dir)
		{
		case DIR_UP: return get(x,y-1);
		case DIR_DOWN: return get(x,y+1);
		case DIR_LEFT: return get(x-1,y);
		case DIR_RIGHT: return get(x+1,y);
		}
		throw std::exception();
	}
	cell_type& get(size_t x, size_t y)
	{
		assert(x<x_ && y<y_);
		return data[y*x_+x];
	}
	cell_type get(size_t x, size_t y) const
	{
		assert(x<x_ && y<y_);
		return data[y*x_+x];
	}
private:
	size_t x_;
	size_t y_;
	matrix_type data;
};

result_type jacobi(size_t x, size_t y, cell_type* data, size_t iterations)
{
	field f1(x,y),f2(x,y);
	f1.initialize(data);
	for(size_t it=0;it<iterations;++it)
	{
		f2.iterate(f1);
		f1.swap(f2);
	}
	return f1.finalize(data);
}


void load_file(const char* path, int* size_x, int* size_y, cell_type** data)
{
	FILE* f=fopen(path,"rb");
	fread(size_x,sizeof(int),1,f);
	fread(size_y,sizeof(int),1,f);
	*data=(cell_type*)malloc((*size_x)*(*size_y)*sizeof(int));
	fread(*data,sizeof(int),(*size_x)*(*size_y),f);
	fclose(f);
}

void save_file(const char* path, int size_x, int size_y, cell_type* data)
{
	FILE* f=fopen(path,"wb");
	fwrite(&size_x,sizeof(int),1,f);
	fwrite(&size_y,sizeof(int),1,f);
	fwrite(data,sizeof(int),(size_x)*(size_y),f);
	fclose(f);
}

int main(int argc, char* argv[])
{
	if (argc==4)
	{
		int size_x, size_y;
		result_type res;
		cell_type* data;
		load_file(argv[1],&size_x,&size_y,&data);
		res=jacobi(size_x,size_y,data,atoi(argv[2]));
		save_file(argv[3],size_x,size_y,data);
		free(data);
		printf("%i\n",(int)res);
	}
}
/**/
