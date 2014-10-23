//add by dk
#include "stdafx.h"
#include "shapes/wavefront.h"
#include "api.cpp"

// GETNUM just gets the next number from a line of input in an OBJ file
#ifndef GETNUM
#define GETNUM(lineBuffer, numBuffer, lindex, nindex, tval)  \
	nindex=0;\
	while ((lineBuffer[lindex] == ' ') || lineBuffer[lindex] == '/') lindex++;\
	while ((lineBuffer[lindex] != ' ') && (lineBuffer[lindex] != '/') && \
	(lineBuffer[lindex] != '\0') && (lineBuffer[lindex] != '\n') && (lindex != LINE_BUFFER_SIZE)) { \
	numBuffer[nindex] = lineBuffer[lindex]; \
	nindex++; \
	lindex++; \
} \
	numBuffer[nindex] = '\0'; \
	tval = atoi(numBuffer);
#endif

// constructor / parser
Wavefront::Wavefront(const char* filename, const Transform *o2w, const Transform *w2o, bool reverseOrientation) :
Shape(o2w, w2o, reverseOrientation), vertexIndex(NULL),  p(NULL), n(NULL), uvs(NULL) {
	FILE* fin;
	fin = fopen(filename, "r");
	if (!fin) return;

	// temporary input buffers
	vector<Point> points;
	vector<int> verts;
	vector<int> normalIndex;	
	vector<int> uvIndex;

	vector<Normal> file_normals;
	vector<float> file_uvvector;

	Point ptmp;
	Normal ntmp;
	float uv1, uv2;

	char lineBuffer[LINE_BUFFER_SIZE];
	char numBuffer[32];
	int lindex=0;
	int nindex=0;
	int ival, uvval, nval;
	ntris=0;

	// parse the data in
	while (fgets(lineBuffer, LINE_BUFFER_SIZE, fin)) {
		switch (lineBuffer[0]) {
			case 'v':
				// case vertex information
				if (lineBuffer[1] == ' ') {
					// regular vertex point
					sscanf(&lineBuffer[2], "%f %f %f", &ptmp.x, &ptmp.y, &ptmp.z);
					points.push_back(ptmp);
				} else if (lineBuffer[1] == 't') {
					// texture coordinates
					sscanf(&lineBuffer[3], "%f %f", &uv1, &uv2);
					file_uvvector.push_back(uv1);
					file_uvvector.push_back(uv2);
				} else if (lineBuffer[1] == 'n') {
					// normal vector
					sscanf(&lineBuffer[2], "%f %f %f", &ntmp.x, &ntmp.y, &ntmp.z);
					file_normals.push_back(ntmp);
				}
				break;
			case 'f':
				// case face information
				lindex = 2;
				ntris++;
				for (int i=0; i < 3; i++) {

					GETNUM(lineBuffer, numBuffer, lindex, nindex, ival)

						// obj files go from 1..n, this just allows me to access the memory
						// directly by droping the index value to 0...(n-1)
						ival--;
					verts.push_back(ival);

					if (lineBuffer[lindex] == '/') {
						lindex++;
						GETNUM(lineBuffer, numBuffer, lindex, nindex, uvval)
							uvIndex.push_back(uvval-1);
					}

					if (lineBuffer[lindex] == '/') {
						lindex++;
						GETNUM(lineBuffer, numBuffer, lindex, nindex, nval)
							normalIndex.push_back(nval-1);
					}
					lindex++;
				}
				break;
			case 'g':
				// not really caring about faces or materials now
				// so just making life easier, I'll ignoring it
				break;
		}
	}

	fclose(fin);

	// merge everything back into one index array instead of multiple arrays
	MergeIndicies(points, file_normals, file_uvvector, verts, normalIndex, uvIndex);

	points.clear();
	file_normals.clear();
	file_uvvector.clear();
	verts.clear();
	normalIndex.clear();
	uvIndex.clear();

	if (n) std::cout << "Used normals" << std::endl;
	if (uvs) std::cout << "Used texture coords" << std::endl;

}

void Wavefront::MergeIndicies(vector<Point> &points, vector<Normal> &normals, vector<float> &uvVec, vector<int> &vIndex, vector<int> &normalIndex, vector<int> &uvIndex) {

	bool useNormals = !normals.empty();
	bool useUVs = !uvVec.empty();


	if (!useNormals && !useUVs) { 
		std::cout << "Copying points" << std::endl;
		// just copy the points into the array
		nverts = vIndex.size();
		p = new Point[points.size()];
		nVerticesTotal = points.size();
		for (unsigned int i=0; i < points.size(); i++)
			p[i] = points[i];
		vertexIndex = new int[nverts];
		for (int i=0; i < nverts; i++)
			vertexIndex[i] = vIndex[i];
		return;
	}

	// assumes that vertexIndex = normalIndex = uvIndex	
	nVerticesTotal = nverts = vIndex.size();				// FIX: Dec, 3
	vertexIndex = new int[nverts];

	p = new Point[nverts];
	if (useNormals) n = new Normal[nverts];
	if (useUVs) uvs = new float[nverts*2];

	for (int i=0; i < nverts; i++) {
		p[i] = points[vIndex[i]];
		if (useNormals) n[i] = normals[normalIndex[i]];
		if (useUVs) { 
			uvs[i*2] = uvVec[uvIndex[i]*2];
			uvs[i*2+1] = uvVec[uvIndex[i]*2 + 1];
		}
		vertexIndex[i] = i;
	}	
}

BBox Wavefront::ObjectBound() const {
	BBox bobj;
	for (int i = 0; i < nverts; i++)
		bobj = Union(bobj, (*WorldToObject)(p[i]));
	return bobj;
}

BBox Wavefront::WorldBound() const {
	BBox worldBounds;
	for (int i = 0; i < nverts; i++)
		worldBounds = Union(worldBounds, p[i]);
	return worldBounds;
}

// Refine
// generates the triangle mesh shape
void Wavefront::Refine(vector<Reference<Shape> > &refined) const {
	ParamSet paramSet;
	paramSet.AddInt("indices", vertexIndex, ntris*3);
	paramSet.AddPoint("P", p, nVerticesTotal);		
	if (n) paramSet.AddNormal("N", n, nVerticesTotal);
	if (uvs) paramSet.AddFloat("uv", uvs, nVerticesTotal*2);
	//bool ReverseOrientation
	refined.push_back(MakeShape("trianglemesh", ObjectToWorld,WorldToObject, ReverseOrientation, paramSet));
}

// CreateShape
// Only one parameter, filename, which directs the program
// to the relative path of the object file
//extern "C" DLLEXPORT 
Shape *CreateShape(const Transform *o2w, const Transform *w2o, bool reverseOrientation, const ParamSet &params) {		
	string filename = params.FindOneString("filename","");
	return new Wavefront(filename.c_str(), o2w, w2o,reverseOrientation);
}