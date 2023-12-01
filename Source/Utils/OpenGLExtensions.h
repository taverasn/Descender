#ifndef OGL_EXTENSIONS_H
#define OGL_EXTENSIONS_H

// Modern OpenGL API Functions must be queried before use
PFNGLCREATESHADERPROC				glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC				glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC				glCompileShader = nullptr;
PFNGLGETSHADERIVPROC				glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog = nullptr;
PFNGLATTACHSHADERPROC				glAttachShader = nullptr;
PFNGLDETACHSHADERPROC				glDetachShader = nullptr;
PFNGLDELETESHADERPROC				glDeleteShader = nullptr;
PFNGLCREATEPROGRAMPROC				glCreateProgram = nullptr;
PFNGLLINKPROGRAMPROC				glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC					glUseProgram = nullptr;
PFNGLGETPROGRAMIVPROC				glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog = nullptr;
PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC			glBindVertexArray = nullptr;
PFNGLGENBUFFERSPROC					glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC					glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC					glBufferData = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer = nullptr;
PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv = nullptr;
PFNGLUNIFORMMATRIX3FVPROC			glUniformMatrix3fv = nullptr;
PFNGLUNIFORMMATRIX2FVPROC			glUniformMatrix2fv = nullptr;
PFNGLDELETEBUFFERSPROC				glDeleteBuffers = nullptr;
PFNGLDELETEPROGRAMPROC				glDeleteProgram = nullptr;
PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays = nullptr;
PFNGLDEBUGMESSAGECALLBACKPROC		glDebugMessageCallback = nullptr;
// DONE: Part 2d
PFNGLGETUNIFORMBLOCKINDEXPROC       glGetUniformBlockIndex = nullptr;
PFNGLBINDBUFFERBASEPROC             glBindBufferBase = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC        glUniformBlockBinding = nullptr;
PFNGLBUFFERSUBDATAPROC              glBufferSubData = nullptr;
PFNGLDRAWELEMENTSINSTANCEDPROC      glDrawElementsInstanced = nullptr;
PFNGLVERTEXATTRIBDIVISORPROC        glVertexAttribDivisor = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex = nullptr;
PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex = nullptr;
PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;

// Sprites and Textures
PFNGLGENERATEMIPMAPPROC				glGenerateMipmap = nullptr;
PFNGLACTIVETEXTUREPROC				glActiveTexture = nullptr;
PFNGLUNIFORM1FVPROC					glUniform1fv = nullptr;
PFNGLUNIFORM2FVPROC					glUniform2fv = nullptr;
PFNGLUNIFORM3FVPROC					glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC					glUniform4fv = nullptr;
PFNGLUNIFORM4FPROC					glUniform4f = nullptr;
PFNGLUNIFORM2FPROC					glUniform2f = nullptr;
PFNGLBLENDFUNCSEPARATEPROC			glBlendFuncSeparate = nullptr;
PFNGLMAPBUFFERPROC					glMapBuffer = nullptr;
PFNGLUNMAPBUFFERPROC				glUnmapBuffer = nullptr;


void QueryOGLExtensionFunctions(GW::GRAPHICS::GOpenGLSurface ogl)
{
	ogl.QueryExtensionFunction(nullptr, "glCreateShader", (void**)&glCreateShader);
	ogl.QueryExtensionFunction(nullptr, "glShaderSource", (void**)&glShaderSource);
	ogl.QueryExtensionFunction(nullptr, "glCompileShader", (void**)&glCompileShader);
	ogl.QueryExtensionFunction(nullptr, "glGetShaderiv", (void**)&glGetShaderiv);
	ogl.QueryExtensionFunction(nullptr, "glGetShaderInfoLog", (void**)&glGetShaderInfoLog);
	ogl.QueryExtensionFunction(nullptr, "glAttachShader", (void**)&glAttachShader);
	ogl.QueryExtensionFunction(nullptr, "glDetachShader", (void**)&glDetachShader);
	ogl.QueryExtensionFunction(nullptr, "glDeleteShader", (void**)&glDeleteShader);
	ogl.QueryExtensionFunction(nullptr, "glCreateProgram", (void**)&glCreateProgram);
	ogl.QueryExtensionFunction(nullptr, "glLinkProgram", (void**)&glLinkProgram);
	ogl.QueryExtensionFunction(nullptr, "glUseProgram", (void**)&glUseProgram);
	ogl.QueryExtensionFunction(nullptr, "glGetProgramiv", (void**)&glGetProgramiv);
	ogl.QueryExtensionFunction(nullptr, "glGetProgramInfoLog", (void**)&glGetProgramInfoLog);
	ogl.QueryExtensionFunction(nullptr, "glGenVertexArrays", (void**)&glGenVertexArrays);
	ogl.QueryExtensionFunction(nullptr, "glBindVertexArray", (void**)&glBindVertexArray);
	ogl.QueryExtensionFunction(nullptr, "glGenBuffers", (void**)&glGenBuffers);
	ogl.QueryExtensionFunction(nullptr, "glBindBuffer", (void**)&glBindBuffer);
	ogl.QueryExtensionFunction(nullptr, "glBufferData", (void**)&glBufferData);
	ogl.QueryExtensionFunction(nullptr, "glEnableVertexAttribArray", (void**)&glEnableVertexAttribArray);
	ogl.QueryExtensionFunction(nullptr, "glDisableVertexAttribArray", (void**)&glDisableVertexAttribArray);
	ogl.QueryExtensionFunction(nullptr, "glVertexAttribPointer", (void**)&glVertexAttribPointer);
	ogl.QueryExtensionFunction(nullptr, "glGetUniformLocation", (void**)&glGetUniformLocation);
	ogl.QueryExtensionFunction(nullptr, "glUniformMatrix4fv", (void**)&glUniformMatrix4fv);
	ogl.QueryExtensionFunction(nullptr, "glUniformMatrix3fv", (void**)&glUniformMatrix3fv);
	ogl.QueryExtensionFunction(nullptr, "glUniformMatrix2fv", (void**)&glUniformMatrix2fv);
	ogl.QueryExtensionFunction(nullptr, "glDeleteBuffers", (void**)&glDeleteBuffers);
	ogl.QueryExtensionFunction(nullptr, "glDeleteProgram", (void**)&glDeleteProgram);
	ogl.QueryExtensionFunction(nullptr, "glDeleteVertexArrays", (void**)&glDeleteVertexArrays);
	ogl.QueryExtensionFunction(nullptr, "glDebugMessageCallback", (void**)&glDebugMessageCallback);
	// DONE: Part 2d
	ogl.QueryExtensionFunction(nullptr, "glGetUniformBlockIndex", (void**)&glGetUniformBlockIndex);
	ogl.QueryExtensionFunction(nullptr, "glBindBufferBase", (void**)&glBindBufferBase);
	ogl.QueryExtensionFunction(nullptr, "glUniformBlockBinding", (void**)&glUniformBlockBinding);
	ogl.QueryExtensionFunction(nullptr, "glBufferSubData", (void**)&glBufferSubData);
	ogl.QueryExtensionFunction(nullptr, "glDrawElementsInstanced", (void**)&glDrawElementsInstanced);
	ogl.QueryExtensionFunction(nullptr, "glVertexAttribDivisor", (void**)&glVertexAttribDivisor);
	ogl.QueryExtensionFunction(nullptr, "glDrawElementsInstancedBaseVertex", (void**)&glDrawElementsInstancedBaseVertex);
	ogl.QueryExtensionFunction(nullptr, "glGetProgramResourceIndex", (void**)&glGetProgramResourceIndex);
	ogl.QueryExtensionFunction(nullptr, "glShaderStorageBlockBinding", (void**)&glShaderStorageBlockBinding);

	ogl.QueryExtensionFunction(nullptr, "glUniform1fv", (void**)&glUniform1fv);
	ogl.QueryExtensionFunction(nullptr, "glUniform2fv", (void**)&glUniform2fv);
	ogl.QueryExtensionFunction(nullptr, "glUniform3fv", (void**)&glUniform3fv);
	ogl.QueryExtensionFunction(nullptr, "glUniform4fv", (void**)&glUniform4fv);
	ogl.QueryExtensionFunction(nullptr, "glGenerateMipmap", (void**)&glGenerateMipmap);
	ogl.QueryExtensionFunction(nullptr, "glActiveTexture", (void**)&glActiveTexture);
	ogl.QueryExtensionFunction(nullptr, "glBlendFuncSeparate", (void**)&glBlendFuncSeparate);
	ogl.QueryExtensionFunction(nullptr, "glMapBuffer", (void**)&glMapBuffer);
	ogl.QueryExtensionFunction(nullptr, "glUnmapBuffer", (void**)&glUnmapBuffer);
	ogl.QueryExtensionFunction(nullptr, "glUniform3f", (void**)&glUniform3f);
	ogl.QueryExtensionFunction(nullptr, "glUniform2f", (void**)&glUniform2f);
	ogl.QueryExtensionFunction(nullptr, "glUniform4f", (void**)&glUniform4f);
	ogl.QueryExtensionFunction(nullptr, "glUniform1f", (void**)&glUniform1f);
	ogl.QueryExtensionFunction(nullptr, "glUniform1i", (void**)&glUniform1i);
	
}

#endif
