//#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform sampler2D lut;

out vec4 finalColor;

const vec2 size = vec2(800, 600);
const int range = 5;
const float dist = 3.0;

vec3 lookup(vec3 color)
{
    return texture(lut, vec2(color.r / 16.0, fragTexCoord.y)).rgb;
}

void main()
{
	vec3 source = texture(texture0, fragTexCoord).rgb;

    vec3 sum = vec3(0);

    // for (int i = -range; i <= range; i++) {
    //     for (int j = -range; j <= range; j++) {
    //         vec2 pos = vec2(i, j);
    //         vec4 p = texture(texture0, fragTexCoord + pos / size * dist);
    //         // p -= 0.1;
    //         p /= length(pos / range);
    //         sum += max(vec4(0), p);
    //     }
    // }

    int axis = range * 2 + 1;
    // sum /= axis * axis;
    // sum *= 3;
    // sum *= 1.2;

	finalColor = vec4(source, 1);
    // finalColor /= (1 + finalColor);
    // finalColor.a = 1;
}