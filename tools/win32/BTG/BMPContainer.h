/*
** BMPContainer.h : Header file for BMPContainer.cpp.
*/

#ifndef __BMPCONTAINER_H
#define __BMPCONTAINER_H

#include "ccList.h"
#include "bmp.h"

class CBMPContainer : public ccNode
{
private:
protected:
public:
	BMPFile myFile;
	CString myFileName;

	CBMPContainer(char *fileName);
	~CBMPContainer();
};

#endif