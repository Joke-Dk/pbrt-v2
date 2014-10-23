//add by dk
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_SHAPES_WAVEFRONT_H
#define PBRT_SHAPES_WAVEFRONT_H

/*
*  wavefront.cpp
*  Mark Colbert
*  ma434187@pegasus.cc.ucf.edu	
*
*  Jan Kautz: Fixed bug for nVerticesTotal size not being set.
*
*  Carsten Dachsbacher, bug fixed: for obj-files with unused vertices, we need to pass
*  the complete vertex list (=> nVerticesTotal)
*
*  The following will parse triangulated obj files that can eaisly be exported from 
*  lightwave, maya, or 3d studio max.  
*
*  TODO: The largest problem about this program is that if there are a different
*	number of normal verticies or UV verticies than point verticies, which there usually are, the 
*  program will just create as many verticies as there are in the index to 
*  easily ensure that every point has the proper UV, normal, and point vertex
*  information.  The fix is to just use a more efficient methodology of combining
*  all the data.
*  
*  TODO: The other problem about this parser is that it only works with triangulated data.
*	In other words, if you give it an object that is anything but triangles, it will
*  only read the first 3 vertecies of a face and you will get one very odd looking
*  model.  Thus, it would be nice to have some sort of triangulation algorithm in
*  this program to allow any generic OBJ file to be input for the plugin.
*
*/


#include "shape.h"
#include "paramset.h"
//#include "dynload.h"
//#include "api.cpp"
#include "iostream"

// line buffer size determines at compile time how large the input
// buffer should be for the file input lines
#define LINE_BUFFER_SIZE 1024

class Wavefront : public Shape {
public:
	Wavefront(const char* filename, const Transform *o2w, const Transform *w2o, bool reverseOrientation);
	~Wavefront() {
		if (vertexIndex) delete[] vertexIndex;
		if (p) delete[] p;
		if (n) delete[] n;
		if (uvs) delete[] uvs;
	}

	BBox ObjectBound() const;
	BBox WorldBound() const;
	bool CanIntersect() const { return false; }
	void Refine(vector<Reference<Shape> > &refined) const;

private:
	void MergeIndicies(vector<Point> &points, vector<Normal> &normals, vector<float> &uvVec,
		vector<int> &vIndex, vector<int> &normalIndex, vector<int> &uvIndex);
	int ntris, nverts;
	int nVerticesTotal;
	int *vertexIndex;
	Point *p;
	Normal *n;
	float *uvs;
};

Shape *CreateShape(const Transform *o2w, const Transform *w2o, bool reverseOrientation, const ParamSet &params);

#endif // PBRT_CORE_SHAPE_H