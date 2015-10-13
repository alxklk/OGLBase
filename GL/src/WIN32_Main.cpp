#define _WIN32_WINNT 0x0501

char const* AppName="Untitled";

#include <Windows.h>
#include <WinGDI.h>


#include <gl/GL.h>
#include "glext.h"
#include "wglext.h"


PFNGLCREATESHADERPROC         glCreateShader        ;
PFNGLSHADERSOURCEPROC         glShaderSource        ;
PFNGLCOMPILESHADERPROC        glCompileShader       ;
PFNGLGETSHADERIVPROC          glGetShaderiv         ;
PFNGLGETSHADERINFOLOGPROC     glGetShaderInfoLog    ;
PFNGLCREATEPROGRAMPROC        glCreateProgram       ;
PFNGLATTACHSHADERPROC         glAttachShader        ;
PFNGLLINKPROGRAMPROC          glLinkProgram         ;
PFNGLUSEPROGRAMPROC           glUseProgram          ;

class GLShader
{
	int vsh;
	int psh;
	int p;
public:
	bool Create(const char* vstext, const char* pstext)
	{
		vsh=glCreateShader(GL_VERTEX_SHADER);
		psh=glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vsh, 1, &vstext, 0);
		glShaderSource(psh, 1, &pstext, 0);

		GLint ok;
		char errorbuf[1024];
		int errorlen=0;
			
			
		glCompileShader(vsh);
		glGetShaderiv(vsh, GL_COMPILE_STATUS, &ok);
		if(!ok)
		{
			glGetShaderInfoLog(vsh, 1024, &errorlen, errorbuf);
			OutputDebugString(errorbuf);
			return false;
		}	

		glCompileShader(psh);
		glGetShaderiv(psh, GL_COMPILE_STATUS, &ok);
		if(!ok)
		{
			glGetShaderInfoLog(psh, 1024, &errorlen, errorbuf);
			OutputDebugString(errorbuf);
			return false;
		}
		p=glCreateProgram();
		glAttachShader(p,vsh);
		glAttachShader(p,psh);
		glLinkProgram(p);
		return true;
	}
	bool CreateFromFile(const char* filename)
	{
	}
	bool Use()
	{
		glUseProgram(p);
		return true;
	}
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







HWND wnd;
void stop(HWND hwnd,unsigned int imsg,WPARAM wpar,LPARAM lpar)
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
	return DefWindowProc(hwnd,imsg,wpar,lpar);
}

int W=640;
int H=480;

#ifdef _DEBUG
int __cdecl main(int argc, char* argv[])
#else

int* __cdecl _errno(){return 0;};

extern "C"
void __cdecl start()
#endif
{
	WNDCLASSEX wc={sizeof(WNDCLASSEX),0,wp,0,0,0,0,0,0,0,"BrickGameWindowClass"};
	RegisterClassEx(&wc);
	wnd=CreateWindowEx(WS_EX_APPWINDOW,"BrickGameWindowClass",AppName,WS_POPUP,CW_USEDEFAULT,CW_USEDEFAULT,W,H,0,0,0,0);

	ShowWindow(wnd,SW_SHOWNORMAL);
	UpdateWindow(wnd);

	MSG msg={0,0,0,0,0,{0,0}};

	CUIGL ui;
	ui.Init(W,H,1,1,wnd);

	/*PFNGLCREATESHADERPROC         */glCreateShader        =(PFNGLCREATESHADERPROC)         wglGetProcAddress("glCreateShader"        );
	/*PFNGLSHADERSOURCEPROC         */glShaderSource        =(PFNGLSHADERSOURCEPROC)         wglGetProcAddress("glShaderSource"        );
	/*PFNGLCOMPILESHADERPROC        */glCompileShader       =(PFNGLCOMPILESHADERPROC)        wglGetProcAddress("glCompileShader"       );
	/*PFNGLGETSHADERIVPROC          */glGetShaderiv         =(PFNGLGETSHADERIVPROC)          wglGetProcAddress("glGetShaderiv"         );
	/*PFNGLGETSHADERINFOLOGPROC     */glGetShaderInfoLog    =(PFNGLGETSHADERINFOLOGPROC)     wglGetProcAddress("glGetShaderInfoLog"    );
	/*PFNGLCREATEPROGRAMPROC        */glCreateProgram       =(PFNGLCREATEPROGRAMPROC)        wglGetProcAddress("glCreateProgram"       );
	/*PFNGLATTACHSHADERPROC         */glAttachShader        =(PFNGLATTACHSHADERPROC)         wglGetProcAddress("glAttachShader"        );
	/*PFNGLLINKPROGRAMPROC          */glLinkProgram         =(PFNGLLINKPROGRAMPROC)          wglGetProcAddress("glLinkProgram"         );
	/*PFNGLUSEPROGRAMPROC           */glUseProgram          =(PFNGLUSEPROGRAMPROC)           wglGetProcAddress("glUseProgram"          );

	GLuint textureA;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureA);
	glBindTexture(GL_TEXTURE_2D, textureA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA,  GL_UNSIGNED_BYTE, NULL);

			
			const GLchar* vshtext=
			"varying vec2 v_p;"
			"void main(){"
			"gl_Position=gl_Vertex;"
			"v_p=gl_Vertex.xy/2.0+0.5;"
			"v_p.y=1.0-v_p.y;"
			"}"
			;
			const GLchar* pshtext=
			"uniform sampler2D tex;"
			"uniform vec2 mp;"
			"varying vec2 v_p;"
			" void main(){"
			" vec2 tc=v_p;"
			"tc.x+=sin(v_p.y*5.0)*0.1;"
			"tc.y+=sin(v_p.x*5.0)*0.1;"
			"vec4 c=texture2D(tex, tc.xy);"
			"gl_FragColor=c;"
			"if(c.r>0.0)"
			"gl_FragColor=vec4(1.0,0.0,0.0,1.0);"
			"else "
			"gl_FragColor=vec4(1.0,1.0,0.0,1.0);"
			"}";
			
			//		const GLchar* pshtext="varying vec3 v_p; void main(){gl_FragColor=vec4(v_p.x, v_p.y, 0.0, 1.0);}";
			
			GLShader sh;
			sh.Create(vshtext, pshtext);


	while(1)
	{
		while(PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
			if(msg.message==WM_QUIT)
				break;
		}

		glUseProgram(0);

		glClearColor(0.3f,0.5f,0.7f,0.5f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDisable(GL_TEXTURE_2D);

		glBegin(GL_TRIANGLES);
		
		glColor3ub(1,0,0);
		glVertex2f(-1.0, -1.0); 
		glVertex2f(-1.0,  1.0); 
		glVertex2f( 1.0,  1.0);

		glColor3ub(0,1,0);
		glVertex2f(-0.3,-0.3); 
		glVertex2f(-0.3, 0.3); 
		glVertex2f( 0.3, 0.3);
		
		glEnd();

		sh.Use();
		glBindTexture(GL_TEXTURE_2D, textureA);

		glClearColor(0.8f,0.6f,0.5f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);

		glBegin(GL_TRIANGLES);
		glColor3f(1,1,1);
		glTexCoord2f(0, 1); glVertex2f(-1.0,-1.0); 
		glTexCoord2f(0, 0); glVertex2f(-1.0, 1.0); 
		glTexCoord2f(1, 0); glVertex2f( 1.0, 1.0);
		glTexCoord2f(0, 1); glVertex2f(-1.0,-1.0); 
		glTexCoord2f(1, 1); glVertex2f( 1.0,-1.0); 
		glTexCoord2f(1, 0); glVertex2f( 1.0, 1.0);
		glEnd();


		ui.FrameEnd();
		Sleep(10);
		if(msg.message==WM_QUIT)
			break;
	}
#ifdef _DEBUG
	return 0;
#else
	ExitProcess(0);
#endif

};
