#include <GL/glew.h>
/* See LICENSE file for copyright and license details. */

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))
#define LENGTH(X)               (sizeof X / sizeof X[0])

int checkglerr(const char *msg, const int line);
void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
void infogl(const int verbose);
void sdldie(const char *msg);
void sdlerrcheck(int line);
void printcomplog(GLuint shaderobj);
void printprogramlog(GLuint prog);
void debugcallback(GLenum source, GLenum type, GLuint id,
   GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
