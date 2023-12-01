// handles everything
#include "Application.h"
// program entry point
int main()
{
	Application Descender;
	if (Descender.Init()) {
		if (Descender.Run()) {
			return Descender.Shutdown() ? 0 : 1;
		}
	}
	return 1;
}