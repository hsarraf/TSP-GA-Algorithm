#ifndef __GRAPH_H__
#define __GRAPH_H__


#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
using namespace std;

typedef short int S_INT;

extern int __gVtxCount;

// class of vertices
//
class GEdgeVertex {
public:
	GEdgeVertex() : _vid(-1) {};
	GEdgeVertex(int vid, int x, int y) : _vid(vid), _x(x), _y(y) {};
	S_INT _vid;				// vertex Id
	S_INT _x;				// x position
	S_INT _y;				// y position
}*__GVerices = NULL;	// array of verices, cities



// class of graph path 
//
class GPath {
public:
	GPath() : _path(NULL), _length(0.0) {}
	GEdgeVertex *_path;			// tha path, vertices array, associated to this graph path
	float _length;				// length of the path

	void updateLength();		// method to update the length of this path
} **__toursPopulationList = NULL, *__bestTour = NULL;	// population list array, and best tour

// update the length of this path, tour indeed
//
void GPath::updateLength() {
	if (_path) {
		_length = 0.0;
		for (int i = 0; i < __gVtxCount; i++)
			_length += (float)sqrt(pow(_path[i]._x - _path[(i + 1) % __gVtxCount]._x, 2) + pow(_path[i]._y - _path[(i + 1) % __gVtxCount]._y, 2));
	}
}



#endif /*__GRAPH_H__*/