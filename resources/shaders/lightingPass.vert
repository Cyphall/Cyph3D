layout(location = 0) in vec2 in_Vertex;
layout(location = 1) in vec2 in_UV;

uniform vec3 viewPos;

out VERT2FRAG {
    vec2  TexCoords;
    vec3  ViewPos;
} vert2frag;

void main()
{
    gl_Position = vec4(in_Vertex, 0, 1);

    vert2frag.TexCoords = in_UV;
    vert2frag.ViewPos = viewPos;
}