#pragma once

#include "common_defines.h"

struct UniformBuffer {
	UniformBuffer();
	~UniformBuffer();

	void init(int dataSize);
	void deinit();
	void subData(const void *data, int size);
	void subData(const void *data, int offset, int size);
	void bind(int idx) const;
	void bind(int idx, int offset, int size) const;

private:
	Handle UBO;
	Handle size;
	mutable int offset;
};
