#ifndef CAMERA_H
#define CAMERA_H

struct Camera {

	void init(/* ... */);
	float FOV() const { return 90.f; }
};

#endif // CAMERA_H