#ifndef MY_CAMERA_H
#define MY_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MyCamera {
public:
	MyCamera() {};
	MyCamera(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up): 
		_eye(eye), _center(center), _up(up) {
		_rotate = glm::mat4(1.0f);
	}

	inline glm::vec3 Eye() const { return _rotate*_eye; }
	inline glm::vec3 Center() const { return _center; }
	inline glm::vec3 Up() const { return _rotate*_up; }

	glm::mat4 Mat() const { return glm::lookAt(_rotate*_eye , _center, _rotate*_up); }

	// move further or closer: >0 further, < 0 closer
	inline void Move(float dist) { _eye += glm::normalize(_eye - _center)*dist; }

	/** rotate the camera
	@param dx: rotate along up 
	@param dy: rotate along the left direction
	*/
	void Rotate(float angleX, float angleY) {
		glm::mat4 m_rotate(_rotate);
		m_rotate = glm::rotate(m_rotate, angleX, _up);
		m_rotate = glm::rotate(m_rotate, angleY, glm::cross(_up, _center - _eye));
		_rotate = m_rotate;
	}

private:
	glm::vec3 _eye, _center, _up;		// center should be (0, 0, 0)
	glm::mat3 _rotate;
};
#endif // !MY_CAMERA_H
