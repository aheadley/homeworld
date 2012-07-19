/*
** REORIENT.H : Header file for REORIENT.CPP.
*/

#ifndef __REORIENT_H
#define __REORIENT_H

#include "matrix.h"

	void ShipReorientPosition(float *xp, float *yp, float *zp);
	void WorldReorientPosition(float *xp, float *yp, float *zp);

	void ShipReorientAngles(float *xr, float *yr, float *zr);
	void WorldReorientAngles(float *xr, float *yr, float *zr);

	void VectorFromShipAngle(float *xr, float *yr, float *zr);
	void VectorFromWorldAngle(float *xr, float *yr, float *zr);

    void nisShipEulerToMatrix(matrix *coordsys, vector *rotVector);
#endif