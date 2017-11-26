/* See LICENSE file for copyright and license details. */
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}

void
die(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

void
infogl(const int verbose)
{
#define PBYTE(CHAINE) ( (CHAINE) != NULL ? (CHAINE) : (const GLubyte *) "????" )
#define PCHAR(CHAINE) ( (CHAINE) != NULL ? (CHAINE) : (const char *) "????" )
	SDL_version linked;
	SDL_GetVersion( &linked );
	printf("// SDL    %u.%u.%u\n", linked.major, linked.minor, linked.patch);

	printf("// OpenGL %s%s\n", PBYTE(glGetString(GL_VERSION)), PBYTE(glGetString(GL_VENDOR)));
	printf("// GPU    %s\n", PBYTE(glGetString(GL_RENDERER)));
	printf("// GLSL   %s\n", PBYTE(glGetString(GL_SHADING_LANGUAGE_VERSION)));

	if (verbose)
		printf("// extensions = %s\n", PBYTE(glGetString(GL_EXTENSIONS)));
#undef PBYTE
#undef PCHAR
}

void
sdldie(const char *msg)
{
	fprintf(stderr, "sdldie: %s: %s\n",msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}

void
sdlerrcheck(int line)
{
	const char *sdlerror = SDL_GetError();
	if ( *sdlerror != '\0' )
	{
		fprintf(stderr,"SDL Error: l.%d: %s\n",line, sdlerror);
		SDL_ClearError();
	}
}

int
checkglerr(const char *msg, const int line)
{
	const char *errors[7] = {
		"GL_INVALID_ENUM",
		"GL_INVALID_VALUE",
		"GL_INVALID_OPERATION",
		"GL_STACK_OVERFLOW",
		"GL_STACK_UNDERFLOW",
		"GL_OUT_OF_MEMORY",
		"GL_INVALID_FRAMEBUFFER_OPERATION",
	};
	int rc = 0;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		if (err >= GL_INVALID_ENUM && err <= GL_INVALID_FRAMEBUFFER_OPERATION)
			fprintf(stderr, "%s l.%d\n", errors[err - GL_INVALID_ENUM], line);
		else
			fprintf(stderr, "Unknown error l.%d\n", line);
		++rc;
	}
	return rc;
}

void
printcomplog(GLuint shaderobj)
{
	int logl;
	char *log;

	glGetShaderiv(shaderobj, GL_INFO_LOG_LENGTH, &logl);
	if (logl > 0) {
		log = (char*)malloc(sizeof(char) * logl);
		if (!log)
			die("Cannot allocate %d bytes for shader log\n", logl);
		glGetShaderInfoLog(shaderobj, logl, NULL, log);
		fprintf(stderr, "\n%s\n", log);
		free(log);
	}
}

void
printprogramlog(GLuint prog)
{
	int logl;
	char *log;

	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logl);
	if (logl > 0) {
		log = (char*)malloc(sizeof(char) * logl);
		if (!log)
			die("Cannot allocate %d bytes for program log\n", logl);
		glGetProgramInfoLog(prog, logl, NULL, log);
		fprintf(stderr, "\n%s\n", log);
		free(log);
	}
}

void
debugcallback(GLenum source, GLenum type, GLuint id,
   GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	printf("%s\n", message);
}
