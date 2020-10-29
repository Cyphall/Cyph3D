in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D Texture;

layout (location = 0) out vec4 Out_Color;

void main()
{
	Out_Color = texture(Texture, TexCoords);
}