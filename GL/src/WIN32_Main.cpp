#define _WIN32_WINNT 0x0501

char const* AppName="GL Test";

#include <Windows.h>
#include <WinGDI.h>

#include <gl/GL.h>
#include "glext.h"
#include "wglext.h"
#include "FIMM.h"


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

int W=640;
int H=480;

#include "cat_ib.h"
#include "cat_vb.h"

const int vbsize=sizeof(vb);
const int ibsize=sizeof(ib);
const int vertxcount=vbsize/sizeof(vb[0]);
const int indexcount=ibsize/sizeof(ib[0]);

int __cdecl main(int argc, char* argv[])
{
	HCURSOR arrowCursor=LoadCursor(0, IDC_ARROW);

	WNDCLASSEX wc={sizeof(WNDCLASSEX), 0, wp, 0, 0, 0, 0, 0, 0, 0, "GLAP301"};
	wc.hCursor=arrowCursor;
	RegisterClassEx(&wc);

	RECT wrect={0, 0, W, H};
	AdjustWindowRectEx(&wrect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);
	HWND win=CreateWindowEx(WS_EX_APPWINDOW, "GLAP301", "Render Window", WS_OVERLAPPEDWINDOW, 0, 0,
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

	GLuint ibo=-1;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibsize, ib, GL_STATIC_DRAW);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLuint vbo=-1;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vbsize, vb, GL_STATIC_DRAW);

//	glVertexPointer(3, GL_FLOAT, sizeof(vb[0])*16, 0);
//	glNormalPointer(GL_FLOAT, sizeof(vb[0])*16, (void*)(sizeof(vb[0])*3));
//	glTangentPointer(3, GL_FLOAT, sizeof(vb[0])*16, 0);
//	glBinormalPointer(3, GL_FLOAT, sizeof(vb[0])*16, 0);
//	glTexCoordPointer(2, GL_FLOAT, sizeof(vb[0])*16, 0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vb[0])*16, (void*)(0*sizeof(float)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vb[0])*16, (void*)(12*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableClientState(GL_VERTEX_ARRAY);

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

		sh.Use();

//		int apos = glGetAttribLocation(sh.GetPName(), "in_Normal");
//		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vb[0])*16, (void*)(8*sizeof(float)));

		glClearColor(0.8f,0.6f,0.5f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

//		glEnable(GL_TEXTURE_2D);
/*
		glBegin(GL_TRIANGLES);
		glColor3f(1,0,1);
		glTexCoord2f(0, 1); glVertex3f(-0.5,-1.0, 0.5); 
		glTexCoord2f(0, 0); glVertex3f(-0.5, 1.0, 0.5); 
		glTexCoord2f(1, 0); glVertex3f( 0.5, 1.0, 0.5);
		glTexCoord2f(0, 1); glVertex3f(-0.5,-1.0, 0.5); 
		glTexCoord2f(1, 1); glVertex3f( 0.5,-1.0, 0.5); 
		glTexCoord2f(1, 0); glVertex3f( 0.5, 1.0, 0.5);
		glEnd();
*/
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
