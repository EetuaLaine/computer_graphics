#define _CRT_SECURE_NO_WARNINGS

#include "App.hpp"

#include "utility.hpp"
#include "base/Main.hpp"
#include "gpu/GLContext.hpp"
#include "gpu/Buffer.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>

using namespace FW;
using namespace std;

// Anonymous namespace. This is a C++ way to define things which
// do not need to be visible outside this file.
namespace {

enum VertexShaderAttributeLocations {
	ATTRIB_POSITION = 0,
	ATTRIB_NORMAL = 1,
	ATTRIB_COLOR = 2
};

const Vertex reference_plane_data[] = {
	{ Vec3f(-1, -1, -1), Vec3f(0, 1, 0) },
	{ Vec3f( 1, -1, -1), Vec3f(0, 1, 0) },
	{ Vec3f( 1, -1,  1), Vec3f(0, 1, 0) },
	{ Vec3f(-1, -1, -1), Vec3f(0, 1, 0) },
	{ Vec3f( 1, -1,  1), Vec3f(0, 1, 0) },
	{ Vec3f(-1, -1,  1), Vec3f(0, 1, 0) }
};

vector<Vertex> loadExampleModel() {
	static const Vertex example_data[] = {
		{ Vec3f( 0.0f,  0.5f, 0), Vec3f(0.0f, 0.0f, -1.0f) },
		{ Vec3f(-0.5f, -0.5f, 0), Vec3f(0.0f, 0.0f, -1.0f) },
		{ Vec3f( 0.5f, -0.5f, 0), Vec3f(0.0f, 0.0f, -1.0f) }
	};
	vector<Vertex> vertices;
	for (auto v : example_data)
		vertices.push_back(v);
	return vertices;
}

vector<Vertex> unpackIndexedData(
	const vector<Vec3f>& positions,
	const vector<Vec3f>& normals,
	const vector<array<unsigned, 6>>& faces)
{
	vector<Vertex> vertices;

	// This is a 'range-for' loop which goes through all objects in the container 'faces'.
	// '&' gives us a reference to the object inside the container; if we omitted '&',
	// 'f' would be a copy of the object instead.
	// The compiler already knows the type of objects inside the container, so we can
	// just write 'auto' instead of having to spell out 'array<unsigned, 6>'.
	for (auto& f : faces) {

		// YOUR CODE HERE (R3)
		// Unpack the indexed data into a vertex array. For every face, you have to
		// create three vertices and add them to the vector 'vertices'.

		// f[0] is the index of the position of the first vertex
		// f[1] is the index of the normal of the first vertex
		// f[2] is the index of the position of the second vertex
		// ...
		Vertex v0; Vertex v1; Vertex v2;
		v0.position = positions[f[0]]; v0.normal = normals[f[1]];
		v1.position = positions[f[2]]; v1.normal = normals[f[3]];
		v2.position = positions[f[4]]; v2.normal = normals[f[5]];
		vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2);
	}

	return vertices;
};

// This is for testing your unpackIndexedData implementation.
// You should get a tetrahedron like in example.exe.
vector<Vertex> loadIndexedDataModel() {
	static const Vec3f point_data[] = {
		Vec3f(0.0f, 0.407f, 0.0f),
		Vec3f(0.0f, -0.3f, -0.5f),
		Vec3f(0.433f, -0.3f, 0.25f),
		Vec3f(-0.433f, -0.3f, 0.25f),
	};
	static const Vec3f normal_data[] = {
		Vec3f(0.8165f, 0.3334f, -0.4714f),
		Vec3f(0.0f, 0.3334f, 0.9428f),
		Vec3f(-0.8165f, 0.3334f, -0.4714f),
		Vec3f(0.0f, -1.0f, 0.0f)
	};
	static const unsigned face_data[][6] = {
		{0, 0, 1, 0, 2, 0},
		{0, 2, 3, 2, 1, 2},
		{0, 1, 2, 1, 3, 1},
		{1, 3, 3, 3, 2, 3}
	};
	vector<Vec3f> points(point_data, point_data + SIZEOF_ARRAY(point_data));
	vector<Vec3f> normals(normal_data, normal_data + SIZEOF_ARRAY(normal_data));
	vector<array<unsigned, 6>> faces;
	for (auto arr : face_data) {
		array<unsigned, 6> f;
		copy(arr, arr+6, f.begin());
		faces.push_back(f);
	}
	return unpackIndexedData(points, normals, faces);
}

// Generate an upright cone with tip at (0, 0, 0), a radius of 0.25 and a height of 1.0.
// You can leave the base of the cone open, like it is in example.exe.
vector<Vertex> loadUserGeneratedModel() {
	static const float radius = 0.25f;
	static const float height = 1.0f;
	static const unsigned faces = 40;
	static const float angle_increment = 2 * FW_PI / faces;

	// Empty array of Vertex structs; every three vertices = one triangle
	vector<Vertex> vertices;
	
	Vertex v0, v1, v2;

	// Generate one face at a time
	for(auto i = 0u; i < faces; ++i)	{
		// YOUR CODE HERE (R2)
		// Figure out the correct positions of the three vertices of this face.
		// v0.position = ...
		// Calculate the normal of the face from the positions and use it for all vertices.
		// v0.normal = v1.normal = v2.normal = ...;
		//
		// Some hints:
		// - Try just making a triangle in fixed coordinates at first.
		// - "FW::cos(angle_increment * i) * radius" gives you the X-coordinate
		//    of the ith vertex at the base of the cone. Z-coordinate is very similar.
		// - For the normal calculation, you'll want to use the cross() function for
		//   cross product, and Vec3f's .normalized() or .normalize() methods.
		
		// Then we add the vertices to the array.
		// .push_back() grows the size of the vector by one, copies its argument,
		// and places the copy at the back of the vector.
		v0.position = Vec3f(FW::cos(angle_increment * i) * radius, -height, FW::sin(angle_increment * i) * radius);
		v1.position = Vec3f(FW::cos(angle_increment * (i + 1)) * radius, -height, FW::sin(angle_increment * (i + 1)) * radius);
		v2.position = Vec3f(0, 0, 0);

		Vec3f norm = FW::cross(v1.position, v0.position).normalized();

		v0.normal = norm; v1.normal = norm; v2.normal = norm;

		vertices.push_back(v0); vertices.push_back(v1); vertices.push_back(v2);
	}
	return vertices;
}

}

App::App(void)
:   common_ctrl_			(CommonControls::Feature_Default & ~CommonControls::Feature_RepaintOnF5),
	current_model_			(MODEL_EXAMPLE),
	model_changed_			(true),
	shading_toggle_			(false),
	shading_mode_changed_	(false),
	camera_rotation_angle_	(0.0f),
	translation_            (Mat4f()),
	object_rotator_			(Mat4f()),
	object_rotation_angle_  (0.0f),
	mouse_delta_			(Vec2i()),
	camera_distance_		(2.1f),
	key_pressed_			(false)
{
	static_assert(is_standard_layout<Vertex>::value, "struct Vertex must be standard layout to use offsetof");
	initRendering();
	
	common_ctrl_.showFPS(true);
	common_ctrl_.addToggle((S32*)&current_model_, MODEL_EXAMPLE,			FW_KEY_1, "Triangle (1)",				&model_changed_);
	common_ctrl_.addToggle((S32*)&current_model_, MODEL_USER_GENERATED,		FW_KEY_2, "Generated cone (2)",			&model_changed_);
	common_ctrl_.addToggle((S32*)&current_model_, MODEL_FROM_INDEXED_DATA,	FW_KEY_3, "Unpacked tetrahedron (3)",	&model_changed_);
	common_ctrl_.addToggle((S32*)&current_model_, MODEL_FROM_FILE,			FW_KEY_4, "Model loaded from file (4)",	&model_changed_);
	// common_ctrl_.addToggle((S32*)&current_model_, MODEL_FROM_PLY_FILE,		FW_KEY_5, "Model loaded from PLY file(5)", &model_changed_);
	common_ctrl_.addSeparator();
	common_ctrl_.addToggle(&shading_toggle_,								FW_KEY_T, "Toggle shading mode (T)",	&shading_mode_changed_);

	window_.setTitle("Assignment 1");

	window_.addListener(this);
	window_.addListener(&common_ctrl_);

	window_.setSize( Vec2i(800, 800) );
}

void App::streamGeometry(const std::vector<Vertex>& vertices) {
	// Load the vertex buffer to GPU.
	glBindBuffer(GL_ARRAY_BUFFER, gl_.dynamic_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	vertex_count_ = vertices.size();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool App::handleEvent(const Window::Event& ev) {
	if (model_changed_)	{
		model_changed_ = false;

		switch (current_model_)
		{
		case MODEL_EXAMPLE:
			streamGeometry(loadExampleModel());
			break;
		case MODEL_FROM_INDEXED_DATA:
			streamGeometry(loadIndexedDataModel());
			break;
		case MODEL_USER_GENERATED:
			streamGeometry(loadUserGeneratedModel());
			break;
		case MODEL_FROM_FILE:
			{
				auto filename = window_.showFileLoadDialog("Load new mesh");
				if (filename.getLength()) {
					streamGeometry(loadObjFileModel(filename.getPtr()));
				} else {
					current_model_ = MODEL_EXAMPLE;
					model_changed_ = true;
				}
			}
			break;
		/*
		case MODEL_FROM_PLY_FILE:
			{
				auto filename = window_.showFileLoadDialog("Load new mesh");
				if (filename.getLength()) {
					streamGeometry(loadPLYFileModel(filename.getPtr()));
				}
				else {
					current_model_ = MODEL_EXAMPLE;
					model_changed_ = true;
				}
			}
			break;
		*/
		default:
			assert(false && "invalid model type");
		}
	}
	
	if (shading_mode_changed_) {
		common_ctrl_.message(shading_toggle_ ?
			"Directional light shading using vertex normals; direction to light (0.5, 0.5, -0.6)" :
			"High visibility shading, color from vertex ID");
		shading_mode_changed_ = false;
	}


	if (ev.type == Window::EventType_KeyDown) {
		// YOUR CODE HERE (R1)
		// React to user input and move the model.
		// Look in framework/gui/Keys.hpp for more key codes.
		// Visual Studio tip: you can right-click an identifier like FW_KEY_HOME
		// and "Go to definition" to jump directly to where the identifier is defined.
		if (ev.key == FW_KEY_HOME)
			camera_rotation_angle_ -= 0.05 * FW_PI;
		else if (ev.key == FW_KEY_END)
			camera_rotation_angle_ += 0.05 * FW_PI;
		else if (ev.key == FW_KEY_PAGE_DOWN)
		{
			translation_.setRow(0, translation_.getRow(0) - Vec4f(0, 0, 0, 0.05));
		}
		else if (ev.key == FW_KEY_PAGE_UP)
		{
			translation_.setRow(0, translation_.getRow(0) + Vec4f(0, 0, 0, 0.05));
		}
		else if (ev.key == FW_KEY_UP)
		{
			translation_.setRow(1, translation_.getRow(1) + Vec4f(0, 0, 0, 0.05));
		}
		else if (ev.key == FW_KEY_DOWN)
		{
			translation_.setRow(1, translation_.getRow(1) - Vec4f(0, 0, 0, 0.05));
		}
		else if (ev.key == FW_KEY_LEFT)
		{
			translation_.setRow(2, translation_.getRow(2) - Vec4f(0, 0, 0, 0.05));
		}
		else if (ev.key == FW_KEY_RIGHT)
		{
			translation_.setRow(2, translation_.getRow(2) + Vec4f(0, 0, 0, 0.05));
		}
		else if (ev.key == FW_KEY_X)
		{
			translation_.setRow(0, translation_.getRow(0) + Vec4f(0.05, 0, 0, 0)); // Scale up along the x-axis
		}
		else if (ev.key == FW_KEY_Z)
		{
			translation_.setRow(0, translation_.getRow(0) - Vec4f(0.05, 0, 0, 0)); // Scale down along the x-axis
		}
		else if (ev.key == FW_KEY_Y)
		{
			object_rotation_angle_ += -0.05 * FW_PI;
			Mat3f rot = Mat3f::rotation(Vec3f(0, 1, 0), object_rotation_angle_);
			object_rotator_.setCol(0, Vec4f(rot.getCol(0), 0));
			object_rotator_.setCol(1, Vec4f(rot.getCol(1), 0));
			object_rotator_.setCol(2, Vec4f(rot.getCol(2), 0));
			object_rotator_.setCol(3, Vec4f(0, 0, 0, 1));
		}
		else if (ev.key == FW_KEY_U)
		{
			object_rotation_angle_ += 0.05 * FW_PI;
			Mat3f rot = Mat3f::rotation(Vec3f(0, 1, 0), object_rotation_angle_);
			object_rotator_.setCol(0, Vec4f(rot.getCol(0), 0));
			object_rotator_.setCol(1, Vec4f(rot.getCol(1), 0));
			object_rotator_.setCol(2, Vec4f(rot.getCol(2), 0));
			object_rotator_.setCol(3, Vec4f(0, 0, 0, 1));
		}
		else if (ev.key == FW_KEY_R)
			key_pressed_ = !key_pressed_;
	}
	
	if (ev.type == Window::EventType_KeyUp) {
	}

	if (ev.type == Window::EventType_Mouse) {
		// EXTRA: you can put your mouse controls here.
		// Event::mouseDelta gives the distance the mouse has moved.
		// Event::mouseDragging tells whether some mouse buttons are currently down.
		// If you want to know which ones, you have to keep track of the button down/up events
		// (e.g. FW_KEY_MOUSE_LEFT).
		if (ev.mouseDragging)
		{
			// Mouse is being dragged, now is the time to do something.
			mouse_delta_ = ev.mouseDelta;
			if (mouse_delta_[0] > 0)
				camera_rotation_angle_ += 0.01 * FW_PI;
			else if (mouse_delta_[0] < 0)
				camera_rotation_angle_ -= 0.01 * FW_PI;
			
			if (mouse_delta_[1] > 0)
				camera_distance_ += 0.01;
			else if (mouse_delta_[1] < 0)
				camera_distance_ -= 0.01;
		}
	}

	if (ev.type == Window::EventType_Close) {
		window_.showModalMessage("Exiting...");
		delete this;
		return true;
	}


	window_.setVisible(true);
	if (ev.type == Window::EventType_Paint)
		render();

	window_.repaint();

	return false;
}

void App::initRendering() {
	// Ask the Nvidia framework for the GLContext object associated with the window.
	// As a side effect, this initializes the OpenGL context and lets us call GL functions.
	auto ctx = window_.getGL();
	
	// Create vertex attribute objects and buffers for vertex data.
	glGenVertexArrays(1, &gl_.static_vao);
	glGenVertexArrays(1, &gl_.dynamic_vao);
	glGenBuffers(1, &gl_.static_vertex_buffer);
	glGenBuffers(1, &gl_.dynamic_vertex_buffer);
	
	// Set up vertex attribute object for static data.
	glBindVertexArray(gl_.static_vao);
	glBindBuffer(GL_ARRAY_BUFFER, gl_.static_vertex_buffer);
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 0);
	glEnableVertexAttribArray(ATTRIB_NORMAL);
	glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, normal));

	// Load the static data to the GPU; needs to be done only once.
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * SIZEOF_ARRAY(reference_plane_data), reference_plane_data, GL_STATIC_DRAW);
	
	// Set up vertex attribute object for dynamic data. We'll load the actual data later, whenever the model changes.
	glBindVertexArray(gl_.dynamic_vao);
	glBindBuffer(GL_ARRAY_BUFFER, gl_.dynamic_vertex_buffer);
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 0);
	glEnableVertexAttribArray(ATTRIB_NORMAL);
	glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, normal));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	// Compile and link the shader program.
	// We use the Nvidia FW for creating the program; it's not difficult to do manually,
	// but takes about half a page of OpenGL boilerplate code.
	// This shader program will be used to draw everything except the user interface.
	// It consists of one vertex shader and one fragment shader.
	auto shader_program = new GLContext::Program(
		"#version 330\n"
		FW_GL_SHADER_SOURCE(
		layout(location = 0) in vec4 aPosition;
		layout(location = 1) in vec3 aNormal;
		
		out vec4 vColor;
		
		uniform mat4 uModelToWorld;
		uniform mat4 uWorldToClip;
		uniform mat3 uNormalTransform;
		uniform float uShading;
		
		const vec3 distinctColors[6] = vec3[6](
			vec3(0, 0, 1), vec3(0, 1, 0), vec3(0, 1, 1),
			vec3(1, 0, 0), vec3(1, 0, 1), vec3(1, 1, 0));
		const vec3 directionToLight = normalize(vec3(0.5, 0.5, -0.6));
		
		void main()
		{
			// EXTRA: oops, someone forgot to transform normals here...
			// mat3 transformer = transpose(inverse(mat3(uWorldToClip * uModelToWorld)));
			float clampedCosine = clamp(dot(uNormalTransform * aNormal, directionToLight), 0.0, 1.0);
			vec3 litColor = vec3(clampedCosine);
			vec3 generatedColor = distinctColors[gl_VertexID % 6];
			// gl_Position is a built-in output variable that marks the final position
			// of the vertex in clip space. Vertex shaders must write in it.
			gl_Position = uWorldToClip * uModelToWorld * aPosition;
			vColor = vec4(mix(generatedColor, litColor, uShading), 1);
		}
		),
		"#version 330\n"
		FW_GL_SHADER_SOURCE(
		in vec4 vColor;
		out vec4 fColor;
		void main()
		{
			fColor = vColor;
		}
		));
	// Tell the FW about the program so it gets properly destroyed at exit.
	ctx->setProgram("shaders", shader_program);

	// Get the IDs of the shader program and its uniform input locations from OpenGL.
	gl_.shader_program = shader_program->getHandle();
	gl_.world_to_clip_uniform = glGetUniformLocation(gl_.shader_program, "uWorldToClip");
	gl_.model_to_world_uniform = glGetUniformLocation(gl_.shader_program, "uModelToWorld");
	gl_.shading_toggle_uniform = glGetUniformLocation(gl_.shader_program, "uShading");
	gl_.normal_transform_uniform = glGetUniformLocation(gl_.shader_program, "uNormalTransform");
}

void App::render() {
	// Clear screen.
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Enable depth testing.
	glEnable(GL_DEPTH_TEST);

	Vec2i window_size = window_.getSize();

	// Extra for setting viewport
	// glViewport((window_size[0] - square_dim) / 2, (window_size[1] - square_dim) / 2, square_dim, square_dim);
	
	// Set up a matrix to transform from world space to clip space.
	// Clip space is a [-1, 1]^3 space where OpenGL expects things to be
	// when it starts drawing them.

	// Our camera is aimed at origin, and orbits around origin at fixed distance.

	Mat4f C;

	if (key_pressed_)
		camera_rotation_angle_ += 0.0001 * FW_PI;

	Mat3f rot = Mat3f::rotation(Vec3i(0, 1, 0), camera_rotation_angle_);

	C.setCol(0, Vec4f(rot.getCol(0), 0));
	C.setCol(1, Vec4f(rot.getCol(1), 0));
	C.setCol(2, Vec4f(rot.getCol(2), 0));
	C.setCol(3, Vec4f(0, 0, camera_distance_, 1));

	// Simple perspective.
	static const float fnear = 0.1f, ffar = 4.0f;
	float min_size = 1.0 * min(window_size[0], window_size[1]);
	Mat4f P;
	// Scale the dimensions depending on the window size to prevent distortion. 
	P.setCol(0, Vec4f(min_size / window_size[0], 0, 0, 0));
	P.setCol(1, Vec4f(0, min_size / window_size[1], 0, 0));
	P.setCol(2, Vec4f(0, 0, (ffar+fnear)/(ffar-fnear), 1));
	P.setCol(3, Vec4f(0, 0, -2*ffar*fnear/(ffar-fnear), 0));


	Mat4f world_to_clip = P * C;
	
	// Set active shader program.
	glUseProgram(gl_.shader_program);
	glUniform1f(gl_.shading_toggle_uniform, shading_toggle_ ? 1.0f : 0.0f);
	glUniformMatrix4fv(gl_.world_to_clip_uniform, 1, GL_FALSE, world_to_clip.getPtr());

	// Draw the reference plane. It is already in world coordinates.
	auto identity = Mat4f();
	glUniformMatrix4fv(gl_.model_to_world_uniform, 1, GL_FALSE, identity.getPtr());
	glBindVertexArray(gl_.static_vao);
	glDrawArrays(GL_TRIANGLES, 0, SIZEOF_ARRAY(reference_plane_data));
	
	// YOUR CODE HERE (R1)
	// Set the model space -> world space transform to translate the model according to user input.
	Mat4f modelToWorld = object_rotator_ * translation_;

	Mat3f transformer = (world_to_clip * modelToWorld).getXYZ().inverted().transposed();

	glUniformMatrix3fv(gl_.normal_transform_uniform, 1, GL_FALSE, transformer.getPtr());

	
	// Draw the model with your model-to-world transformation.
	glUniformMatrix4fv(gl_.model_to_world_uniform, 1, GL_FALSE, modelToWorld.getPtr());
	glBindVertexArray(gl_.dynamic_vao);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertex_count_);

	// Undo our bindings.
	glBindVertexArray(0);
	glUseProgram(0);
	
	// Check for OpenGL errors.
	GLContext::checkErrors();
	
	// Show status messages. You may find it useful to show some debug information in a message.
	common_ctrl_.message(sprintf("Use Home/End to rotate camera."), "instructions");
	common_ctrl_.message(sprintf("Camera is at (%.2f %.2f %.2f) looking towards origin.",
		-FW::sin(camera_rotation_angle_) * camera_distance_, 0.0f,
		-FW::cos(camera_rotation_angle_) * camera_distance_), "camerainfo");
}

vector<Vertex> App::loadObjFileModel(string filename) {
	window_.showModalMessage(sprintf("Loading mesh from '%s'...", filename.c_str()));

	vector<Vec3f> positions, normals;
	vector<array<unsigned, 6>> faces;

	// Open input file stream for reading.
	ifstream input(filename, ios::in);

	// Read the file line by line.
	string line;
	while(getline(input, line)) {
		// Replace any '/' characters with spaces ' ' so that all of the
		// values we wish to read are separated with whitespace.
		for (auto& c : line)
			if (c == '/')
				c = ' ';
			
		// Temporary objects to read data into.
		array<unsigned, 6>  f; // Face index array
		Vec3f               v;
		string              s;

		// Create a stream from the string to pick out one value at a time.
		istringstream        iss(line);

		// Read the first token from the line into string 's'.
		// It identifies the type of object (vertex or normal or ...)
		iss >> s;

		if (s == "v") { // vertex position
			// YOUR CODE HERE (R4)
			// Read the three vertex coordinates (x, y, z) into 'v'.
			// Store a copy of 'v' in 'positions'.
			// See std::vector documentation for push_back.
			for (int i = 0; i < 3; i++) {
				iss >> s;
				float coord = std::stof(s);
				v[i] = coord;
			}
			positions.push_back(v);
		} else if (s == "vn") { // normal
			// YOUR CODE HERE (R4)
			// Similar to above.
			for (int i = 0; i < 3; i++) {
				iss >> s;
				float coord = std::stof(s);
				v[i] = coord;
			}
			normals.push_back(v);
		} else if (s == "f") { // face
			// YOUR CODE HERE (R4)
			// Read the indices representing a face and store it in 'faces'.
			// The data in the file is in the format
			// f v1/vt1/vn1 v2/vt2/vn2 ...
			// where vi = vertex index, vti = texture index, vni = normal index.
			//
			// Remember we already replaced the '/' characters with whitespaces.
			//
			// Since we are not using textures in this exercise, you can ignore
			// the texture indices by reading them into a temporary variable.

			unsigned sink; // Temporary variable for reading the unused texture indices.
			unsigned idx;
			for (int i = 0; i < 3; i++) {
				iss >> s;
				idx = std::stoi(s) - 1;
				f[i*2] = idx;
				iss >> s;
				iss >> s;
				idx = std::stoi(s) - 1;
				f[i*2 + 1] = idx;
			}
			faces.push_back(f);
			// Note that in C++ we index things starting from 0, but face indices in OBJ format start from 1.
			// If you don't adjust for that, you'll index past the range of your vectors and get a crash.

			// It might be a good idea to print the indices to see that they were read correctly.
			// cout << f[0] << " " << f[1] << " " << f[2] << " " << f[3] << " " << f[4] << " " << f[5] << endl;
		}
	}
	common_ctrl_.message(("Loaded mesh from " + filename).c_str());
	return unpackIndexedData(positions, normals, faces);
}

/*
vector<Vertex> App::loadPLYFileModel(string filename) {
	window_.showModalMessage(sprintf("Loading mesh from '%s'...", filename.c_str()));

	vector<Vec3f> positions, normals;
	vector<array<unsigned, 6>> faces;

	
		//This function can only handle binary little endian format with vertices first in x,y,z order and faces next
		//in property list uchar int vertex_indices format. This kind of example model is provided in the submission.
		//Also this only works when faces exclusively contain triangles. 
	

	int vertex_count;
	int face_count;
	unsigned int num = 1;
	bool big_endian = true;
	char* ptr = (char*)&num;
	if (ptr[0] == 1){
		big_endian = false;
		cout << "Little endian" << endl;
	}
	else {
		cout << "Big endian" << endl;
	}

	// Open input file stream for reading.
	ifstream input(filename, ios::binary);

	// Read the file line by line.
	string line;

	// Read first line, should be ply.
	getline(input, line);
	assert(line == "ply");

	// Read and validate format from second line.
	getline(input, line);
	assert(line == "format ascii 1.0" || line == "format binary_little_endian 1.0" || line == "format binary_big_endian 1.0");

	while (getline(input, line) && line != "end_header") {
		string s;
		istringstream        iss(line);
		iss >> s;
		if (s == "element") {
			iss >> s;
			if (s == "vertex") {
				iss >> s;
				vertex_count = stoi(s);
			}
			else if (s == "face") {
				iss >> s;
				face_count = stoi(s);
			}
		}
	}

	int total_read = 0;

	for (int i = 0; i < vertex_count; i++) {
		float x;
		float y;
		float z;
		Vec3f v;
		char* buffer = new char[sizeof(x)];

		input.read(buffer, sizeof(x));
		memcpy(&x, buffer, sizeof(x));

		buffer = new char[sizeof(x)];
		
		input.read(buffer, sizeof(y));
		memcpy(&y, buffer, sizeof(y));

		buffer = new char[sizeof(x)];

		input.read(buffer, sizeof(z));
		memcpy(&z, buffer, sizeof(z));

		v[0] = x; v[1] = y; v[2] = z;

		positions.push_back(v.normalized());
	}

	for (int i = 0; i < face_count; i++) {
		unsigned char u;
		char* buffer = new char[sizeof(u)];

		input.read(buffer, sizeof(u));
		memcpy(&u, buffer, sizeof(u));

		int count = u;

		char* int_buffer = new char[sizeof(int)];

		array<unsigned, 6> f;

		// Assuming that count always equal to 3
		for (int j = 0; j < count; j++) {
			input.read(int_buffer, sizeof(int));
			int idx;
			memcpy(&idx, int_buffer, sizeof(int));
			assert(idx >= 0);
			f[j * 2] = idx;
		}
		
		Vec3f norm = cross(positions[f[2]], positions[f[4]]).normalized();
		normals.push_back(norm);
		f[1] = normals.size() - 1; f[3] = normals.size() - 1; f[5] = normals.size() - 1;
		faces.push_back(f);
	}

	cout << positions.size() << " " << normals.size() << " " << faces.size() << endl;

	common_ctrl_.message(("Loaded mesh from " + filename).c_str());
	return unpackIndexedData(positions, normals, faces);
}
*/

void FW::init(void) {
	new App;
}
