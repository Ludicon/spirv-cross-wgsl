var<private> FragColor : vec4f;

fn foo(foo_1 : vec4f) -> vec4f
{
    return foo_1 + vec4f(1.0f);
}

fn foo_8(foo_1 : vec3f) -> vec4f
{
    return foo_1.xyzz + vec4f(1.0f);
}

fn foo_11(foo_1 : vec4f) -> vec4f
{
    return foo_1 + vec4f(2.0f);
}

fn foo_14(foo_1 : vec2f) -> vec4f
{
    return foo_1.xyxy + vec4f(2.0f);
}

fn frag_main()
{
    var foo_2 : vec4f = vec4f(1.0f);
    var foo_1 : vec4f = foo(foo_2);
    var foo_4 : vec3f = vec3f(1.0f);
    var foo_3 : vec4f = foo_8(foo_4);
    var foo_6 : vec4f = vec4f(1.0f);
    var foo_5 : vec4f = foo_11(foo_6);
    var foo_9 : vec2f = vec2f(1.0f);
    var foo_7 : vec4f = foo_14(foo_9);
    FragColor = ((foo_1 + foo_3) + foo_5) + foo_7;
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : vec4f,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
