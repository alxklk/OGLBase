#define _WIN32_WINNT 0x0501

char const* AppName="GL Test";

#include <Windows.h>
#include <WinGDI.h>

#include <gl/GL.h>
#include "glext.h"
#include "wglext.h"
#include "FIMM.h"
#include "picopng.h"

#include <stdio.h>


PFNGLCREATESHADERPROC                  glCreateShader                           ;
PFNGLSHADERSOURCEPROC                  glShaderSource                           ;
PFNGLCOMPILESHADERPROC                 glCompileShader                          ;
PFNGLGETSHADERIVPROC                   glGetShaderiv                            ;
PFNGLGETSHADERINFOLOGPROC              glGetShaderInfoLog                       ;
PFNGLCREATEPROGRAMPROC                 glCreateProgram                          ;
PFNGLATTACHSHADERPROC                  glAttachShader                           ;
PFNGLLINKPROGRAMPROC                   glLinkProgram                            ;
PFNGLUSEPROGRAMPROC                    glUseProgram                             ;
PFNGLGENBUFFERSPROC                    glGenBuffers                             ;
PFNGLDELETEBUFFERSPROC                 glDeleteBuffers                          ;
PFNGLBINDBUFFERPROC                    glBindBuffer                             ;
PFNGLBUFFERDATAPROC                    glBufferData                             ;
PFNGLMAPBUFFERPROC                     glMapBuffer                              ;
PFNGLUNMAPBUFFERPROC                   glUnmapBuffer                            ;
PFNGLVERTEXATTRIBPOINTERPROC           glVertexAttribPointer                    ;
PFNGLGETATTRIBLOCATIONPROC             glGetAttribLocation                      ;
PFNGLENABLEVERTEXATTRIBARRAYPROC       glEnableVertexAttribArray                ;
PFNGLACTIVETEXTUREPROC                 glActiveTexture                          ;
PFNGLPROGRAMUNIFORM1IPROC              glProgramUniform1i                       ;
PFNGLGETUNIFORMLOCATIONPROC            glGetUniformLocation                     ;

struct SAppState
{
public:
	bool quitRequested;
	bool reloadShaders;
	int frameStamp;

	SAppState():
		quitRequested(false),
		reloadShaders(true),
		frameStamp(0)
	{

	}
}gAppState;



void LOG(const char* logtext, int level=0)
{
	OutputDebugString(logtext);
//	puts(logtext);
}


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
		glAttachShader(p,vsh);
		glAttachShader(p,fsh);
		glLinkProgram(p);
		return true;
	}
	bool CreateFromFile(const char* vsfilename, const char* fsfilename)
	{
		CFIMM vsfi;
		CFIMM fsfi;

		vsfi.Open(vsfilename);
		fsfi.Open(fsfilename);

		if(vsfi.Addr()&&vsfi.Addr())
		{
			return Create(vsfi.CAddr(),fsfi.CAddr(),vsfi.GetSize(),fsfi.GetSize());
		}
		LOG("\nShader source file - ?\n");
		return false;
	}
	bool Use()
	{
		glUseProgram(p);
		return true;
	}
	int GetPName()const{return p;}
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
		PIXELFORMATDESCRIPTOR pfd={0, 0, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // wrong way ;)
		SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd);
		glrc=wglCreateContext(dc);
		wglMakeCurrent(dc,glrc);

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
		wglMakeCurrent(dc,0);
		ReleaseDC(wnd,dc);
		return 0;
	}

};


HWND wnd;void stop(HWND hwnd,unsigned int imsg,WPARAM wpar,LPARAM lpar)
{
	PostQuitMessage(0);
}

union XYLPARAM
{
	unsigned int l;
	struct{
		short int x;
		short int y;
	};
	XYLPARAM(LPARAM I):l((unsigned int)I){};
};

LRESULT CALLBACK wp(HWND hwnd,unsigned int imsg,WPARAM wpar,LPARAM lpar)
{
	if(imsg==WM_DESTROY) 
	{
		stop(hwnd,imsg,wpar,lpar);
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
	}

	return DefWindowProc(hwnd,imsg,wpar,lpar);
}

int W=1280;
int H=720;

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
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bytes);
	delete[] bytes;
	return tex;
}


int __cdecl main(int argc, char* argv[])
{
	/*
	FILE* f=fopen("out.h", "wb");
	char* sym[]={"x", "y", "z", "nx", "ny", "nz", "s", "t", "s1", "t1", "tx", "ty", "tz", "bx", "by", "bz"};
	char* nym[]={"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"};

	
	fprintf(f, "//");
	for(int j=0; j<16; j++)
	{
		fprintf(f, "% 12s", sym[j]);
	}
	fprintf(f, "\n");
	fprintf(f, "//");
	for(int j=0; j<16; j++)
	{
		fprintf(f, "% 12s", nym[j]);
	}

	fprintf(f, "float vb[]={\n");
	for(int i=0; i<vbsize/sizeof(vb[0])/16; i++)
	{
		for(int j=0; j<16; j++)
		{
			fprintf(f, "% 11.6f,", vb[j+i*16]);
		}
		fprintf(f, "\n");
	}

	fprintf(f, "\n};\n");
	fclose(f);

	return 0;
	*/

	HCURSOR arrowCursor=LoadCursor(0, IDC_ARROW);

	WNDCLASSEX wc={sizeof(WNDCLASSEX), 0, wp, 0, 0, 0, 0, 0, 0, 0, "GLAP301"};
	wc.hCursor=arrowCursor;
	RegisterClassEx(&wc);

	RECT wrect={0, 0, W, H};
	AdjustWindowRectEx(&wrect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);
	HWND win=CreateWindowEx(WS_EX_APPWINDOW, "GLAP301", "Render Window", WS_OVERLAPPEDWINDOW, -1300, 0,
		wrect.right-wrect.left, wrect.bottom-wrect.top, 0, 0, 0, 0);

	ShowWindow(win, SW_SHOWNORMAL);
	UpdateWindow(win);

	MSG msg={0,0,0,0,0,{0,0}};

	CUIGL GL;
	GL.Init(W,H,1,1,win);

	/*PFNGLCREATESHADERPROC             */glCreateShader              =(PFNGLCREATESHADERPROC)              wglGetProcAddress("glCreateShader"              );
	/*PFNGLSHADERSOURCEPROC             */glShaderSource              =(PFNGLSHADERSOURCEPROC)              wglGetProcAddress("glShaderSource"              );
	/*PFNGLCOMPILESHADERPROC            */glCompileShader             =(PFNGLCOMPILESHADERPROC)             wglGetProcAddress("glCompileShader"             );
	/*PFNGLGETSHADERIVPROC              */glGetShaderiv               =(PFNGLGETSHADERIVPROC)               wglGetProcAddress("glGetShaderiv"               );
	/*PFNGLGETSHADERINFOLOGPROC         */glGetShaderInfoLog          =(PFNGLGETSHADERINFOLOGPROC)          wglGetProcAddress("glGetShaderInfoLog"          );
	/*PFNGLCREATEPROGRAMPROC            */glCreateProgram             =(PFNGLCREATEPROGRAMPROC)             wglGetProcAddress("glCreateProgram"             );
	/*PFNGLATTACHSHADERPROC             */glAttachShader              =(PFNGLATTACHSHADERPROC)              wglGetProcAddress("glAttachShader"              );
	/*PFNGLLINKPROGRAMPROC              */glLinkProgram               =(PFNGLLINKPROGRAMPROC)               wglGetProcAddress("glLinkProgram"               );
	/*PFNGLUSEPROGRAMPROC               */glUseProgram                =(PFNGLUSEPROGRAMPROC)                wglGetProcAddress("glUseProgram"                );
	/*PFNGLGENBUFFERSPROC               */glGenBuffers                =(PFNGLGENBUFFERSPROC)                wglGetProcAddress("glGenBuffers"                );
	/*PFNGLDELETEBUFFERSPROC            */glDeleteBuffers             =(PFNGLDELETEBUFFERSPROC)             wglGetProcAddress("glDeleteBuffers"             );
	/*PFNGLBINDBUFFERPROC               */glBindBuffer                =(PFNGLBINDBUFFERPROC)                wglGetProcAddress("glBindBuffer"                );
	/*PFNGLBUFFERDATAPROC               */glBufferData                =(PFNGLBUFFERDATAPROC)                wglGetProcAddress("glBufferData"                );
	/*PFNGLMAPBUFFERPROC                */glMapBuffer                 =(PFNGLMAPBUFFERPROC)                 wglGetProcAddress("glMapBuffer"                 );
	/*PFNGLUNMAPBUFFERPROC              */glUnmapBuffer               =(PFNGLUNMAPBUFFERPROC)               wglGetProcAddress("glUnmapBuffer"               );
	/*PFNGLVERTEXATTRIBPOINTERPROC      */glVertexAttribPointer       =(PFNGLVERTEXATTRIBPOINTERPROC)       wglGetProcAddress("glVertexAttribPointer"       );
	/*PFNGLGETATTRIBLOCATIONPROC        */glGetAttribLocation         =(PFNGLGETATTRIBLOCATIONPROC)         wglGetProcAddress("glGetAttribLocation"         );
	/*PFNGLENABLEVERTEXATTRIBARRAYPROC  */glEnableVertexAttribArray   =(PFNGLENABLEVERTEXATTRIBARRAYPROC)   wglGetProcAddress("glEnableVertexAttribArray"   );
	/*PFNGLACTIVETEXTUREPROC            */glActiveTexture             =(PFNGLACTIVETEXTUREPROC)             wglGetProcAddress("glActiveTexture"             );
	/*PFNGLPROGRAMUNIFORM1IPROC         */glProgramUniform1i          =(PFNGLPROGRAMUNIFORM1IPROC)          wglGetProcAddress("glProgramUniform1i"          );
	/*PFNGLGETUNIFORMLOCATIONPROC       */glGetUniformLocation        =(PFNGLGETUNIFORMLOCATIONPROC)        wglGetProcAddress("glGetUniformLocation"        );

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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4*16, (void*)(0 *4));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 4*16, (void*)(3 *4));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 4*16, (void*)(10*4));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 4*16, (void*)(13*4));
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 4*16, (void*)(6 *4));

	glEnable(GL_DEPTH);

	GLuint tex=LoadTexture("..\\bin\\data\\body_diff.png");
	GLuint norm=LoadTexture("..\\bin\\data\\body_norm.png");

	GLShader sh;
	while(1)
	{
		while(PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
			if(msg.message==WM_QUIT)
				break;
		}

		if(gAppState.reloadShaders)
		{
			sh.CreateFromFile("..\\src\\model.vs","..\\src\\model.fs");
			gAppState.reloadShaders=false;
		}


//		int apos = glGetAttribLocation(sh.GetPName(), "in_Normal");
//		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vb[0])*16, (void*)(8*sizeof(float)));

//		GLint texUniform=glGetUniformLocation(sh.GetPName(), "sTex");
		glProgramUniform1i(sh.GetPName(), 0, 0);
		glProgramUniform1i(sh.GetPName(), 1, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, norm);

		glClearColor(0.8f, 0.6f, 0.5f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);
		glDepthFunc(GL_LESS);
		glCullFace(GL_FRONT_AND_BACK);
		sh.Use();

		glDrawElements(GL_TRIANGLES, indexcount, GL_UNSIGNED_INT, 0);
		GL.FrameEnd();
		if(msg.message==WM_QUIT)
			break;
		if(gAppState.quitRequested)
			break;
		Sleep(10);
	}
	return 0;
};
