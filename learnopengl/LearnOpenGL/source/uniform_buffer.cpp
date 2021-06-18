#include "uniform_buffer.h"

#include "common_headers.h"

UniformBuffer::UniformBuffer() : UBO(-1), size(0), offset(0) { }

UniformBuffer::~UniformBuffer() {
	deinit();
}

void UniformBuffer::init(int dataSize) {
	deinit();

	size = dataSize;

	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::deinit() {
	glDeleteBuffers(1, &UBO);
	UBO = -1;
	size = 0;
	offset = 0;
}

void UniformBuffer::subData(const void *data, int size) {
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	
	offset += size;

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::subData(const void *data, int offset, int size) {
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::bind(int idx) const {
	assert(UBO > 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, idx, UBO);
	// We bind at the beging of the frame. At this point the buffer's data must have been set.
	offset = 0;
}

void UniformBuffer::bind(int idx, int offset, int size) const {
	assert(UBO > 0 && size - offset + 1 <= this->size);
	glBindBufferRange(GL_UNIFORM_BUFFER, idx, UBO, offset, size);
	this->offset = 0;
}
