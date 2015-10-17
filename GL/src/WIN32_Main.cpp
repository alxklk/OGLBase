#define _WIN32_WINNT 0x0501

char const* AppName="GL Test";

#include <Windows.h>
#include <WinGDI.h>

#include <gl/GL.h>
#include <stdio.h>
#include <math.h>

#include "glext.h"
//#include "wglext.h" // 
#include "FIMM.h"
#include "picopng.h"
#include "matrix.h"

PFNGLCREATESHADERPROC                  glCreateShader;
PFNGLSHADERSOURCEPROC                  glShaderSource;
PFNGLCOMPILESHADERPROC                 glCompileShader;
PFNGLGETSHADERIVPROC                   glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC              glGetShaderInfoLog;
PFNGLCREATEPROGRAMPROC                 glCreateProgram;
PFNGLATTACHSHADERPROC                  glAttachShader;
PFNGLLINKPROGRAMPROC                   glLinkProgram;
PFNGLUSEPROGRAMPROC                    glUseProgram;
PFNGLGENBUFFERSPROC                    glGenBuffers;
PFNGLDELETEBUFFERSPROC                 glDeleteBuffers;
PFNGLBINDBUFFERPROC                    glBindBuffer;
PFNGLBUFFERDATAPROC                    glBufferData;
PFNGLMAPBUFFERPROC                     glMapBuffer;
PFNGLUNMAPBUFFERPROC                   glUnmapBuffer;
PFNGLVERTEXATTRIBPOINTERPROC           glVertexAttribPointer;
PFNGLGETATTRIBLOCATIONPROC             glGetAttribLocation;
PFNGLENABLEVERTEXATTRIBARRAYPROC       glEnableVertexAttribArray;
PFNGLACTIVETEXTUREPROC                 glActiveTexture;
PFNGLGETUNIFORMLOCATIONPROC            glGetUniformLocation;
PFNGLPROGRAMUNIFORM1IPROC              glProgramUniform1i;
PFNGLPROGRAMUNIFORM3FPROC              glProgramUniform3f;
PFNGLPROGRAMUNIFORM4FPROC              glProgramUniform4f;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC       glProgramUniformMatrix4fv;

class CTimer
{
	double time;
	__int64 itime;
public:
	CTimer():
		time(0)
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&itime);
	}
	double GetTime()
	{
		__int64 inewtime;
		__int64 ifreq;
		::QueryPerformanceCounter((LARGE_INTEGER*)&inewtime);
		::QueryPerformanceFrequency((LARGE_INTEGER*)&ifreq);
		time+=(inewtime-itime)/(double)ifreq;
		itime=inewtime;
		return time;
	}
};


struct SAppState
{
public:
	bool quitRequested;
	bool reloadShaders;
	int frameStamp;

	SAppState() :
		quitRequested(false),
		reloadShaders(true),
		frameStamp(0)
	{
	}
}gAppState;

void LOG(const char* logtext, int level=0)
{
	OutputDebugString(logtext);
	puts(logtext);
}

template <typename T> static bool wglProc(T*& funcp, const char* name)
{
	{
		void* p=(void*)wglGetProcAddress(name);
		if(p)
		{
			funcp=(T*)p;
			return true;
		}
		LOG("Unknown function ");
		LOG(name);
		LOG("\n");
		return false;
	}
};

#define WGLPROC(p) wglProc(p, #p)

class GLShader
{
	int vsh;
	int fsh;
	int p;
public:
	bool Create(const char* vstext, const char* fstext, int vslen=0, int fslen=0)
	{
		vsh=glCreateShader(GL_VERTEX_SHADER);
		fsh=glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vsh, 1, &vstext, &vslen);
		glShaderSource(fsh, 1, &fstext, &fslen);

		GLint ok;
		const int errbufsize=4096;
		char errorbuf[errbufsize];
		int errorlen=0;

		glCompileShader(vsh);
		glGetShaderiv(vsh, GL_COMPILE_STATUS, &ok);
		if(!ok)
		{
			LOG("\nError in vertex shader\n");
			glGetShaderInfoLog(vsh, errbufsize, &errorlen, errorbuf);
			LOG(errorbuf);
			if(errorlen>errbufsize)
				LOG("... \nError text clipped\n");
			return false;
		}

		glCompileShader(fsh);
		glGetShaderiv(fsh, GL_COMPILE_STATUS, &ok);
		if(!ok)
		{
			LOG("\nError in fragment shader\n");
			glGetShaderInfoLog(fsh, errbufsize, &errorlen, errorbuf);
			LOG(errorbuf);
			if(errorlen>errbufsize)
				LOG("... \nError text clipped\n");
			return false;
		}
		p=glCreateProgram();
		glAttachShader(p, vsh);
		glAttachShader(p, fsh);
		glLinkProgram(p);
		return true;
	}
	bool CreateFromFile(const char* vsfilename, const char* fsfilename)
	{
		CFIMM vsfi;
		CFIMM fsfi;

		vsfi.Open(vsfilename);
		fsfi.Open(fsfilename);

		if(vsfi.Addr()&&vsfi.GetSize())
		{
			return Create(vsfi.CAddr(), fsfi.CAddr(), vsfi.GetSize(), fsfi.GetSize());
		}
		LOG("\nShader source file - ?\n");
		return false;
	}
	bool Use()
	{
		glUseProgram(p);
		return true;
	}
	int GetPName()const { return p; }
};


class CUIGL
{
	HWND wnd;
	HDC dc;
	HGLRC glrc;

	int framen;

public:
	virtual int Init(int w, int h, int sx, int sy, HWND iwnd)
	{
		framen=0;
		wnd=iwnd;

		dc=GetDC(wnd);
		PIXELFORMATDESCRIPTOR pfd={0, 0, PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // wrong way ;)
		SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd);
		glrc=wglCreateContext(dc);
		wglMakeCurrent(dc, glrc);

		return 0;
	}

	virtual int FrameBegin()
	{
		return 0;
	}

	virtual int FrameEnd()
	{
		framen++;
		SwapBuffers(dc);
		return 0;
	}

	virtual int ShutDown()
	{
		FrameEnd();
		wglDeleteContext(glrc);
		wglMakeCurrent(dc, 0);
		ReleaseDC(wnd, dc);
		return 0;
	}

};


HWND wnd; void stop(HWND hwnd, unsigned int imsg, WPARAM wpar, LPARAM lpar)
{
	PostQuitMessage(0);
}

union XYLPARAM
{
	unsigned int l;
	struct
	{
		short int x;
		short int y;
	};
	XYLPARAM(LPARAM I) :l((unsigned int)I) {};
};

int mx;
int my;
int mb;

float mpx=0;
float mpy=0;
float dz=3.0;

LRESULT CALLBACK wp(HWND hwnd, unsigned int imsg, WPARAM wpar, LPARAM lpar)
{
	if(imsg==WM_DESTROY)
	{
		stop(hwnd, imsg, wpar, lpar);
	}
	else if(imsg==WM_MOUSEMOVE)
	{
		static int prevx=mx;
		static int prevy=my;

		mx=XYLPARAM(lpar).x;
		my=XYLPARAM(lpar).y;
		mb=(0!=(wpar&MK_LBUTTON));
		if(mb)
		{
			mpx+=0.01*(mx-prevx);
			mpy-=0.01*(my-prevy);
		}
		prevx=mx;
		prevy=my;
	}
	else if(imsg==WM_MOUSEWHEEL)
	{
		float dw=GET_WHEEL_DELTA_WPARAM(wpar)/WHEEL_DELTA;
		dz*=pow(0.95f,dw);
	}
	else if(imsg==WM_KEYDOWN)
	{
		if(wpar==VK_F5)
		{
			LOG("\nReload shaders\n");
			gAppState.reloadShaders=true;
		}
		else if(wpar==VK_ESCAPE)
		{
			gAppState.quitRequested=true;
		}
		else if(wpar=='A')
		{
			dz-=0.1;
			if(dz<1.0)
				dz=1.0;
		}
		else if(wpar=='Z')
		{
			dz+=0.1;

		}
	}

	return DefWindowProc(hwnd, imsg, wpar, lpar);
}

int W=1600;
int H=1000;

#include "cat_ib.h"
#include "cat_vb.h"

const int vbsize=sizeof(vb);
const int ibsize=sizeof(ib);
const int vertxcount=vbsize/sizeof(vb[0]);
const int indexcount=ibsize/sizeof(ib[0]);

int loadPNG(char* filename, unsigned char** dest, unsigned long* w, unsigned long* h)
{
	CFIMM F;
	F.Open(filename);
	if(!F.Addr())
		return 0;
	int fsize=F.GetSize();
	ppng::varray<unsigned char> out;
	if(decodePNG(out, *w, *h, (const unsigned char*)F.CAddr(), F.GetSize()))
		return 0;
	if(*w&&*h)
		*dest=new unsigned char[*w**h*4];
	for(unsigned int i=0; i<*h; i++)
	{
		int ni=*h-i-1;
		for(unsigned int j=0; j<*w; j++)
		{
			(*dest)[i*4**w+j*4+0]=out[ni*4**w+j*4+2];
			(*dest)[i*4**w+j*4+1]=out[ni*4**w+j*4+1];
			(*dest)[i*4**w+j*4+2]=out[ni*4**w+j*4+0];
			(*dest)[i*4**w+j*4+3]=out[ni*4**w+j*4+3];
		}
	}
	return 1;
}

unsigned int LoadTexture(char* filename)
{
	unsigned char* bytes;
	unsigned long width;
	unsigned long height;
	if(!loadPNG(filename, &bytes, &width, &height))
		return 0;
	glEnable(GL_TEXTURE_2D);

	GLuint tex=-1;
	glGenTextures(1, &tex);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bytes);
	delete[] bytes;
	return tex;
}

int __cdecl main(int argc, char* argv[])
{
	WNDCLASSEX wc={sizeof(WNDCLASSEX), 0, wp, 0, 0, 0, 0, 0, 0, 0, "GLAP301"};
	wc.hCursor=LoadCursor(0, IDC_ARROW);;
	wc.hIcon=LoadIcon(0,IDI_APPLICATION);
	RegisterClassEx(&wc);

	RECT wrect={0, 0, W, H};
	AdjustWindowRectEx(&wrect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);
	HWND win=CreateWindowEx(WS_EX_APPWINDOW, "GLAP301", "Render Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		wrect.right-wrect.left, wrect.bottom-wrect.top, 0, 0, 0, 0);

	ShowWindow(win, SW_SHOWNORMAL);
	UpdateWindow(win);

	MSG msg={0, 0, 0, 0, 0, {0, 0}};

	CTimer timer;

	CUIGL GL;
	GL.Init(W, H, 1, 1, win);

	WGLPROC(glCreateShader);
	WGLPROC(glShaderSource);
	WGLPROC(glCompileShader);
	WGLPROC(glGetShaderiv);
	WGLPROC(glGetShaderInfoLog);
	WGLPROC(glCreateProgram);
	WGLPROC(glAttachShader);
	WGLPROC(glLinkProgram);
	WGLPROC(glUseProgram);
	WGLPROC(glGenBuffers);
	WGLPROC(glDeleteBuffers);
	WGLPROC(glBindBuffer);
	WGLPROC(glBufferData);
	WGLPROC(glMapBuffer);
	WGLPROC(glUnmapBuffer);
	WGLPROC(glVertexAttribPointer);
	WGLPROC(glGetAttribLocation);
	WGLPROC(glEnableVertexAttribArray);
	WGLPROC(glActiveTexture);
	WGLPROC(glGetUniformLocation);
	WGLPROC(glProgramUniform1i);
	WGLPROC(glProgramUniform3f);
	WGLPROC(glProgramUniform4f);
	WGLPROC(glProgramUniformMatrix4fv);

	GLuint ibo=-1;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibsize, ib, GL_STATIC_DRAW);

	GLuint vbo=-1;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vbsize, vb, GL_STATIC_DRAW);

	glEnableClientState(GL_VERTEX_ARRAY);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glEnable(GL_DEPTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	GLuint body_tex=LoadTexture("data\\body_diff.png");
	GLuint body_norm=LoadTexture("data\\body_norm.png");
	GLuint fur_tex=LoadTexture("data\\fur_diff.png");
	GLuint fur_norm=LoadTexture("data\\fur_norm.png");

	GLShader sh;

	glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
	glClearDepth(1.0f);

	while(1)
	{
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if(msg.message==WM_QUIT)
				break;
		}

		if(gAppState.reloadShaders)
		{
			sh.CreateFromFile("data\\model.vs", "data\\model.fs");
			gAppState.reloadShaders=false;

			glVertexAttribPointer(glGetAttribLocation(sh.GetPName(), "i_pos"), 3, GL_FLOAT, GL_FALSE, 4*16, (void*)(0*4));
			glVertexAttribPointer(glGetAttribLocation(sh.GetPName(), "i_nor"), 3, GL_FLOAT, GL_FALSE, 4*16, (void*)(3*4));
			glVertexAttribPointer(glGetAttribLocation(sh.GetPName(), "i_tan"), 3, GL_FLOAT, GL_FALSE, 4*16, (void*)(10*4));
			glVertexAttribPointer(glGetAttribLocation(sh.GetPName(), "i_bin"), 3, GL_FLOAT, GL_FALSE, 4*16, (void*)(13*4));
			glVertexAttribPointer(glGetAttribLocation(sh.GetPName(), "i_tex"), 2, GL_FLOAT, GL_FALSE, 4*16, (void*)(6*4));
		}

		glProgramUniform1i(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"sTex"), 0);
		glProgramUniform1i(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"sNor"), 1);

		float4x4 wm;
		wm=float4x4::Scale(0.05)
			*float4x4::RotY(-sin(mpx), cos(mpx))
			*float4x4::RotX(-1, 0)
			*float4x4::RotX(-sin(mpy*0.3), cos(mpy*0.3))
			;

		float fov=1.3;
		float camrx=(mx-W/2.0f)/5000.0;
		float camry=(my-H/2.0f)/5000.0;
		float4x4 vpm=float4x4::Projection(0.25, 2000, fov, H/(float)W*fov).Transposed()
			*float4x4::Translation(0, 0, dz)
			*float4x4::RotY(0, -1)
			*float4x4::RotX(sin(camry), cos(camry))
			*float4x4::RotY(-sin(camrx), cos(camrx))
			;

		
		glProgramUniformMatrix4fv(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"u_wm" ), 1, false, &wm.m00);
		glProgramUniformMatrix4fv(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"u_vpm"), 1, false, &vpm.m00);

		double time=timer.GetTime();

		float lightPos0[3]={sin(time),0.5, cos(time)};
		float lightPos1[3]={sin(-time*0.5),-0.5, cos(time*0.5)};;
		float lightPos2[3]={camrx*70, camry*70, -3};

		float lightCol0[3]={1.0,0.7,0.3};
		float lightCol1[3]={0.3,0.7,1.0};
		float lightCol2[3]={1.0,1.0,1.0};

		glProgramUniform3f(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"lightPos0"), lightPos0[0], lightPos0[1], lightPos0[2]);
		glProgramUniform3f(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"lightPos1"), lightPos1[0], lightPos1[1], lightPos1[2]);
		glProgramUniform3f(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"lightPos2"), lightPos2[0], lightPos2[1], lightPos2[2]);

		glProgramUniform3f(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"lightCol0"), lightCol0[0], lightCol0[1], lightCol0[2]);
		glProgramUniform3f(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"lightCol1"), lightCol1[0], lightCol1[1], lightCol1[2]);
		glProgramUniform3f(sh.GetPName(), glGetUniformLocation(sh.GetPName(),"lightCol2"), lightCol2[0], lightCol2[1], lightCol2[2]);


		glDepthMask(true);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		sh.Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, body_tex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, body_norm);

		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		glDrawElements(GL_TRIANGLES, (4340*3), GL_UNSIGNED_INT, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fur_tex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fur_norm);

		glDepthMask(false);
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);

		glDrawElements(GL_TRIANGLES, indexcount-(4340*3), GL_UNSIGNED_INT, (void*)(4340*3*4));

		GL.FrameEnd();
		if(msg.message==WM_QUIT)
			break;
		if(gAppState.quitRequested)
			break;
		Sleep(10); // prevent 100% CPU consuming
	}
	return 0;
};
