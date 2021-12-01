#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(30);
	ofSetWindowTitle("openFrameworks");

	ofBackground(0);
	ofSetLineWidth(2);
	ofEnableDepthTest();

	auto radius = 15;
	auto x_span = radius * sqrt(3);
	auto flg = true;
	for (float y = -500; y < 500; y += radius * 1.5) {

		for (float x = -500; x < 500; x += x_span) {

			glm::vec3 location;
			if (flg) {

				location = glm::vec3(x, y, 0);
			}
			else {

				location = glm::vec3(x + (x_span / 2), y, 0);
			}

			this->location_list.push_back(location);
			this->life_list.push_back(0);
			this->noise_param_list.push_back(ofRandom(1000));
		}
		flg = !flg;
	}

	this->frame.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINES);
}

//--------------------------------------------------------------
void ofApp::update() {

	ofSeedRandom(39);

	this->face.clear();
	this->frame.clear();

	for (int i = 0; i < this->location_list.size(); i++) {

		this->life_list[i] = this->life_list[i] > 0 ? this->life_list[i] - 0.5 : 0;
		if (this->life_list[i] > 0) {

			this->noise_param_list[i] += ofMap(this->life_list[i], 0, 100, 0.05, 0.1);
		}
	}

	for (int i = 0; i < 1; i++) {

		int deg_start = ofMap(ofNoise(ofRandom(1000), ofGetFrameNum() * 0.005), 0, 1, -360, 360);
		int radius = ofMap(ofNoise(ofRandom(1000), ofGetFrameNum() * 0.01), 0, 1, 0, 500);

		for (int deg = deg_start; deg < deg_start + 360; deg += 60) {

			auto noise_location = glm::vec3(radius * cos(deg * DEG_TO_RAD), radius * sin(deg * DEG_TO_RAD), 0);
			for (int i = 0; i < this->location_list.size(); i++) {

				if (glm::distance(noise_location, this->location_list[i]) < 40) {

					this->life_list[i] = this->life_list[i] < 100 ? this->life_list[i] + 4 : 100;
				}
			}
		}
	}

	for (int i = 0; i < this->location_list.size(); i++) {

		auto noise_seed = glm::vec3(ofRandom(1000), ofRandom(1000), ofRandom(1000));
		auto noise_deg = glm::vec3(
			ofMap(ofNoise(glm::vec4(this->location_list[i] * 0.005, noise_seed.x + this->noise_param_list[i])), 0, 1, -360, 360),
			ofMap(ofNoise(glm::vec4(this->location_list[i] * 0.005, noise_seed.y + this->noise_param_list[i])), 0, 1, -360, 360),
			ofMap(ofNoise(glm::vec4(this->location_list[i] * 0.005, noise_seed.z + this->noise_param_list[i])), 0, 1, -360, 360));

		auto height = 5.f;
		if (this->life_list[i] > 10) {

			this->setHexagonToMesh(this->face, this->frame, this->location_list[i], 15, height, noise_deg);
		}
		else if(this->life_list[i] > 0){

			this->setHexagonToMesh(this->face, this->frame, this->location_list[i], 15, height, noise_deg * ofMap(this->life_list[i], 0, 10, 0, 1));
		}
		else {

			this->setHexagonToMesh(this->face, this->frame, this->location_list[i], 15, height, glm::vec3());
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	this->cam.begin();
	//ofRotateX(300);

	this->face.draw();
	this->frame.draw();

	this->cam.end();
}

//--------------------------------------------------------------
void ofApp::setHexagonToMesh(ofMesh& face_target, ofMesh& frame_target, glm::vec3 location, float radius, float height, glm::vec3 deg) {

	ofColor face_color = ofColor(0);
	ofColor frame_color = ofColor(255);

	auto rotation_x = glm::rotate(glm::mat4(), deg.x * (float)DEG_TO_RAD, glm::vec3(1, 0, 0));
	auto rotation_y = glm::rotate(glm::mat4(), deg.y * (float)DEG_TO_RAD, glm::vec3(0, 1, 0));
	auto rotation_z = glm::rotate(glm::mat4(), deg.z * (float)DEG_TO_RAD, glm::vec3(0, 0, 1));

	for (int deg = 90; deg < 450; deg += 60) {

		auto face_radius = radius - 0.5;
		auto face_index = face_target.getNumVertices();

		vector<glm::vec3> vertices;
		vertices.push_back(glm::vec3(0, 0, 0));
		vertices.push_back(glm::vec3(face_radius * cos(deg * DEG_TO_RAD), face_radius * sin(deg * DEG_TO_RAD), 0));
		vertices.push_back(glm::vec3(face_radius * cos((deg + 60) * DEG_TO_RAD), face_radius * sin((deg + 60) * DEG_TO_RAD), 0));
		vertices.push_back(glm::vec3(0, 0, height));
		vertices.push_back(glm::vec3(face_radius * cos((deg + 60) * DEG_TO_RAD), face_radius * sin((deg + 60) * DEG_TO_RAD), height));
		vertices.push_back(glm::vec3(face_radius * cos(deg * DEG_TO_RAD), face_radius * sin(deg * DEG_TO_RAD), height));

		for (auto& vertex : vertices) {

			vertex = location + glm::vec4(vertex, 0) * rotation_z * rotation_y * rotation_x;
		}

		face_target.addVertices(vertices);

		face_target.addIndex(face_index + 0); face_target.addIndex(face_index + 1); face_target.addIndex(face_index + 2);
		face_target.addIndex(face_index + 3); face_target.addIndex(face_index + 4); face_target.addIndex(face_index + 5);
		face_target.addIndex(face_index + 1); face_target.addIndex(face_index + 2); face_target.addIndex(face_index + 4);
		face_target.addIndex(face_index + 1); face_target.addIndex(face_index + 4); face_target.addIndex(face_index + 5);

		auto frame_index = frame_target.getNumVertices();

		vertices.clear();
		vertices.push_back(glm::vec3(0, 0, 0));
		vertices.push_back(glm::vec3(radius * cos(deg * DEG_TO_RAD), radius * sin(deg * DEG_TO_RAD), 0));
		vertices.push_back(glm::vec3(radius * cos((deg + 60) * DEG_TO_RAD), radius * sin((deg + 60) * DEG_TO_RAD), 0));
		vertices.push_back(glm::vec3(0, 0, height));
		vertices.push_back(glm::vec3(radius * cos((deg + 60) * DEG_TO_RAD), radius * sin((deg + 60) * DEG_TO_RAD), height));
		vertices.push_back(glm::vec3(radius * cos(deg * DEG_TO_RAD), radius * sin(deg * DEG_TO_RAD), height));

		for (auto& vertex : vertices) {

			vertex = location + glm::vec4(vertex, 0) * rotation_z * rotation_y * rotation_x;
		}

		frame_target.addVertices(vertices);

		frame_target.addIndex(frame_index + 1); frame_target.addIndex(frame_index + 2);
		frame_target.addIndex(frame_index + 4); frame_target.addIndex(frame_index + 5);
		frame_target.addIndex(frame_index + 1); frame_target.addIndex(frame_index + 5);
		frame_target.addIndex(frame_index + 2); frame_target.addIndex(frame_index + 4);

		for (int i = 0; i < vertices.size(); i++) {

			face_target.addColor(face_color);
			frame_target.addColor(frame_color);
		}
	}
}

//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}