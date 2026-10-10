#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef NDEBUG
#define GL_ERROR()
#else
void GLCheckError(const char* filename, unsigned int line);
#define	GL_ERROR()	GLCheckError(__FILE__, __LINE__)
#endif

#endif
