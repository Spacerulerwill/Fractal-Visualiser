#shader vertex

#version 330

layout(location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0); 
}

#shader fragment

#version 330

uniform ivec2 resolution = ivec2(1280, 720);
uniform vec2 location = vec2(0, 0);
uniform vec2 mousePos = vec2(0, 0);
uniform bool juliaMode = false;
uniform float zoom = 2.0;
uniform int iterations = 200;
uniform vec3 color_1 = vec3(0.5);
uniform vec3 color_2 = vec3(0.5);
uniform vec3 color_3 = vec3(1.0);
uniform vec3 color_4 = vec3(0.0, 0.33, 0.67);
uniform int time;

out vec4 FragColor;

// Constants
#define PI 3.1415925359
#define TWO_PI 6.2831852
#define MAX_STEPS 100
#define MAX_DIST 100.
#define SURFACE_DIST .01

float GetDist(vec3 p)
{
    vec4 s = vec4(0, 1, 6, 1); //Sphere. xyz is position w is radius
    float sphereDist = length(p - s.xyz) - s.w;
    float planeDist = p.y;
    float d = min(sphereDist, planeDist);

    return d;
}

vec3 GetNormal(vec3 p)
{
    float d = GetDist(p); // Distance
    vec2 e = vec2(.01, 0); // Epsilon
    vec3 n = d - vec3(
        GetDist(p - e.xyy),
        GetDist(p - e.yxy),
        GetDist(p - e.yyx));

    return normalize(n);
}


float RayMarch(vec3 ro, vec3 rd)
{
    float dO = 0.; //Distane Origin
    for (int i = 0; i < MAX_STEPS; i++)
    {
        vec3 p = ro + rd * dO;
        float ds = GetDist(p); // ds is Distance Scene
        dO += ds;
        if (dO > MAX_DIST || ds < SURFACE_DIST) break;
    }
    return dO;
}

float shadow(vec3 ro, vec3 rd, int k)
{
    float res = 1.0;
    for (float t = 0; t < MAX_STEPS; )
    {
        float h = GetDist(ro + rd * t);
        if (h < 0.001)
            return 0.0;
        res = min(res, k * h / t);
        t += h;
    }
    return res;
}

float CalculateDiffuseLighting(vec3 p)
{
    // Light (directional diffuse)
    vec3 lightPos = vec3(5, 5., 5.0); // Light Position
    vec3 l = normalize(lightPos - p); // Light Vector
    vec3 n = GetNormal(p); // Normal Vector

    float dif = dot(n, l); // Diffuse light
    dif = clamp(dif, 0., 1.); // Clamp so it doesnt go below 0

    float d = shadow(p + n * SURFACE_DIST * 2., l, 2);

    dif *= d;

    return dif;
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - .5 * resolution.xy) / resolution.y;
    vec3 ro = vec3(0, 1, 0); // Ray Origin/ Camera
    vec3 rd = normalize(vec3(uv.x, uv.y, 1));
    float d = RayMarch(ro, rd); // Distance


    vec3 p = ro + rd * d;

    float diff = CalculateDiffuseLighting(p);

    vec3 color = vec3(diff);

    // Set the output color
    gl_FragColor = vec4(color, 1.0);
}