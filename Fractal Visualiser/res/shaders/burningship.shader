#shader vertex

#version 330

layout(location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0); 
}

#shader fragment

#version 330

#define B 4.

uniform ivec2 resolution;
uniform vec2 location = vec2(0, 0);
uniform vec2 mousePos;
uniform bool juliaMode = false;
uniform float zoom  = 2.0;
uniform int iterations = 200;
uniform vec3 color_1 = vec3(0.5);
uniform vec3 color_2 = vec3(0.5);
uniform vec3 color_3 = vec3(1.0);
uniform vec3 color_4 = vec3(0.0, 0.33, 0.67);

out vec4 FragColor;

vec2 abscompsquare(vec2 z)
{
    float temp = abs(z.x);
    z.x = abs(z.x * z.x) - abs(z.y * z.y);
    z.y = 2.0 * temp * abs(z.y);
    return z;
}

float burningship(vec2 point) {
    vec2 z;

    if (juliaMode) { //z is point - julia set
        z = point;
        point = mousePos;

    }
    else { //z starts at 0 and is squared, with point added on - mandelbrot set
        z = vec2(0.0);
    }

    //calculate iterationts until it escapes
    int iters = 0;
    for (; iters < iterations; ++iters)
    {
        z = abscompsquare(z) + point;
        if (dot(z, z) > 4.0) break;
    }

    return iters - log(log(dot(z, z)) / log(B)) / log(2.);
}

vec3 pal(float t) {
    return color_1 + color_2 * cos(6.28318 * (color_3 * t + color_4));
}
void main()
{

    vec2 uv = gl_FragCoord.xy / vec2(resolution);
    float ratio = float(resolution.x) / resolution.y;
    uv.x *= ratio;
    uv -= vec2(ratio / 2, 0.5); //move center of mandelbrot to center of screen

    uv *= zoom; //zoom
    uv += location; // position

    // flip vertically
    uv.y *= -1;

    float sn = float(burningship(uv)) / iterations;

    vec3 color = pal(fract(6. * sn));


    FragColor = vec4(color, 1.0);
}