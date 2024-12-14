vec3 toGamma(vec3 c)
{
    return pow(c, vec3(1.0 / 2.2));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // hold mouse button + move to move around scene
    vec3 linearColor = texelFetch(iChannel0, ivec2(fragCoord), 0).xyz;
    
    fragColor = vec4(toGamma(linearColor),1.0);
}