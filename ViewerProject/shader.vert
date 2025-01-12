#version 330 core

layout(location = 0) in vec3 aPos;  // 위치 입력
layout(location = 1) in vec3 aNormal; // 법선 (현재는 사용 안함)
layout(location = 2) in vec2 aTexCoord; // 텍스처 좌표 (현재는 사용 안함)

uniform mat4 uMVP; // MVP 행렬 (유니폼 변수)

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0); // MVP로 변환된 좌표를 출력
}