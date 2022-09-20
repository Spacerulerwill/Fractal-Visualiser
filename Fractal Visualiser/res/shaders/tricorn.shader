#shader vertex

#version 330

layout(location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0); 
}

#shader fragment

#version 330

uniform ivec2 resolution;
uniform vec2 location = vec2(0, 0);
uniform vec2 mousePos;
uniform bool juliaMode = false;
uniform float zoom  = 2.0;
uniform float color_1 = 0.0;
uniform float color_2 = 0.0;
uniform float color_3 = 0.0;
uniform float color_4 = 0.0;
uniform int iterations = 200;

out vec4 FragColor;

vec2 conjsquare(vec2 z)
{
    z = vec2(z.x, -z.y);
    float temp = z.x;
    z.x = z.x * z.x - z.y * z.y;
    z.y = 2.0 * temp * z.y;
    return z;
}

vec3 hsv2rgb(vec3 hue) {
    vec4 K = vec4(color_1, color_2, color_3, color_4);
    vec3 p = abs(fract(hue.xxx + K.xyz) * 6.0 - K.www);
    return hue.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), hue.y);
}

vec3 mandelbrot(vec2 point) {
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
        z = conjsquare(z) + point;
        if (dot(z, z) > 4.0) break;
    }

    //calculate color based on that
    float hue = iters / float(iterations);

    return hsv2rgb(vec3(hue, 1.0, 1.0));
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

    FragColor = vec4(mandelbrot(uv), 1.0);
}