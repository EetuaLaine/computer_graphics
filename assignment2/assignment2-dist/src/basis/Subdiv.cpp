#include "App.hpp"
#include "base/Main.hpp"
#include "gpu/GLContext.hpp"
#include "3d/Mesh.hpp"
#include "io/File.hpp"
#include "io/StateDump.hpp"
#include "base/Random.hpp"

#include "Subdiv.hpp"

#include <stdio.h>
#include <conio.h>

#include <vector>
#include <map>

using namespace FW;

namespace FW {

void MeshWithConnectivity::fromMesh( const Mesh<VertexPNC>& m )
{
	positions.resize(m.numVertices());
	normals.resize(m.numVertices());
	colors.resize(m.numVertices());

	for (int i = 0; i < m.numVertices(); ++i) {
		positions[i] = m.vertex(i).p;
		normals[i] = m.vertex(i).n;
		colors[i] = m.vertex(i).c.getXYZ();
	}

	indices.reserve(m.numTriangles());

	// move indices
	for (int i = 0; i < m.numSubmeshes(); ++i)
		for (int t = 0; t < m.indices(i).getSize(); ++t)
			indices.push_back(m.indices(i)[t]);

	computeConnectivity();
}

// assumes vertices and indices are already filled in.
void MeshWithConnectivity::computeConnectivity()
{
	// assign default values. boundary edges (no neighbor on other side) are denoted by -1.
	neighborTris.assign(indices.size(), Vec3i(-1,-1,-1));
	neighborEdges.assign(indices.size(), Vec3i(-1,-1,-1));

	// bookkeeping: map edges (vert0, vert1) to (triangle, edge_number) pairs
	typedef std::map<std::pair<int, int>, std::pair<int, int>> edgemap_t;
	edgemap_t M;

	for (int i = 0; i < (int)indices.size(); ++i) {
		// vertex index is also an index for the corresponding edge starting at that vertex
		for (int j = 0; j < 3; ++j) {
			int v0 = indices[i][j];
			int v1 = indices[i][(j+1)%3];
			auto it = M.find(std::make_pair(v1, v0));
			if (it == M.end()) {
				// edge not found, add myself to mapping
				// (opposite direction than when finding because we look for neighbor edges)
				M[std::make_pair(v0, v1)] = std::make_pair(i, j);
			} else {
				if (it->second.first == -1)	{
					FW::printf( "Non-manifold edge detected\n" );
				} else {
					// other site found, let's fill in the data
					int other_t = it->second.first;
					int other_e = it->second.second;

					neighborTris[i][j] = other_t;
					neighborEdges[i][j] = other_e;

					neighborTris[other_t][other_e] = i;
					neighborEdges[other_t][other_e] = j;

					it->second.first = -1;
				}
			}
		}
	}
	
}

// Run a debug version of the subdivision pass where we only subdivide the one triangle
// that's under the mouse cursor. Returns a list of positions that need to be drawn by App
std::vector<Vec3f> MeshWithConnectivity::debugHighlight(Vec2f mousePos, Mat4f worldToClip)
{
	Vec2i closestIdx = -1;
	float minCost = 1e9;

	// loop through vertices and find the one that's closest to our mouse click
	for (int i = 0; i < indices.size(); ++i)
		for(int j = 0; j < 3; ++j)
		{
			int idx = indices[i][j];
			Vec4f clip = worldToClip * Vec4f(positions[idx], 1.0f);
			Vec3f clipPos = clip.getXYZ() / clip.w;
			float depth = clip.w;

			// use a cost function that prefers points that are closer to camera
			float dist = (clipPos.getXY() - mousePos).length();
			float cost = dist + depth * .01f;

			if (cost < minCost)
			{
				minCost = cost;
				closestIdx = Vec2i(i, j);
			}
		}

	// If we found no valid vertices, return
	if (closestIdx.x == -1)
	{
		std::cout << "no vertices found under mouse position, aborting debug!\n";
		return std::vector<Vec3f>();
	}

	// clear debug data from previous calls
	highlightIndices.clear();

	// Call subdivision with the debugPass flag on to get debug data
	debugPass = true;
	debugVertexIdx = closestIdx;
	LoopSubdivision();

	// Set flag to false so we can run actual subdivision later
	debugPass = false;

	// create position vector out of highlight indices
	std::vector<Vec3f> debugPoints;
	for (auto& idx : highlightIndices)
	{
		Vec3f pos = positions[idx];
		Vec3f n = normals[idx];

		pos += n * .001f;
		debugPoints.push_back(pos);
	}

	// return debug data so that App can draw it
	return debugPoints;
}

void MeshWithConnectivity::toMesh(Mesh<VertexPNC>& dest) {
	dest.resetVertices((int)positions.size());
	for (size_t i = 0; i < positions.size(); ++i) {
		dest.mutableVertex((int)i).p = positions[i];
		dest.mutableVertex((int)i).n = normals[i];
		dest.mutableVertex((int)i).c = Vec4f(colors[i], 1.0f);
	}
	dest.resizeSubmeshes(1);
	dest.mutableIndices(0).replace(0, dest.indices(0).getSize(), &indices[0], (int)indices.size());
}

void MeshWithConnectivity::LoopSubdivision() {
	// generate new (odd) vertices

	// visited edge -> vertex position information
	// Note that this is different from the one in computeConnectivity()
	typedef std::map<std::pair<int, int>, int> edgemap_t;
	edgemap_t new_vertices;

	// The new data must be doublebuffered or otherwise some of the calculations below would
	// not read the original positions but the newly changed ones, which is slightly wrong.
	std::vector<Vec3f> new_positions(positions.size());
	std::vector<Vec3f> new_normals(normals.size());
	std::vector<Vec3f> new_colors(colors.size());

	// If we're debugging, skip this part since we're only interested in the 1-ring portion. Feel free to change this if you need to.
	if (!debugPass)
	{
		for (size_t i = 0; i < indices.size(); ++i)
			for (int j = 0; j < 3; ++j) {
				int v0 = indices[i][j];
				int v1 = indices[i][(j + 1) % 3];

				// Map the edge endpoint indices to new vertex index.
				// We use min and max because the edge direction does not matter when we finally
				// rebuild the new faces (R3); this is how we always get unique indices for the map.
				auto edge = std::make_pair(min(v0, v1), max(v0, v1));

				// With naive iteration, we would find each edge twice, because each is part of two triangles
				// (if the mesh does not have any holes/empty borders). Thus, we keep track of the already
				// visited edges in the new_vertices map. That requires the small R3 task below in the 'if' block.
				if (new_vertices.find(edge) == new_vertices.end()) {
					// YOUR CODE HERE (R4): compute the position for odd (= new) vertex.
					// You will need to use the neighbor information to find the correct vertices and then combine the four corner vertices with the correct weights.
					// Be sure to see section 3.2 in the handout for an in depth explanation of the neighbor index tables; the scheme is somewhat involved.
					Vec3f pos, col, norm;
					// This default implementation just puts the new vertex at the edge midpoint.
					int neighbor_triangle = neighborTris[i][j];

					// neighbor_vertex is the vertex index of the target vertex in the neighbor triangle (0 to 2).
					int neighbor_edge = neighborEdges[i][j];

					int neighbor_vertex = (neighbor_edge + 2) % 3;

					// Vertex index of the relevant vertex at the side of the current triangle.
					int this_triangle_vertex = (j + 2) % 3;

					// 3/8 weight for these.
					Vec3f p0 = positions[v0];
					Vec3f p1 = positions[v1];

					int this_vertex = indices[i][this_triangle_vertex];
					int neighbor_vertex_index;
					Vec3f p2;
					Vec3f c2;
					Vec3f n2;
					float neighbor_weight;

					// 1/8 weight for these.
					if (neighbor_triangle != -1 && neighbor_edge != -1) {
						neighbor_vertex_index = indices[neighbor_triangle][neighbor_vertex];
						p2 = positions[neighbor_vertex_index];
						c2 = colors[neighbor_vertex_index];
						n2 = normals[neighbor_vertex_index];
						neighbor_weight = (1.0f / 8);
					}
					else {
						p2 = Vec3f(0.0f, 0.0f, 0.0f);
						c2 = Vec3f(0.0f, 0.0f, 0.0f);
						n2 = Vec3f(0.0f, 0.0f, 0.0f);
						neighbor_weight = (2.0f / 8);
					}

					Vec3f p3 = positions[this_vertex];

					pos = (3.0f / 8) * (p0 + p1) + neighbor_weight * (p2 + p3);
					col = (3.0f / 8) * (colors[v0] + colors[v1]) + neighbor_weight * (c2 + colors[this_vertex]);
					norm = (3.0f / 8) * (normals[v0] + normals[v1]) + neighbor_weight * (n2 + normals[this_vertex]);

				new_positions.push_back(pos);
				new_colors.push_back(col);
				new_normals.push_back(norm);

				// YOUR CODE HERE (R3):
				// Map the edge to the correct vertex index.
				// This is just one line! Use new_vertices and the index of the position that was just pushed back to the vector.
				new_vertices.emplace(edge, new_positions.size() - 1);
				}
			}
	}
	// compute positions for even (old) vertices
	std::vector<bool> vertexComputed(new_positions.size(), false);

	for (int i = 0; i < (int)indices.size(); ++i) {
		for (int j = 0; j < 3; ++j) {
			int v0 = indices[i][j];

			// If we're doing the debug pass, set vertex index to the one under mouse position
			if (debugPass)
			{
				i = debugVertexIdx.x;
				j = debugVertexIdx.y;
				v0 = indices[i][j];
			}

			// don't redo if this one is already done
			if (vertexComputed[v0] && !debugPass)
				continue;

			vertexComputed[v0] = true;

			Vec3f pos, col, norm;
			// YOUR CODE HERE (R5): reposition the old vertices

			// TODO: GO AROUND THE NEIGHBOR TRIANGLES LIKE A CAROUSEL UNTIL GETTING BACK TO THE ORIGINAL ONE

			// This default implementation just passes the data through unchanged.
			// You need to replace these three lines with the loop over the 1-ring
			// around vertex v0, and compute the new position as a weighted average
			// of the other vertices as described in the handout.

			// If you're having a difficult time, you can try debugging your implementation
			// with the debug highlight mode. If you press alt, LoopSubdivision will be called
			// for only the vertex under your mouse cursor, which should help with debugging.
			// You can also push vertex indices into the highLightIndices vector to draw the
			// vertices with a visible color, so you can ensure that the 1-ring generated is correct.
			// The solution exe implements this so you can see an example of what you can do with the
			// highlight mode there.

			int anchor_point = indices[i][(j + 1) % 3];
			int v1 = -2;
			float B = 3.0f / 16;
			std::vector<Vec3f> neighbor_positions;
			std::vector<Vec3f> neighbor_colors;
			std::vector<Vec3f> neighbor_normals;
			Vec3f p1 = positions[v0];
			Vec3f c1 = colors[v0];
			Vec3f n1 = normals[v0];

			v1 = indices[i][(j + 1) % 3];

			neighbor_positions.push_back(positions[v1]);
			neighbor_colors.push_back(colors[v1]);
			neighbor_normals.push_back(normals[v1]);

			int border_edge = (j + 2) % 3;
			int current_triangle = neighborTris[i][border_edge];
			int current_edge = neighborEdges[i][border_edge];


			while (current_edge != -1 && current_triangle != -1 && current_triangle != i) {
				v1 = indices[current_triangle][(current_edge + 1) % 3];
				neighbor_positions.push_back(positions[v1]);
				neighbor_colors.push_back(colors[v1]);
				neighbor_normals.push_back(normals[v1]);
				border_edge = (current_edge + 2) % 3;
				current_edge = neighborEdges[current_triangle][border_edge];
				current_triangle = neighborTris[current_triangle][border_edge];
			}


			if (current_triangle == i) 
			{
				int n = neighbor_positions.size();
				if (n > 3)
					B = 3 / (8.0f * n);

				for (int k = 0; k < n; k++)
				{
					pos += neighbor_positions[k];
					col += neighbor_colors[k];
					norm += neighbor_colors[k];
				}
				pos *= B;
				col *= B;
				norm *= B;

				pos += (1 - n * B) * p1;
				col += (1 - n * B) * c1;
				norm += (1 - n * B) * n1;
			}
			else 
			{
				pos = positions[v0];
				col = colors[v0];
				norm = normals[v0];
			}


			// Stop here if we're doing the debug pass since we don't actually need to modify the mesh
			if (debugPass)
				return;

			new_positions[v0] = pos;
			new_colors[v0] = col;
			new_normals[v0] = norm;
		}
	}

	// Again, if we're doing the debug pass, we only care about our 1-ring so we can stop now
	if (debugPass)
		return;

	// and then, finally, regenerate topology
	// every triangle turns into four new ones
	std::vector<Vec3i> new_indices;
	new_indices.reserve(indices.size()*4);
	for (size_t i = 0; i < indices.size(); ++i) {
		Vec3i even = indices[i]; // start vertices of e_0, e_1, e_2

		// YOUR CODE HERE (R3):
		// fill in X and Y (it's the same for both)
		auto edge_a = std::make_pair(min(even[0], even[1]), max(even[0], even[1]));
		auto edge_b = std::make_pair(min(even[1], even[2]), max(even[1], even[2]));
		auto edge_c = std::make_pair(min(even[2], even[0]), max(even[2], even[0]));


		// The edges edge_a, edge_b and edge_c now define the vertex indices via new_vertices.
		// (The mapping is done in the loop above.)
		// The indices define the smaller triangle inside the indices defined by "even", in order.
		// Read the vertex indices out of new_vertices to build the small triangle "odd"

		Vec3i odd = Vec3i(new_vertices.find(edge_a)->second, 
						  new_vertices.find(edge_b)->second, 
			              new_vertices.find(edge_c)->second);

		// Then, construct the four smaller triangles from the surrounding big triangle  "even"
		// and the inner one, "odd". Push them to "new_indices".

		// NOTE: REMOVE the following line after you're done with the new triangles.
		// This just keeps the mesh intact and serves as an example on how to add new triangles.
		new_indices.push_back( Vec3i( odd[0], odd[1], odd[2] ) );
		new_indices.push_back( Vec3i( odd[0], odd[1], even[1] ) );
		new_indices.push_back( Vec3i( odd[1], odd[2], even[2] ) );
		new_indices.push_back( Vec3i( odd[2], odd[0], even[0] ) );
	}

	// ADD THESE LINES when R3 is finished. Replace the originals with the repositioned data.
	indices = std::move(new_indices);
	positions = std::move(new_positions);
	normals = std::move(new_normals);
	colors = std::move(new_colors);
}

} // namespace FW
