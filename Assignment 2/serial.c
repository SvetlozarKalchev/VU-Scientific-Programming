#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CELL(A,X,Y,MX,MY) (*((A)+(Y)*(MX)+(X)))

cell_type get(cell_type* data, int x, int y, int end_x, int end_y)
{
	if (x<0) return border(DIR_LEFT);
	if (y<0) return border(DIR_UP);
	if (x>=end_x) return border(DIR_RIGHT);
	if (y>=end_y) return border(DIR_DOWN);
	return CELL(data,x,y,end_x,end_y);
}

result_type test(int size_x, int size_y, int iterations)
{
	cell_type *this_data=malloc(size_y*size_x*sizeof(cell_type));
	cell_type *next_data=malloc(size_y*size_x*sizeof(cell_type));
	cell_type *tmp;
	int x,y;
	int it;
	result_type res=0;
	for(x=0;x<size_x;++x)
	{
		for(y=0;y<size_y;++y)
		{
			CELL(this_data,x,y,size_x,size_y)=initialize(x,y);
		}
	}
	for(it=0;it<iterations;++it)
	{
		for(x=0;x<size_x;++x)
		{
			for(y=0;y<size_y;++y)
			{
				CELL(next_data,x,y,size_x,size_y)=evolve(CELL(this_data,x,y,size_x,size_y),get(this_data,x,y-1,size_x,size_y),get(this_data,x,y+1,size_x,size_y),get(this_data,x-1,y,size_x,size_y),get(this_data,x+1,y,size_x,size_y));
			}
		}
		tmp=this_data;
		this_data=next_data;
		next_data=tmp;
	}
	for(x=0;x<size_x;++x)
	{
		for(y=0;y<size_y;++y)
		{
			res+=finalize(CELL(this_data,x,y,size_x,size_y));
		}
	}
	free(this_data);
	free(next_data);
	return res;
}

result_type jacobi(int size_x, int size_y, cell_type** data, int iterations)
{
	cell_type *this_data=*data;
	cell_type *next_data=malloc(size_y*size_x*sizeof(cell_type));
	cell_type *tmp;
	int x,y;
	int it;
	result_type res=0;
	for(x=0;x<size_x;++x)
	{
		for(y=0;y<size_y;++y)
		{
			CELL(this_data,x,y,size_x,size_y)=initialize(x,y);
		}
	}
	for(it=0;it<iterations;++it)
	{
		for(x=0;x<size_x;++x)
		{
			for(y=0;y<size_y;++y)
			{
				CELL(next_data,x,y,size_x,size_y)=evolve(CELL(this_data,x,y,size_x,size_y),get(this_data,x,y-1,size_x,size_y),get(this_data,x,y+1,size_x,size_y),get(this_data,x-1,y,size_x,size_y),get(this_data,x+1,y,size_x,size_y));
			}
		}
		tmp=this_data;
		this_data=next_data;
		next_data=tmp;
	}
	for(x=0;x<size_x;++x)
	{
		for(y=0;y<size_y;++y)
		{
			res+=finalize(CELL(this_data,x,y,size_x,size_y));
		}
	}
	*data=this_data;
	free(next_data);
	return res;
}

void generate_file(int size_x, int size_y, const char* path)
{
	int x,y;
	FILE* f=fopen(path,"wb");
	fwrite(&size_x,sizeof(int),1,f);
	fwrite(&size_y,sizeof(int),1,f);
	for(y=0;y<size_y;++y)
	{
		for(x=0;x<size_x;++x)
		{
			int value;
			value=initialize(x,y);
			fwrite(&value,sizeof(int),1,f);
		}
	}
	fclose(f);
}

void load_file(const char* path, int* size_x, int* size_y, cell_type** data)
{
	FILE* f=fopen(path,"rb");
	fread(size_x,sizeof(int),1,f);
	fread(size_y,sizeof(int),1,f);
	*data=malloc((*size_x)*(*size_y)*sizeof(int));
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
	if (argc==5 && strcmp(argv[1],"-g")==0) generate_file(atoi(argv[2]),atoi(argv[3]),argv[4]);
	if (argc==4)
	{
		int size_x, size_y;
		result_type res;
		cell_type* data;
		load_file(argv[1],&size_x,&size_y,&data);
		res=jacobi(size_x,size_y,&data,atoi(argv[2]));
		save_file(argv[3],size_x,size_y,data);
		free(data);
		printf("%i\n",(int)res);
	}
}
/**/
