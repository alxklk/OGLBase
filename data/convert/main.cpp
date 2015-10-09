#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <map>

using namespace std;

typedef struct s_float3
{
	float x;
	float y;
	float z;
	void Set(const float& ix, const float& iy, const float& iz)
	{
		x=ix;
		y=iy;
		z=iz;
	}
}float3;

typedef struct s_float2
{
	float x;
	float y;
	void Set(const float& ix, const float& iy)
	{
		x=ix;
		y=iy;
	}
}float2;

typedef struct s_vertex_PNT
{
	float3 position;
	float3 normal;
	float2 texture;
}vertex_PNT;

typedef struct s_index_PNT
{
	int pindex;
	int nindex;
	int tindex;
	s_index_PNT(int pidx, int nidx, int tidx): pindex(pidx), nindex(nidx), tindex(tidx){}
}index_PNT;

bool operator<(const s_index_PNT& left, const s_index_PNT& right)
{
	static const double M_PNT1=1024.0*16.0;
	static const double M_PNT2=M_PNT1*M_PNT1;
	double vl=M_PNT2*left .pindex+M_PNT1*left .nindex+left .tindex;
	double vr=M_PNT2*right.pindex+M_PNT1*right.nindex+right.tindex;
	return (vl<vr);
}

vector<float3>v;
vector<float3>n;
vector<float2>t;
vector<vertex_PNT>vbuf;
vector<unsigned int>ibuf;
vector<unsigned int>gbuf;
typedef map<index_PNT, int>Tvmap;
Tvmap vmap;

void AddIndex(int vX, int nX, int tX)
{
	Tvmap::iterator found=vmap.find(index_PNT(vX, nX, tX));
	if(found==vmap.end())
	{
		int index=vbuf.size();
		vmap[index_PNT(vX, nX, tX)]=index;
		ibuf.push_back(index);
		vbuf.resize(vbuf.size()+1);
		vbuf.back().normal  =n[nX];
		vbuf.back().position=v[vX];
		vbuf.back().texture =t[tX];
	}
	else
	{
		int index=found->second;
		ibuf.push_back(index);
	}
}


int main(int argc, char **argv)
{
	printf("hello world\n");
	char wd[256];
	getcwd(wd, 256);
	printf("%s\n",wd);
	const char* fname="cat.obj";
//	fname=argv[1];
	FILE* f=fopen(fname,"rb");
	if(f)
	{
		char string[256];
		while(true)
		{
			char* res=fgets(string,255,f);
			if(!res)
				break;
			if((string[0]=='v')&&(string[1]==' '))
			{
				float x;
				float y;
				float z;
				int nf=sscanf(string+2, "%f %f %f", &x, &y, &z);
				if(nf==3)
				{
					v.resize(v.size()+1);
					v.back().Set(x,y,z);
				}
			}
			if((string[0]=='g')&&(string[1]==' '))
			{
				if(ibuf.size()%3)
					printf("wrong index count");
				gbuf.push_back(ibuf.size()/3);
			}
			if((string[0]=='v')&&(string[1]=='n'))
			{
				float x;
				float y;
				float z;
				int nf=sscanf(string+2, "%f %f %f", &x, &y, &z);
				if(nf==3)
				{
					n.resize(n.size()+1);
					n.back().Set(x,y,z);
				}
			}
			if((string[0]=='v')&&(string[1]=='t'))
			{
				float x;
				float y;
				int nf=sscanf(string+2, "%f %f", &x, &y);
				if(nf==2)
				{
					t.resize(t.size()+1);
					t.back().Set(x,y);
				}
			}
			if((string[0]=='f')&&(string[1]==' '))
			{
				int v0, v1, v2;
				int n0, n1, n2;
				int t0, t1, t2;
				int nf=sscanf(string+2, "%i/%i/%i %i/%i/%i %i/%i/%i", 
				&v0, &n0, &t0, 
				&v1, &n1, &t1, 
				&v2, &n2, &t2 
				);
				if(nf==9)
				{
					AddIndex(v0-1,n0-1,t0-1);
					AddIndex(v1-1,n1-1,t1-1);
					AddIndex(v2-1,n2-1,t2-1);
				}
			}
		}
		printf("Positions %i Normals %i Texcoords %i Vertices %i Faces %i\n", 
			(int)v.size(), (int)n.size(), (int)t.size(), (int)vbuf.size(), (int)ibuf.size());
		fclose(f);
		
		{
			FILE* f=fopen("model.h", "wb");
			fprintf(f, "int gb[]={");
			for(unsigned int i=0;i<gbuf.size();i++)
			{
				fprintf(f, "%i, ",gbuf[i]);
			}
			fseek(f,-2,SEEK_CUR);
			fprintf(f, "};\n");
			fprintf(f, "float vb[]={\n");
			fprintf(f, "\t//      p.x,         p.y,         p.z,         n.x,         n.y,         n.z,         t.x,         t.y\n");
			for(unsigned int i=0;i<vbuf.size();i++)
			{
				fprintf(f, "\t% 11.6f, % 11.6f, % 11.6f, % 11.6f, % 11.6f, % 11.6f, % 11.6f, % 11.6f,//% 7i\n",
					vbuf[i].position.x,vbuf[i].position.y,vbuf[i].position.z,
					vbuf[i].normal.x  ,vbuf[i].normal.y  ,vbuf[i].normal.z,
					vbuf[i].texture.x ,vbuf[i].texture.y,
					i
				);
			}
			fprintf(f, "};\n");
			fprintf(f, "int ib[]={\n\t");
			for(unsigned int i=0;i<ibuf.size();i++)
			{
				if(i&&(i%3==0))
					fprintf(f, " // % 3i\n\t",i/3);
				fprintf(f, "% 6i,", ibuf[i]);
			}
			fprintf(f, "};\n");
			fclose(f);
		}
	}
	return 0;
}
