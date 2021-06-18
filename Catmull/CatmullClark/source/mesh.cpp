#include "mesh.h"

// For each edge we will keep an adjecent faces indices
struct AdjFaces {
	// It is possible there are holes in the geometry
	// This is when an edge may have only one adjecent face
	int f1 = -1;
	int f2 = -1;
};

using EdgeAdjFacesMap = HashMap<int, AdjFaces>;
using VertAdjFacesMap = HashMap<int, Set<int>>; // for each vert -> set of face indecies
using VertAdjEdgesMap = VertAdjFacesMap; // for each vert -> set of edge indices
using FaceAdjEdgesMap = HashMap<int, int[4]>; // for each face -> 4 edge indices

struct Vec2iHasher {
	std::size_t operator()(const Vec2i &v) const {
		std::size_t h = std::size_t(v.x) << 32 + std::size_t(v.y);
		return std::hash<std::size_t>()(h);
	}
};

void findEdgesAndFacePoints(
	const Mesh &mesh,
	Vec<Vec3> &facePoints,
	Vec<Vec2i> &edges,
	VertAdjEdgesMap &vertAdjEdges,
	VertAdjFacesMap &vertAdjFaces,
	FaceAdjEdgesMap &faceAdjEdges,
	EdgeAdjFacesMap &edgeAdjFaces) {
	const auto &ps = mesh.ps;
	const auto &faces = mesh.faces;

	using Edge = Vec2i;
	using EdgeMap = HashMapCustom<Edge, int, Vec2iHasher>;
	EdgeMap edgesMap;

	for (int i = 0; i < faces.size(); ++i) {
		int adjEdgeIdx = 0;
		const auto &face = faces[i];

		Vec3 facePoint{ 0.f, 0.f, 0.f };
		for (int j = 0; j < Mesh::QUAD_FACE_VERTS; ++j) {
			facePoint += ps[face[j]];
			
			// Add face to the adjacency set of its verts
			vertAdjFaces[face[j]].insert(i);

			// Find new edges and update maps
			Edge edge;
			edge.x = face[j];
			edge.y = face[(j + 1) % Mesh::QUAD_FACE_VERTS];

			Edge reverseEdge{ edge.y, edge.x };

			EdgeMap::const_iterator it = edgesMap.find(edge);
			EdgeMap::const_iterator rit = edgesMap.find(reverseEdge);

			int eIdx = -1;
			if (it == edgesMap.end() && rit == edgesMap.end()) {
				edges.push_back(edge);
				eIdx = edges.size() - 1;
				edgesMap[edge] = eIdx;
			} else {
				eIdx = it == edgesMap.end() ? rit->second : it->second;
			}

			vertAdjEdges[edge.x].insert(eIdx);
			vertAdjEdges[edge.y].insert(eIdx);
			faceAdjEdges[i][adjEdgeIdx++] = eIdx;

			if (edgeAdjFaces[eIdx].f1 == -1) {
				edgeAdjFaces[eIdx].f1 = i;
			} else {
				edgeAdjFaces[eIdx].f2 = i;
			}
		}
		facePoints[i] = facePoint / float(Mesh::QUAD_FACE_VERTS);
	}
}

void findEdgePoints(
	const Mesh &mesh,
	const Vec<Vec3> &facePoints,
	const Vec<Vec2i> &edges,
	Vec<Vec2i> &newEdges,
	Vec<Vec3> &edgePoints,
	VertAdjEdgesMap &vertAdjEdges,
	EdgeAdjFacesMap &edgeAdjFaces) {
	const auto &ps = mesh.ps;
	const auto &faces = mesh.faces;
	
	const int n = ps.size();
	const int m = faces.size();
	const int k = edges.size();
	int neIdx = 0;

	for (int i = 0; i < edges.size(); ++i) {
		const auto edge = edges[i];

		// for each vertex of the edge, add the edge index to the adjecency set of that vertex
		vertAdjEdges[edge[0]].insert(i);
		vertAdjEdges[edge[1]].insert(i);

		// Find the middle point of the edge and average it with the average of the neigbouring faces' face points.
		const Vec3 edgeMiddle = (ps[edge[0]] + ps[edge[1]]) / 2.f;
		const int f1 = edgeAdjFaces[i].f1;
		const int f2 = edgeAdjFaces[i].f2;

		// These new edge points will be added to the ps array, after we add the new face points.
		// So the old ps array has size n, after we add the face points its size becomes n + m. Thus:
		const int edgeMiddleIdx = n + m + i;
		// Connect middle point to vertices of the edge
		newEdges[neIdx++] = { edgeMiddleIdx, edge[0] };
		newEdges[neIdx++] = { edgeMiddleIdx, edge[1] };

		// Connect middle point to the neighbouring facepoints
		if (f1 >= 0) {
			newEdges[neIdx++] = { edgeMiddleIdx, n + f1 };
		}
		if (f2 >= 0) {
			newEdges[neIdx++] = { edgeMiddleIdx, n + f2 };
		}

		// If edge is on a border of a hole we use the middle point as an edge point
		if (f1 == -1 || f2 == -1) {
			edgePoints[i] = edgeMiddle;
			continue;
		}

		const Vec3 facesMiddle = (facePoints[f1] + facePoints[f2]) / 2.f;
		edgePoints[i] = (edgeMiddle + facesMiddle) / 2.f;
	}
}

void updatePoints(
	const VertAdjFacesMap &vertAdjFaces,
	const VertAdjEdgesMap &vertAdjEdges,
	const EdgeAdjFacesMap &edgeAdjFaces,
	const Vec<Vec3> &facePoints,
	const Vec<Vec2i> &edges,
	Vec<Vec3> &ps) {

	// TODO: check if vertex is on a border of a hole
	for (int i = 0; i < ps.size(); ++i) {
		const Set<int>& adjFaces = vertAdjFaces.find(i)->second;
		const Set<int>& adjEdges = vertAdjEdges.find(i)->second;

		if (adjEdges.size() == adjFaces.size()) {
			Vec3 avgFacesPoint{ 0.f, 0.f, 0.f };
			for (const auto& v : adjFaces) {
				avgFacesPoint += facePoints[v];
			}
			avgFacesPoint /= float(adjFaces.size());

			Vec3 avgEdgesPoint{ 0.f, 0.f, 0.f };
			for (const auto& v : adjEdges) {
				avgEdgesPoint += (ps[edges[v][0]] + ps[edges[v][1]]) / 2.f;
			}
			avgEdgesPoint /= float(adjEdges.size());

			const int n = adjEdges.size();
			ps[i] = (ps[i] * float(n - 3) + avgFacesPoint + 2.f * avgEdgesPoint) / float(n);
		} else { // vertex is on the border of a hole
			int n = 0;
			Vec3 newPoint = ps[i];
			for (const auto& v : adjEdges) {
				const AdjFaces& af = edgeAdjFaces.find(v)->second;
				if (af.f1 >= 0 && af.f2 >= 0) {
					continue;
				}
				newPoint += (ps[edges[v][0]] + ps[edges[v][1]]) / 2.f;
				++n;
			}
			ps[i] = newPoint / float(n + 1);
		}
	}
}

void generateNewFaces(
	const Vec<Vec4i> &faces,
	const FaceAdjEdgesMap &faceAdjEdges,
	int vertCount,
	Vec<Vec4i> &newFaces) {
	const int n = vertCount;
	const int m = faces.size();

	int nfIdx = 0;
	for (int i = 0; i < faces.size(); ++i) {
		const auto face = faces[i];
		const int a = face[0];
		const int b = face[1];
		const int c = face[2];
		const int d = face[3];

		const auto &adjEdges = faceAdjEdges.find(i)->second;
		// indices of the edge points in the updated ps array
		const int edgeAB = n + m + adjEdges[0];
		const int edgeBC = n + m + adjEdges[1];
		const int edgeCD = n + m + adjEdges[2];
		const int edgeDA = n + m + adjEdges[3];

		// keep the order of the points in the new faces, since we'll need it for the triangulation
		const int fp = n + i;
		newFaces[nfIdx++] = { a, edgeAB, fp, edgeDA };
		newFaces[nfIdx++] = { edgeAB, b, edgeBC, fp };
		newFaces[nfIdx++] = { fp, edgeBC, c, edgeCD };
		newFaces[nfIdx++] = { edgeDA, fp, edgeCD, d };
	}
}

void Mesh::subdivide() {
	EdgeAdjFacesMap edgeAdjFaces;
	VertAdjFacesMap vertAdjFaces;
	VertAdjEdgesMap vertAdjEdges;
	FaceAdjEdgesMap faceAdjEdges;

	Vec<Vec2i> edges;
	Vec<Vec3> facePoints(faces.size());
	findEdgesAndFacePoints(*this, facePoints, edges, vertAdjEdges, vertAdjFaces, faceAdjEdges, edgeAdjFaces);

	Vec<Vec2i> newEdges(2 * edges.size() + 4 * faces.size());
	Vec<Vec3> edgePoints(edges.size());
	findEdgePoints(*this, facePoints, edges, newEdges, edgePoints, vertAdjEdges, edgeAdjFaces);
	
	updatePoints(vertAdjFaces, vertAdjEdges, edgeAdjFaces, facePoints, edges, ps);

	const int n = ps.size(); // save old vertex count for later use
	ps.insert(ps.end(), facePoints.cbegin(), facePoints.cend());
	ps.insert(ps.end(), edgePoints.cbegin(), edgePoints.cend());

	Vec<Vec4i> newFaces(faces.size() * QUAD_FACE_VERTS);
	generateNewFaces(faces, faceAdjEdges, n, newFaces);
	
	edges = newEdges;
	faces = newFaces;
	subdivided = true;

	triangulate();
}

Vec<Vec3>& Mesh::points() {
	return ps;
}

const Vec<Vec3>& Mesh::points() const {
	return ps;
}

void Mesh::triangulate() {
	if (faces.empty()) {
		return;
	}

	triFaces.clear();
	triFaces.resize(faces.size() * 2);
	for (int i = 0; i < faces.size(); ++i) {
		auto f = faces[i];
		triFaces[2 * i + 0] = { f[0], f[1], f[2] };
		triFaces[2 * i + 1] = { f[0], f[2], f[3] };
	}
}

/* 
============================================================================================
 Default Mesh
============================================================================================
*/
Mesh* newDefaultCube() {
	unsigned short* data = (unsigned short*)malloc(sizeof(Mesh) + sizeof(unsigned short));
	data[0] = (unsigned short)0xBEBE; // indicate the mesh was created with this constructor
	Mesh* cube = new (data + 1) Mesh;

	cube->ps = {
		{ -0.5f, -0.5f,  0.5f }, // 0
		{  0.5f, -0.5f,  0.5f }, // 1
		{  0.5f,  0.5f,  0.5f }, // 2
		{ -0.5f,  0.5f,  0.5f }, // 3

		{ -0.5f, -0.5f, -0.5f }, // 4
		{  0.5f, -0.5f, -0.5f }, // 5
		{  0.5f,  0.5f, -0.5f }, // 6
		{ -0.5f,  0.5f, -0.5f }, // 7
	};

	cube->faces = {
		{ 0, 1, 2, 3 }, // 0
		{ 1, 5, 6, 2 }, // 1
		{ 4, 5, 1, 0 }, // 2
		{ 3, 2, 6, 7 }, // 3
		{ 5, 4, 7, 6 }, // 4
		{ 4, 0, 3, 7 }, // 5
	};

	cube->triangulate();

	return cube;
}

bool deleteDefaultCube(Mesh *&cube) {
	unsigned short* data = reinterpret_cast<unsigned short*>(cube);
	data = data - 1;
	if (data[0] != 0xBEBE) {
		return false;
	}

	free(data);
	cube = nullptr;
	return true;
}