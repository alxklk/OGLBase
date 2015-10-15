#version 400

in vec3 v_p;
void main()
{
    gl_FragColor=vec4(1-v_p,1.0);
}
