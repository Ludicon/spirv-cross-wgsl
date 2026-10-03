struct Light
{
    dir : vec3f,
    intensity : f32,
}

var<private> vColor : vec4f;
var<private> FragColor : vec4f;
var<private> lights : array<f32, 4>;
var<private> g_color : vec4f;

fn modify(v : ptr<function, vec4f>, f : ptr<function, f32>, scale : f32)
{
    (*v) *= scale;
    (*f) = (*v).x + (*v).y;
}

fn modify_struct(l : ptr<function, Light>)
{
    (*l).intensity *= 2.0f;
    (*l).dir = normalize((*l).dir);
}

fn write_global()
{
    g_color = vColor * 0.5f;
}

fn overload(x : f32) -> f32
{
    return x * 2.0f;
}

fn overload_31(x : vec2f) -> vec2f
{
    return x * 3.0f;
}

fn sum(arr : array<f32, 4>) -> f32
{
    var s = 0.0f;
    for (var i = 0; i < 4; i++)
    {
        s += arr[i];
    }
    return s;
}

fn frag_main()
{
    lights = array<f32, 4>(1.0f, 2.0f, 3.0f, 4.0f);
    var c = vColor;
    var param = c;
    let param_2 = 2.0f;
    var param_1 : f32;
    modify(&param, &param_1, param_2);
    c = param;
    let f = param_1;
    var l = Light(vec3f(1.0f, 2.0f, 3.0f), 1.0f);
    var param_3 = l;
    modify_struct(&param_3);
    l = param_3;
    write_global();
    let param_4 = f;
    let param_5 = vec2f(f);
    let param_6 = lights;
    FragColor = ((c + vec4f(l.dir * l.intensity, f)) + g_color) + vec4f(overload(param_4), overload_31(param_5), sum(param_6));
}

struct SPIRV_Cross_Input
{
    @location(0) vColor : vec4f,
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main(stage_input : SPIRV_Cross_Input) -> SPIRV_Cross_Output
{
    vColor = stage_input.vColor;
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
