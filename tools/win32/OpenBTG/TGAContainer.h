/*
** TGAContainer.h : Header file for TGAContainer.cpp.
*/

#ifndef __TGACONTAINER_H
#define __TGACONTAINER_H

#include "ccList.h"
#include "TGA.h"

class CTGAContainer : public ccNode
{
private:
protected:
public:
	TGAFile myFile;
	CString myFileName;

	CTGAContainer(char *fileName);
	~CTGAContainer();
};

#endif