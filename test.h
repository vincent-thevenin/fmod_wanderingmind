#ifndef _AUDIO_ENGINE_H_
#define _AUDIO_ENGINE_H_

#include "fmod.hpp"
#include "fmod_errors.h"
#include "vendors/json.hpp"
#include "vendors/nanoflann.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <string>
#include <vector>

#include <sio_client.h>

using json = nlohmann::json;


struct data
{
    std::vector<float> embedding; // coordinate of audio chunk (currently 2D)
    std::string path; // path to file
    int chunk; // chunk number in file
};

template <typename T>
struct PointCloud
{
	struct Point
	{
		T  x,y;
	};

	std::vector<Point>  pts;

	// Must return the number of data points
	inline size_t kdtree_get_point_count() const { return pts.size(); }

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline T kdtree_get_pt(const size_t idx, const size_t dim) const
	{
		if (dim == 0) return pts[idx].x;
		else return pts[idx].y;
	}

	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }

};

#endif