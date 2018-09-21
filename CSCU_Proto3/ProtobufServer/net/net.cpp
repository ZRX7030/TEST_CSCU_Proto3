#include "net.h"

Net::Net()
{
	terminated = false;
}

Net::~Net()
{

}

char *Net::errorString(int err)
{
	return NULL; 
}


void Net::destroy()
{
	terminated = true;
}
