//#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

const vec2 size = vec2(800, 600);
const int range = 2;

void main()
{
	vec4 source = texture(texture0, fragTexCoord);

    vec4 sum = vec4(0);

    for (int i = -range; i <= range; i++) {
        for (int j = -range; j <= range; j++) {
            sum += texture(texture0, fragTexCoord + vec2(i, j) / size);
        }
    }

    sum /= 4 * range * range;

	finalColor = sum;
    finalColor.a = 1;
}